/**
 * @file usb.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-10
 *
 * @copyright Copyright (c) 2024
 */

#include "assert.h"
#include "err.h"
#include "dev/gpio.h"
#include "dev/gpio_signals.h"
#include "dev/usb.h"

#include "stm32f401/rcc.h"
#include "stm32f401/gpio.h"
#include "stm32f401/flash.h"
#include "stm32f401/pwr.h"
#include "stm32f401/usb.h"
#include "stm32f401/nvic.h"
#include "sys/yield.h"
#include "sys/sleep.h"
#include "sys/ev.h"
#include "util/msblsb.h"
#include "util/minmax.h"
#include "util/string.h"
#include "util/elems.h"


/* setup the logging level */
#define DEBUG DLVL_WARN
#include "debug.h"

/* system events */
ev_t usb_ev;

/* in/out endpoint related stuff */
typedef struct {
	/* current transfer pointer */
	void *ptr;
	/* transfer error code */
	err_t ec;
	/* current transfer size */
	size_t size, offs;
	/* transfer callback */
	usb_cb_t callback;
	/* setup flag */
	int setup;
    /* zero-length packet indication */
    int zlp;
} usb_ep_t;

/* in endpoints */
static usb_ep_t ep_in[4], ep_out[4];
/* last time we saw sof */
static time_t sof_ts; uint32_t sofs_recvd;

/* call endpoint callback */
static void USB_FinishTransfer(usb_ep_t *ep, err_t ec)
{
	/* transfer callback */
	usb_cb_t cb = ep->callback;
	/* size of the transfer */
	size_t size = ep->offs;
	/* store the error code */
	ep->ec = ec; ep->setup = 0;

	dprintf_d("transfer on ep %d (%d) of size %d is done (ec = %d)\n",
		ep - ep_in, ep - ep_out, size, ec);

	/* there is no callback for this transfer */
	if (!cb)
		return;
	/* clear the callback pointer before calling the callback, so that we don't
	 * end up with pointer to the function that is not up to date */
	ep->callback = 0; cb(&(usb_cbarg_t) { .error = ep->ec, .size = size });
}

/* dump the packet from the rx fifo */
static size_t USB_DumpPacket(size_t size)
{
    /* number of bytes that we need to read rounded up to full 32 bit words
     * that fifo is organized around */
    size_t b_left = (size + 3) & ~0x3;
    /* read data from fifo, b_left must be signed for the comparison to work */
    for (; b_left; b_left -= 4)
        (void)USBFS_FIFO(0);

    /* return the size of the dumped packet */
    return size;
}

/* read usb packet */
static size_t USB_ReadPacket(void *ptr, size_t size)
{
	/* data pointer */
	uint8_t *p = ptr; size_t b_left;
	/* read data from fifo */
	for (b_left = size; b_left >= 4; b_left -= 4, p += 4) {
		/* read register */
		uint32_t temp = USBFS_FIFO(0);
		/* alignment safe write */
		p[0] = temp >> 0, p[1] = temp >> 8;
		p[2] = temp >> 16, p[3] = temp >> 24;
	}

	/* last few bytes */
	if (b_left) {
		/* read last word */
		uint32_t temp = USBFS_FIFO(0);
		/* store last bytes */
		switch (b_left) {
		case 3 : *p++ = temp, temp >>= 8;
		case 2 : *p++ = temp, temp >>= 8;
		case 1 : *p++ = temp, temp >>= 8;
		}
	}
	/* return number of bytes read */
	return size;
}

/* write usb packet */
static size_t USB_WritePacket(int ep_num, const void *ptr, size_t size)
{
	/* data pointer */
	const uint8_t *p = ptr; size_t sz;
    /* limit next transfer size to the number of bytes available in fifo */
    size = min((USBFS_IE(ep_num)->DTXFSTS & USB_DTXFSTS_INEPTFSAV) * 4, size);
	/* read data from fifo */
	for (sz = size; sz >= 4; sz -= 4, p += 4)
		USBFS_FIFO(ep_num) = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;

	uint32_t temp = 0;
	/* form 32 bit word from last bytes */
	switch (sz) {
	case 3 : temp |= p[2] << 16;
	case 2 : temp |= p[1] << 8;
	case 1 : USBFS_FIFO(ep_num) = p[0] | temp;
	}

    dprintf_d("data write done, ep = %d, size = %d\n", ep_num, size);
	/* return number of bytes written */
	return size;
}

/* handle bus reset */
static void USB_HandleReset(void)
{
	/* disable remote wake-up signaling */
	USBFS->DCTL &= ~USB_DCTL_RWUSIG;

	/* flush tx fifos */
	USB_FlushTxFifo(0x10);
	/* flush rx fifo */
	USB_FlushRxFifo();

	/* mask out all the interrupts */
	USBFS->DIEPMSK = USBFS->DOEPMSK = USBFS->DAINTMSK = 0;

	/* clear endpoint interrupts */
	for (int i = 0; i < 4; i++) {
		/* disable in endpoint */
		if (USBFS_IE(i)->DIEPCTL & USB_DIEPCTL_EPENA)
			USBFS_IE(i)->DIEPCTL = USB_DIEPCTL_EPDIS | USB_DIEPCTL_SNAK;
		/* disable out endpoint */
		if (USBFS_OE(i)->DOEPCTL & USB_DOEPCTL_EPENA)
			USBFS_OE(i)->DOEPCTL = USB_DOEPCTL_EPDIS | USB_DOEPCTL_SNAK;
		/* clear interrupts */
		USBFS_IE(i)->DIEPINT = USBFS_OE(i)->DOEPINT = 0xff;
	}

	/* set out endpoint common interrupt mask: setup phase done,
	* transfer completed, endpoint disabled, OUT token received when endpoint
	* disabled */
	USBFS->DOEPMSK |= USB_DOEPMSK_STUPM | USB_DOEPMSK_XFRCM;
	/* timeout condition, transfer completed, endpoint disabled */
	USBFS->DIEPMSK |= USB_DIEPMSK_TOM | USB_DIEPMSK_XFRCM;

	/* set device address to 0 */
	USBFS->DCFG &= ~USB_DCFG_DAD;

    /* prepare event argument */
    usb_evarg_t ea = { .type = USB_EVARG_TYPE_RESET };
	/* start usb operation */
	Ev_Notify(&usb_ev, &ea, 0);

	/* clear flag */
	USBFS->GINTSTS = USB_GINTSTS_USBRST;
}

/* handle enumaration done */
static void USB_HandleEnum(void)
{
	/* configure turnaround time for full speed according to ahb frequency */
	USBFS->GUSBCFG = (USBFS->GUSBCFG & ~USB_GUSBCFG_TRDT) |
		  0x6 << LSB(USB_GUSBCFG_TRDT);

	/* set in endpoint in error state */
    for (usb_ep_t *in = ep_in; in != ep_in + elems(ep_in); in++)
        USB_FinishTransfer(in, EUSB_RESET);

    /* set out endpoint in error state */
    for (usb_ep_t *out = ep_out; out != ep_out + elems(ep_out); out++)
        USB_FinishTransfer(out, EUSB_RESET);

	/* prepare event argument */
    usb_evarg_t ea = { .type = USB_EVARG_TYPE_ENUM_DONE };
	/* start usb operation */
	Ev_Notify(&usb_ev, &ea, 0);

	/* clear global in nak */
	USBFS->DCTL |= USB_DCTL_CGINAK;
	/* clean flag */
	USBFS->GINTSTS = USB_GINTSTS_ENUMDNE;
}

/* handle rx fifo full */
static void USB_HandleRxlvlIsr(void)
{
	/* pop status register */
	uint32_t stat = USBFS->GRXSTSP;
	/* get length */
	uint32_t len = (stat & USB_GRXSTSP_BCNT) >> LSB(USB_GRXSTSP_BCNT);
	/* get the endpoint number */
	uint32_t ep_num = stat & USB_GRXSTSP_EPNUM;
	/* endpoint pointer */
	usb_ep_t *out = &ep_out[ep_num];

	dprintf_d("rx stat = %x, len = %d, ep_num = %d ec = %d\n", stat,
		len, ep_num, out->ec);

	/* decide what to do on packet status field */
	switch (stat & USB_GRXSTSP_PKTSTS) {
	/* setup frame received */
	case USB_GRXSTSP_PKTSTS_DEV_STP_RX :
	case USB_GRXSTSP_PKTSTS_DEV_OUT_RX : {
		/* maximal acceptable size of data for this endpoint */
        size_t max_len = 0, leftover = 0;
        /* ongoing valid transfer? */
        if (out->ec == EBUSY && out->offs < out->size)
            max_len = out->size - out->offs;
        /* not everything can be consumed */
        if (max_len < len)
            leftover = len - max_len;

        /* read the packet contents */
        out->offs += USB_ReadPacket((uint8_t *)out->ptr + out->offs,
			min(max_len, len));
        /* rest of the data needs to be dumped */
        if (leftover) {
            dprintf_w("warning - leftover = %d\n", leftover);
			USB_DumpPacket(leftover);
		}
	} break;
	}
}

/* output endpoint interrupt */
static void USB_HandleOutEp(void)
{
	/* determine which endpoints raised the interrupt */
	uint32_t irq = (USBFS->DAINT & USBFS->DAINTMSK & USB_DAINTMSK_OEPM) >>
			LSB(USB_DAINTMSK_OEPM);
	/* endpoint number, current endpoint interrupt flags, 310a bit 15 in doepint,
	 * read below for details */
	uint32_t ep_num, ep_irq;
	/* out endpoint pointer */
	usb_ep_t *out = ep_out; usb_oe_t *oe;

	/* process */
	for (ep_num = 0; irq; ep_num++, irq >>= 1, out++) {
		/* endpoint did not raise the interrupt? */
		if (!(irq & 1))
			continue;

		/* get the pointer to the register bank */
		oe = USBFS_OE(ep_num);
		/* read endpoint specific interrupt */
		ep_irq = oe->DOEPINT & USBFS->DOEPMSK;

		/* transfer complete */
		if (ep_irq & USB_DOEPINT_XFRC) {
			/* finalize the transfer */
			USB_FinishTransfer(out, EOK);
			/* clear flag */
			oe->DOEPINT = USB_DOEPINT_XFRC;
		}

		/* setup transaction done? */
		if (ep_irq & USB_DOEPINT_STUP) {
			/* finalize the transfer */
			USB_FinishTransfer(out, EOK);
			/* clear flag */
			oe->DOEPINT = USB_DOEPINT_STUP;
		}
	}
}

/* input endpoint interrupt */
static void USB_HandleInEpIsr(void)
{
	/* determine which endpoints raised the interrupt */
	uint32_t irq = (USBFS->DAINT & USBFS->DAINTMSK & USB_DAINTMSK_IEPM) >>
			LSB(USB_DAINTMSK_IEPM);
	/* fifo empty interrupt mask */
	uint32_t fe = USBFS->DIEPEMPMSK, fe_bit = 0x0001;
	/* current endpoint index and interrupt flags, transfer size */
	uint32_t ep_num, ep_irq;
	/* in endpoint pointer */
	usb_ep_t *in = ep_in; usb_ie_t *ie;

	/* all other ep go here */
	for (ep_num = 0; irq; ep_num++, irq >>= 1, fe >>= 1, fe_bit <<= 1, in++) {
		/* endpoint did not raise the interrupt? */
		if (!(irq & 1))
			continue;

		/* get the pointer to the register bank */
		ie = USBFS_IE(ep_num);
		/* read endpoint specific interrupt */
		ep_irq = ie->DIEPINT & (USBFS->DIEPMSK | USB_DIEPINT_TXFE);
		/* check if fifo empty is masked away */
		if (!(fe & 1))
			ep_irq &= ~USB_DIEPINT_TXFE;

		/* clrear interrupts */
        ie->DIEPINT = ep_irq;
		/* transfer complete? */
		if (ep_irq & USB_DIEPINT_XFRC) {
			/* do we need to follow this transfer with zero-length packet? */
            if (in->zlp) {
                /* clear the flag */
                in->zlp = 0;
                /* clear size and packet count setting */
                ie->DIEPTSIZ &= ~(USB_DIEPTSIZ_XFRSIZ | USB_DIEPTSIZ_PKTCNT);
                /* single packet is about to be sent */
                ie->DIEPTSIZ |= 1 << LSB(USB_DIEPTSIZ_PKTCNT);
                /* enable endpoint and clear nak condition */
                ie->DIEPCTL = (ie->DIEPCTL & ~USB_DIEPCTL_EPDIS) |
                    USB_DIEPCTL_CNAK | USB_DIEPCTL_EPENA;
            /* no need to append the zlp */
            } else {
                USB_FinishTransfer(in, EOK);
            }
        }

		/* fifo empty? */
		if (ep_irq & USB_DIEPINT_TXFE) {
			/* store packet */
			in->offs += USB_WritePacket(ep_num, (uint8_t *)in->ptr + in->offs,
                in->size - in->offs);
			/* update offset & mask out fifo empty interrupt */
			if (in->size <= in->offs) {
				in->offs = in->size; USBFS->DIEPEMPMSK &= ~fe_bit;
            }
		}
	}
}

/* incomplete isochronous transfer occured */
static void USB_HandleIncIsoIsr(void)
{
    /* prepare event argument */
    usb_evarg_t ea = { .type = USB_EVARG_TYPE_ISOINC };
	/* start usb operation */
	Ev_Notify(&usb_ev, &ea, 0);

    /* clear flag */
    USBFS->GINTSTS = USB_GINTSTS_IISOIXFR;
}

/* handle the sof routine */
static void USB_HandleSOF(void)
{
	/* store the sof timestamp */
	sof_ts = time(0); sofs_recvd++;
	/* clear the bit */
	USBFS->GINTSTS = USB_GINTSTS_SOF;
}

/* my interrupt handler */
void USB_HandlerTask(void *arg)
{
    for (;; Yield()) {
        /* get interrupt flags */
        uint32_t irq = USBFS->GINTSTS & USBFS->GINTMSK;

        /* invalid interrupt? */
        if (!irq)
            continue;

        /* display interrupt information */
		if (irq & ~(USB_GINTSTS_SOF))
			dprintf_d("irq = %08x\n", irq);

		/* usb reset */
        if (irq & USB_GINTSTS_USBRST)
            USB_HandleReset();

		/* usb enumeration done */
        if (irq & USB_GINTSTS_ENUMDNE)
            USB_HandleEnum();


		/* wakeup interrupt */
        if (irq & USB_GINTSTS_WKUINT)
            USBFS->GINTSTS = USB_GINTSTS_WKUINT;

        /* usb suspend interrupt */
        if (irq & USB_GINTSTS_USBSUSP)
            USBFS->GINTSTS = USB_GINTSTS_USBSUSP;

		/* invalid mode interrupt? */
        if (irq & USB_GINTSTS_MMIS)
            USBFS->GINTSTS = USB_GINTSTS_MMIS;

		/* start of frame was received */
		if (irq & USB_GINTSTS_SOF)
			USB_HandleSOF();


        /* OUT endpoint interrupt */
        if (irq & USB_GINTSTS_OEPINT)
            USB_HandleOutEp();

        /* IN endpoint interrupt */
        if (irq & USB_GINTSTS_IEPINT)
            USB_HandleInEpIsr();

        /* isochronous transfer incomplete */
        if (irq & USB_GINTSTS_IISOIXFR)
            USB_HandleIncIsoIsr();

        /* got packet in rx fifo? */
        if (irq & USB_GINTSTS_RXFLVL)
            USB_HandleRxlvlIsr();
    }
}

/* initialize usb support */
err_t USB_Init(void)
{
    /* enable usb clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
    /* enable clock to the power configurator */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    /* configure alternate function for the gpio pins */
    GPIOSig_CfgAltFunction((gpio_signal_t)GPIO_SIGNAL_BLACKPILL_A11,
		GPIO_AF_OTG1_FS);
    GPIOSig_CfgAltFunction((gpio_signal_t)GPIO_SIGNAL_BLACKPILL_A12,
        GPIO_AF_OTG1_FS);
    /* configure the speed of pins */
    GPIOSig_CfgPull((gpio_signal_t)GPIO_SIGNAL_BLACKPILL_A11,
        GPIO_OSPEED_HIGH);
    GPIOSig_CfgPull((gpio_signal_t)GPIO_SIGNAL_BLACKPILL_A12,
        GPIO_OSPEED_HIGH);

	/* disable the interrupts */
	USBFS->GAHBCFG &= ~USB_GAHBCFG_GINTMSK;
	/* Init the Core (common init.) */
	/* Select FS Embedded PHY */
	USBFS->GUSBCFG |= USB_GUSBCFG_PHYSEL;

	/* core reset */
	/* Wait for AHB master IDLE state. */
	while ((USBFS->GRSTCTL & USB_GRSTCTL_AHBIDL) == 0);
	/* Core Soft Reset */
	USBFS->GRSTCTL |= USB_GRSTCTL_CSRST;
	/* wait for reset fo finish */
	while ((USBFS->GRSTCTL & USB_GRSTCTL_CSRST) != 0);
	/* Deactivate the power down */
	USBFS->GCCFG = USB_GCCFG_PWRDWN;

	/* set current mode */
	/* reset mode setting */
	USBFS->GUSBCFG &= ~(USB_GUSBCFG_FHMOD | USB_GUSBCFG_FDMOD);
	/* enforce device mode */
	USBFS->GUSBCFG |= USB_GUSBCFG_FDMOD;
	/* wait for at least 50 ms */
	Sleep(50);

	/* reset out endpoints */
	for (int i = 0; i < 4; i++)
		USBFS->DIEPTXF[i] = 0;

    /* initialize device */
    /* keep device disconnected */
    USBFS->DCTL  |= USB_DCTL_SDIS;
    /* Deactivate VBUS Sensing B */
    USBFS->GCCFG |= USB_GCCFG_NOVBUSSENS;
    /* deactivate sending for "A" and "B" Devices */
    USBFS->GCCFG &= ~USB_GCCFG_VBUSBSEN;
    USBFS->GCCFG &= ~USB_GCCFG_VBUSASEN;

    /* Restart the Phy Clock */
    USBFS->PCGCCTL = 0;
    /* Set Full speed phy */
    USBFS->DCFG |= USB_DCFG_DSPD_0 | USB_DCFG_DSPD_1;


	/* flush tx fifos */
	USB_FlushTxFifo(0x10);
	/* flush rx fifo */
	USB_FlushRxFifo();

	/* Clear all pending Device Interrupts */
	/* .. for IN endpoints */
	USBFS->DIEPMSK = 0;
	/* .. for OUT enpoints */
	USBFS->DOEPMSK = 0;
	/* mask all interrupts */
	USBFS->DAINTMSK = 0;

	/* reset in endpoints */
	for (int i = 0; i < 4; i++) {
		/* reset configuration */
		USBFS_IE(i)->DIEPCTL = USB_DIEPCTL_EPDIS | USB_DIEPCTL_SNAK;
		/* reset size */
		USBFS_IE(i)->DIEPTSIZ = 0;
		/* clear interrupts */
		USBFS_IE(i)->DIEPINT  = 0xFF;
	}

	/* reset out endpoints */
	for (int i = 0; i < 4; i++) {
		/* reset configuration */
		USBFS_OE(i)->DOEPCTL = USB_DOEPCTL_EPDIS | USB_DOEPCTL_SNAK;
		/* reset size */
		USBFS_OE(i)->DOEPTSIZ = 0;
		/* clear interrupts */
		USBFS_OE(i)->DOEPINT  = 0xFF;
	}

	/* Disable all interrupts. */
	USBFS->GINTMSK = 0;
	/* Clear any pending interrupts */
	USBFS->GINTSTS = 0xBFFFFFFF;

	/* enable rx level interrupt */
	USBFS->GINTMSK |= USB_GINTMSK_RXFLVLM;
	/* Enable interrupts matching to the Device mode ONLY */
	USBFS->GINTMSK |= USB_GINTMSK_USBSUSPM | USB_GINTMSK_USBRST |
					  USB_GINTMSK_ENUMDNEM | USB_GINTMSK_IEPINT |
					  USB_GINTMSK_OEPINT   | USB_GINTMSK_IISOIXFRM |
					  USB_GINTMSK_WUIM | USB_GINTMSK_SOFM;


    /* start the host clock modules */
    USBFS->PCGCCTL &= ~(USB_PCGCCTL_GATECLK | USB_PCGCCTL_STOPCLK);
    /* this shall enable pull up resitor */
    USBFS->DCTL &= ~USB_DCTL_SDIS;
	/* wait for at least 3 ms */
	Sleep(3);
    /* enable interrupts globally */
    USBFS->GAHBCFG |= USB_GAHBCFG_GINTMSK;

    /* we use the usb otg peripheral as a system task*/
    Yield_Task(USB_HandlerTask, 0, 2048);

	/* report status */
	return EOK;
}

/* is the link active? */
int USB_IsLinkActive(void)
{
	/* if we did not receive the sof since 100ms we can consider the link
	 * to be dead */
	return sofs_recvd && dtime_now(sof_ts) < 100;
}

/* set rx fifo size in 32 bit words */
void USB_SetRxFifoSize(size_t size)
{
	/* convert to words */
	size = (size + 3) / 4;
    /* minimal fifo length allowed */
    if (size < 16)
		size = 16;
	/* set rx fifo size */
	USBFS->GRXFSIZ = size;
}

/* set tx fifo size TODO: make it into bytes */
void USB_SetTxFifoSize(usb_epnum_t ep_num, size_t size)
{
    /* fifo offset */
    uint32_t offset = USBFS->GRXFSIZ;
	/* convert to words */
	size = (size + 3) / 4;

    /* minimal fifo length allowed */
    if (size < 16)
        size = 16;

    /* endpoint 0 */
    if (ep_num == 0) {
        USBFS->DIEPTXF0_HNPTXFSIZ = size << LSB(USB_DIEPTXF_INEPTXFD) | offset;
    /* other endpoints */
    } else {
        /* get offset from ep0 */
        offset += USBFS->DIEPTXF0_HNPTXFSIZ >> LSB(USB_DIEPTXF_INEPTXFD);
        /* get offset from other endpoints */
        for (uint32_t i = 1; i < ep_num; i++)
            offset += USBFS->DIEPTXF[i - 1] >> LSB(USB_DIEPTXF_INEPTXFD);
        /* store */
        USBFS->DIEPTXF[ep_num - 1] = size << LSB(USB_DIEPTXF_INEPTXFD) | offset;
    }
}

/* flush rx fifo */
void USB_FlushRxFifo(void)
{
    /* wait for ahb master idle state */
    while (!(USBFS->GRSTCTL & USB_GRSTCTL_AHBIDL));

    /* flush receive fifo */
    USBFS->GRSTCTL = USB_GRSTCTL_RXFFLSH;
    /* wait for flush to complete */
    while ((USBFS->GRSTCTL & USB_GRSTCTL_RXFFLSH) != 0);
}

/* flush tx fifo */
void USB_FlushTxFifo(usb_epnum_t ep_num)
{
    /* wait for ahb master idle state */
    while (!(USBFS->GRSTCTL & USB_GRSTCTL_AHBIDL));
    /* Tx fifo */
    USBFS->GRSTCTL = USB_GRSTCTL_TXFFLSH | ep_num << LSB(USB_GRSTCTL_TXFNUM);
    /* wait for flush to complete */
    while ((USBFS->GRSTCTL & USB_GRSTCTL_TXFFLSH) != 0);
}

/* start data transmission */
err_t USB_StartINTransfer(usb_epnum_t ep_num, void *ptr, size_t size,
    usb_cb_t cb)
{
    /* endpoint pointer */
    usb_ep_t *in = &ep_in[ep_num]; usb_ie_t *ie = USBFS_IE(ep_num);
    /* single packet max size, packet count */
    uint32_t max_size, pkt_cnt;
    /* endpoint type */
    uint32_t ep_type = ie->DIEPCTL & USB_DIEPCTL_EPTYP;

    /* transfer is already in progress */
    if (in->ec == EBUSY)
        return in->ec;

    /* store pointer and size and operation finished callback */
    in->ptr = ptr, in->size = size, in->offs = 0, in->callback = cb;
    in->ec = EBUSY;

    dprintf_d("starting IN transfer on ep %d\n", ep_num);

    /* get single packet max. size */
    max_size = ie->DIEPCTL & USB_DIEPCTL_MPSIZ;
    /* ep0 uses special coding for mpsiz field */
    if (ep_num == USB_EP0)
        max_size = 64 >> (max_size & 0x3);
    /* get packet count for this transfer (account for incomplete packets) */
    pkt_cnt = max(1, (size + max_size - 1) / max_size);

    /* bulk transfers must end up with an zlp if the transfer size is the exact
     * multiple of the max packet size */
    if ((ie->DIEPCTL & USB_DIEPCTL_EPTYP) == USB_DIEPCTL_EPTYP_BULK)
        in->zlp = (pkt_cnt * max_size == size);

    dprintf_d("pkt_cnt = %d, for size =%d, zlp = %d\n", pkt_cnt, size, in->zlp);

    /* clear size and packet count setting */
    ie->DIEPTSIZ &= ~(USB_DIEPTSIZ_XFRSIZ | USB_DIEPTSIZ_PKTCNT);
    /* program transfer size */
    ie->DIEPTSIZ |= pkt_cnt << LSB(USB_DIEPTSIZ_PKTCNT) | (size);
    /* isochronous endpoint? */
    if (ep_type == USB_DIEPCTL_EPTYP_ISO) {
        /* set the number of packets for frame */
        ie->DIEPTSIZ = (ie->DIEPTSIZ & ~USB_DIEPTSIZ_MULCNT) |
            1 << LSB(USB_DIEPTSIZ_MULCNT);
    }

    /* enable endpoint and clear nak condition */
    ie->DIEPCTL = (ie->DIEPCTL & ~USB_DIEPCTL_EPDIS) |
        USB_DIEPCTL_CNAK | USB_DIEPCTL_EPENA;

    /* enable fifo tx empty interrupt for this endpoint: the interrupt routine
     * will fetch the data pointed by the ptr */
    if (size && ep_type != USB_DIEPCTL_EPTYP_ISO) {
        USBFS->DIEPEMPMSK |= 1 << ep_num;
    /* in case of isochronous transfers we feed the fifo right away */
    } else if (ep_type == USB_DIEPCTL_EPTYP_ISO) {
        /* we need to manually set the even-odd frame id using the frame count
            * provided */
        if (USBFS->DSTS & (1 << LSB(USB_DSTS_FNSOF))) {
            ie->DIEPCTL |= USB_DIEPCTL_SD0PID_SEVNFRM;
        } else {
            ie->DIEPCTL |= USB_DIEPCTL_SODDFRM;
        }
        /* write the packet contents */
        in->offs = USB_WritePacket(ep_num, ptr, size);
    }

    /* always returns 0, no sync operation possible */
    return 0;
}

/* wait for in transfer to finsh */
err_t USB_WaitINTransfer(usb_epnum_t ep_num, dtime_t timeout)
{
    /* output enpoint control block */
    usb_ep_t *in = &ep_in[ep_num];

    /* waiting loop */
    for (time_t ts = time(0); in->ec == EBUSY; Yield()) {
        /* handle timeout */
        if (timeout && dtime_now(ts) > timeout)
            return ETIMEOUT;
        /* handle cancellation */
        if (Yield_IsCancelled())
            return ECANCEL;
    }
    /* return the data size or the error code if error has occured */
    return in->ec == EOK ? in->offs: in->ec;
}

/* stop out endpoint transfer */
err_t USB_StopINTransfer(usb_epnum_t ep_num)
{
    /* output enpoint control block */
    usb_ep_t *in = &ep_in[ep_num]; usb_ie_t *ie = USBFS_IE(ep_num);
    /* trasnfer is not ongoing */
    if (in->ec != EBUSY)
        return EFATAL;

    /* disable endpoint */
    ie->DIEPCTL |= USB_DIEPCTL_EPDIS | USB_DIEPCTL_SNAK;
    /* wait until the enpoint is still enabled */
    if (ep_num != USB_EP0)
        while (ie->DIEPCTL & USB_DIEPCTL_EPENA);
    /* clear the size register */
    ie->DIEPTSIZ = 0; // TODO: test it out
    /* flush transmission fifo */
    USB_FlushTxFifo(ep_num);
    // TODO: clear interrupts? use endpoint disabled interrupt?
    /* finalize the transfer */
    USB_FinishTransfer(in, EUSB_EP_DIS);

    /* report success */
    return EUSB_EP_DIS;
}

/* start data reception */
err_t USB_StartOUTTransfer(usb_epnum_t ep_num, void *ptr, size_t size,
    usb_cb_t cb)
{
    /* endpoint pointer */
    usb_ep_t *out = &ep_out[ep_num]; usb_oe_t *oe = USBFS_OE(ep_num);
    /* single packet max size, packet count */
    uint32_t max_size, pkt_cnt;

    /* transfer is already in progress */
    if (out->ec == EBUSY)
        return out->ec;

    dprintf_d("starting OUT transfer on ep %d\n", ep_num);

    /* store pointer and size and operation finished callback */
    out->ptr = ptr, out->size = size, out->offs = 0, out->setup = 0;
    out->callback = cb; out->ec = EBUSY;

    /* get single packet max. size */
    max_size = oe->DOEPCTL & USB_DOEPCTL_MPSIZ;
    /* ep0 uses special coding for mpsiz field */
    if (ep_num == USB_EP0)
        max_size = 64 >> (max_size & 0x3);
    /* get packet count for this transfer (account for incomplete packets) */
    pkt_cnt = max(1, (size + max_size - 1) / max_size);

    /* clear size and packet count setting */
    oe->DOEPTSIZ &= ~(USB_DOEPTSIZ_XFRSIZ | USB_DOEPTSIZ_PKTCNT);
    /* program transfer size */
    oe->DOEPTSIZ |= pkt_cnt << LSB(USB_DOEPTSIZ_PKTCNT) | max_size;
    /* clear nak and enable endpoint for incoming data */
    oe->DOEPCTL = (oe->DOEPCTL & ~USB_DOEPCTL_EPDIS) |
        USB_DOEPCTL_CNAK | USB_DOEPCTL_EPENA;

    /* always returns 0, no sync operation possible */
    return 0;
}

/* wait for out transfer to finsh */
err_t USB_WaitOUTTransfer(usb_epnum_t ep_num, dtime_t timeout)
{
    /* output enpoint control block */
    usb_ep_t *out = &ep_out[ep_num];

    /* waiting loop */
    for (time_t ts = time(0); out->ec == EBUSY; Yield()) {
        /* handle timeout */
        if (timeout && dtime_now(ts) > timeout)
            return ETIMEOUT;
        /* handle cancellation */
        if (Yield_IsCancelled())
            return ECANCEL;
    }
    /* return the data size or the error code if error has occured */
    return out->ec == EOK ? out->offs: out->ec;
}

/* stop out endpoint transfer */
err_t USB_StopOUTTransfer(usb_epnum_t ep_num)
{
    /* output enpoint control block */
    usb_ep_t *out = &ep_out[ep_num]; usb_oe_t *oe = USBFS_OE(ep_num);
    /* trasnfer is not ongoing */
    if (out->ec != EBUSY)
        return EFATAL;

    /* disable endpoint */
    oe->DOEPCTL |= USB_DOEPCTL_EPDIS | USB_DOEPCTL_SNAK;
    /* wait until the enpoint is still enabled */
    if (ep_num != USB_EP0)
        while (oe->DOEPCTL & USB_DIEPCTL_EPENA);
    /* clear the size register */
    oe->DOEPTSIZ = 0; // TODO: test it
    /* finalize the transfer */
    USB_FinishTransfer(out, EUSB_EP_DIS);
    /* report success */
    return EUSB_EP_DIS;
}

/* start setup transfer: size must be a multiple of 8 (setup frame size), use
 * 24 for best results since host may issue 3 back to back setup packets */
err_t USB_StartSETUPTransfer(int ep_num, void *ptr, size_t size,
    usb_cb_t cb)
{
    /* endpoint pointer */
    usb_ep_t *out = &ep_out[ep_num]; usb_oe_t *oe = USBFS_OE(ep_num);
    /* transfer is already in progress */
    if (out->ec == EBUSY)
        return out->ec;

    /* store pointer and size and operation finished callback */
    out->ptr = ptr, out->size = size, out->offs = 0, out->setup = 1;
    out->callback = cb; out->ec = EBUSY;

    dprintf_d("starting setup transfer, size = %d\n", size);

    /* prepare size register: accept 3 packets */
    oe->DOEPTSIZ = 3 * 8 | 1 << LSB(USB_DOEPTSIZ0_PKTCNT) |
            3 << LSB(USB_DOEPTSIZ0_STUPCNT);
    /* always returns 0, no sync operation possible */
    return 0;
}

/* configure IN endpoint and activate it */
void USB_ConfigureINEndpoint(usb_epnum_t ep_num, usb_eptype_t type, size_t mp_size)
{
    /* register bank */
    usb_ie_t *ie = USBFS_IE(ep_num);
    /* read register */
    uint32_t temp = ie->DIEPCTL & ~(USB_DIEPCTL_EPTYP |
            USB_DIEPCTL_MPSIZ | USB_DIEPCTL_TXFNUM);
    /* ep0 uses special encoding for max packet size */
    if (ep_num == USB_EP0)
        mp_size = (mp_size == 8) ? 0x3 : (mp_size == 16) ? 0x2 :
                    (mp_size == 32) ? 0x1 : 0x0;
    /* write back */
    ie->DIEPCTL = temp | type << LSB(USB_DIEPCTL_EPTYP) |
        mp_size << LSB(USB_DIEPCTL_MPSIZ) | ep_num << LSB(USB_DIEPCTL_TXFNUM) |
        USB_DIEPCTL_USBAEP | USB_DIEPCTL_SD0PID_SEVNFRM;

    /* enable interrupt generation */
    USBFS->DAINTMSK |= 1 << (ep_num + LSB(USB_DAINTMSK_IEPM));
}

/* configure OUT endpoint and activate it */
void USB_ConfigureOUTEndpoint(usb_epnum_t ep_num, usb_eptype_t type, size_t mp_size)
{
    /* register bank */
    usb_oe_t *oe = USBFS_OE(ep_num);
    /* read register */
    uint32_t temp = oe->DOEPCTL & ~(USB_DOEPCTL_EPTYP |
            USB_DOEPCTL_MPSIZ);
    /* ep0 uses special encoding for max packet size */
    if (ep_num == USB_EP0)
        mp_size = (mp_size == 8) ? 0x3 : (mp_size == 16) ? 0x2 :
                    (mp_size == 32) ? 0x1 : 0x0;
    /* write back */
    oe->DOEPCTL = temp | type << LSB(USB_DOEPCTL_EPTYP) |
            mp_size << LSB(USB_DOEPCTL_MPSIZ) | USB_DIEPCTL_USBAEP |
            USB_DOEPCTL_SD0PID_SEVNFRM;

    /* enable interrupt generation */
    USBFS->DAINTMSK |= 1 << (ep_num + LSB(USB_DAINTMSK_OEPM));
}

/* set device address */
void USB_SetDeviceAddress(uint8_t addr)
{
    /* modify address field */
    USBFS->DCFG = (USBFS->DCFG & ~USB_DCFG_DAD) | addr << LSB(USB_DCFG_DAD);
}

/* stall out endpoint */
void USB_StallOUTEndpoint(usb_epnum_t ep_num)
{
    /* stall endpoint */
    USBFS_OE(ep_num)->DOEPCTL |= USB_DOEPCTL_STALL;
}

/* stall in endpoint */
void USB_StallINEndpoint(usb_epnum_t ep_num)
{
    /* stall endpoint */
    USBFS_IE(ep_num)->DIEPCTL |= USB_DIEPCTL_STALL;
}

/* disable in endoint */
void USB_DisableINEndpoint(usb_epnum_t ep_num)
{
    /* set the disable and nak bits */
    USBFS_IE(ep_num)->DIEPCTL |= USB_DIEPCTL_EPDIS | USB_DIEPCTL_SNAK;
}

/* disable out endoint */
void USB_DisableOUTEndpoint(usb_epnum_t ep_num)
{
    /* set the disable and nak bits */
    USBFS_OE(ep_num)->DOEPCTL |= USB_DOEPCTL_EPDIS | USB_DOEPCTL_SNAK;
}