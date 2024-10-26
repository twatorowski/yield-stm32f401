/**
 * @file usb.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-10-26
 *
 * @copyright Copyright (c) 2024
 */

#include <stdint.h>

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
#include "sys/sleep.h"
#include "util/msblsb.h"
#include "util/minmax.h"
#include "util/string.h"
#include "util/elems.h"

#define DEBUG
#include "debug.h"


/* initialize usb device */
err_t USB_DevInit(void)
{

	/* reset out endpoints */
	for (int i = 0; i < elems(USBFS->DIEPTXF); i++)
		USBFS->DIEPTXF[i] = 0;

	/* disable sensing for the usb voltage */
	USBFS->DCTL  |= USB_DCTL_SDIS;
    USBFS->GCCFG |= USB_GCCFG_NOVBUSSENS;
    USBFS->GCCFG &= ~USB_GCCFG_VBUSBSEN;
    USBFS->GCCFG &= ~USB_GCCFG_VBUSASEN;

	/* Restart the Phy Clock */
  	USBFS->PCGCCTL = 0x00;

	/* setup full speed operation with internal phy */
	USBFS->DCFG |= USB_DCFG_DSPD_0 | USB_DCFG_DSPD_1;


	/* flush the fifos */
	/* Wait for AHB master IDLE state. */
	while ((USBFS->GRSTCTL & USB_GRSTCTL_AHBIDL) == 0);
	/* Tx fifo */
	USBFS->GRSTCTL = USB_GRSTCTL_TXFFLSH | 0x10 << LSB(USB_GRSTCTL_TXFNUM);
	/* wait for flush to complete */
	while ((USBFS->GRSTCTL & USB_GRSTCTL_TXFFLSH) != 0);

	/* Wait for AHB master IDLE state. */
	while ((USBFS->GRSTCTL & USB_GRSTCTL_AHBIDL) == 0);
	/* flush receive fifo */
	USBFS->GRSTCTL = USB_GRSTCTL_RXFFLSH;
	/* wait for flush to complete */
	while ((USBFS->GRSTCTL & USB_GRSTCTL_RXFFLSH) != 0);



	/* Clear all pending Device Interrupts */
	USBFS->DIEPMSK = 0x00;
	USBFS->DOEPMSK = 0x00;
	USBFS->DAINTMSK = 0x00;


	/* reset in endpoints */
	for (int i = 0; i < 4; i++) {
		/* reset configuration */
		USBFS_IE(i)->DIEPCTL = USB_DIEPCTL_EPDIS | USB_DIEPCTL_SNAK;
		/* reset size */
		USBFS_IE(i)->DIEPTSIZ = 0;
		/* clear interrupts */
		USBFS_IE(i)->DIEPINT  = 0xFB7FU;
	}

	/* reset out endpoints */
	for (int i = 0; i < 4; i++) {
		/* reset configuration */
		USBFS_OE(i)->DOEPCTL = USB_DOEPCTL_EPDIS | USB_DOEPCTL_SNAK;
		/* reset size */
		USBFS_OE(i)->DOEPTSIZ = 0;
		/* clear interrupts */
		USBFS_OE(i)->DOEPINT  = 0xFB7FU;
	}


	USBFS->DIEPMSK &= ~(USB_DIEPMSK_TXFURM);
	/* Disable all interrupts. */
	USBFS->GINTMSK = 0U;
	/* Clear any pending interrupts */
	USBFS->GINTSTS = 0xBFFFFFFFU;

	/* Enable interrupts matching to the Device mode ONLY */
	USBFS->GINTMSK |= USB_GINTMSK_USBSUSPM | USB_GINTMSK_USBRST |
		USB_GINTMSK_ENUMDNEM | USB_GINTMSK_IEPINT |
		USB_GINTMSK_OEPINT   | USB_GINTMSK_IISOIXFRM |
		USB_GINTMSK_PXFRM_IISOOXFRM | USB_GINTMSK_WUIM;



	/* report status */
	return EOK;
}

/* initialize usb support */
int USB_Init(void)
{
	/* enable usb clock */
	RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
	/* enable clock to the power configurator */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    /* configure alternate function for the gpio pins */
    GPIOSig_CfgAltFunction((gpio_signal_t)GPIO_SIGNAL_BLACKPILL_A11, GPIO_AF_OTG1_FS);
    GPIOSig_CfgAltFunction((gpio_signal_t)GPIO_SIGNAL_BLACKPILL_A12, GPIO_AF_OTG1_FS);
    /* configure the speed of pins */
    GPIOSig_CfgPull((gpio_signal_t)GPIO_SIGNAL_BLACKPILL_A11, GPIO_OSPEED_VERY_HIGH);
    GPIOSig_CfgPull((gpio_signal_t)GPIO_SIGNAL_BLACKPILL_A12, GPIO_OSPEED_VERY_HIGH);

	/* disable the interrupts */
	USBFS->GAHBCFG &= ~USB_GAHBCFG_GINT;
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
	for (int i = 0; i < elems(USBFS->DIEPTXF); i++)
		USBFS->DIEPTXF[i] = 0;

	/* initialize device */
	/* Deactivate VBUS Sensing B */
	USBFS->GCCFG &= ~USB_GCCFG_NOVBUSSENS;
	/* B-peripheral session valid override enable */
	// USBFS->GOTGCTL |= USB_GOTGCTL_BVALOEN; //TODO: ?
	// USBFS->GOTGCTL |= USB_GOTGCTL_BVALOVAL; //TODO: ?
	/* Restart the Phy Clock */
	USBFS->PCGCCTL = 0;
	/* Device mode configuration */
	USBFS->DCFG |= 0; //TODO: ???
	/* Set Full speed phy */
	USBFS->DCFG |= USB_DCFG_DSPD_0 | USB_DCFG_DSPD_1;

	/* flush the fifos */
	/* Tx fifo */
	USBFS->GRSTCTL = USB_GRSTCTL_TXFFLSH | 0x10 << LSB(USB_GRSTCTL_TXFNUM);
	/* wait for flush to complete */
	while ((USBFS->GRSTCTL & USB_GRSTCTL_TXFFLSH) != 0);
	/* flush receive fifo */
	USBFS->GRSTCTL = USB_GRSTCTL_RXFFLSH;
	/* wait for flush to complete */
	while ((USBFS->GRSTCTL & USB_GRSTCTL_RXFFLSH) != 0);

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
					  USB_GINTMSK_WUIM;


	/* report status */
	return EOK;
}
