/**
 * @file otg.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-10-25
 *
 * @copyright Copyright (c) 2024
 */

#ifndef STM32F401_OTG_H
#define STM32F401_OTG_H

#include "stm32f401.h"

/* registers bases */
#define USBFS_BASE                      0x50000000

/* core registers */
#define USBFS                           ((usb_t *)USBFS_BASE)

/* in-endpoints - full speed */
#define USBFS_IE(x)                     ((usb_ie_t *)(USBFS_BASE + 0x900 + \
                                            0x20 * (x)))
/* out endpoints - full speed */
#define USBFS_OE(x)                     ((usb_oe_t *)(USBFS_BASE + 0xB00 + \
                                            0x20 * (x)))
/* host channels - full speed */
#define USBFS_HC(x)                     ((usb_hc_t *)(USBFS_BASE + 0xB00 + \
                                            0x20 * (x)))
/* usb fifo specific push/pop register */
#define USBFS_FIFO(x)                    *((reg32_t *)(USBFS_BASE + 0x1000 + \
                                            0x1000 * (x)))

/* usb common registers */
typedef struct {
    reg32_t GOTGCTL;
    reg32_t GOTGINT;
    reg32_t GAHBCFG;
    reg32_t GUSBCFG;
    reg32_t GRSTCTL;
    reg32_t GINTSTS;
    reg32_t GINTMSK;
    reg32_t GRXSTSR;
    reg32_t GRXSTSP;
    reg32_t GRXFSIZ;
    reg32_t DIEPTXF0_HNPTXFSIZ;
    reg32_t HNPTXSTS;
    reg32_t RESERVED0[2];
    reg32_t GCCFG;
    reg32_t CID;
    reg32_t RESERVED1[48];
    reg32_t HPTXFSIZ;
    reg32_t DIEPTXF[0x0F];

    /* host registers */
    reg32_t HCFG;
    reg32_t HFIR;
    reg32_t HFNUM;
    reg32_t RESERVED50;
    reg32_t HPTXSTS;
    reg32_t HAINT;
    reg32_t HAINTMSK;

    /* device mode registers */
    reg32_t DCFG;
    reg32_t DCTL;
    reg32_t DSTS;
    reg32_t RESERVED2;
    reg32_t DIEPMSK;
    reg32_t DOEPMSK;
    reg32_t DAINT;
    reg32_t DAINTMSK;
    reg32_t RESERVED3;
    reg32_t RESERVED4;
    reg32_t DVBUSDIS;
    reg32_t DVBUSPULSE;
    reg32_t DTHRCTL;
    reg32_t DIEPEMPMSK;
    reg32_t DEACHINT;
    reg32_t DEACHMSK;
    reg32_t RESERVED5;
    reg32_t DINEP1MSK;
    reg32_t RESERVED6[15];
    reg32_t DOUTEP1MSK;
	reg32_t RESERVED7[350];

    /* power and clock gating */
	reg32_t PCGCCTL;
} usb_t;

/** USB_IN_Endpoint    -Specific_Registers */
    typedef struct {
    reg32_t DIEPCTL;
    reg32_t RESERVED04;
    reg32_t DIEPINT;
    reg32_t RESERVED0C;
    reg32_t DIEPTSIZ;
    reg32_t DIEPDMA;
    reg32_t DTXFSTS;
    reg32_t RESERVED18;
} usb_ie_t;

/** USB_OUT_Endpoint    -Specific_Registers */
typedef struct {
    reg32_t DOEPCTL;
    reg32_t RESERVED04;
    reg32_t DOEPINT;
    reg32_t RESERVED0C;
    reg32_t DOEPTSIZ;
    reg32_t DOEPDMA;
    reg32_t RESERVED18[2];
} usb_oe_t;

/* host channel specific registers */
typedef struct {
    reg32_t HCCHAR;
    reg32_t HCSPLT;
    reg32_t HCINT;
    reg32_t HCINTMSK;
    reg32_t HCTSIZ;
    reg32_t HCDMA;
    reg32_t RESERVED[2];
} usb_hc_t;


/********************  Bit definition for USB_GOTGCTL     register  ***********/
#define USB_GOTGCTL_SRQSCS                                   0x00000001
#define USB_GOTGCTL_SRQ                                      0x00000002
#define USB_GOTGCTL_HNGSCS                                   0x00000100
#define USB_GOTGCTL_HNPRQ                                    0x00000200
#define USB_GOTGCTL_HSHNPEN                                  0x00000400
#define USB_GOTGCTL_DHNPEN                                   0x00000800
#define USB_GOTGCTL_CIDSTS                                   0x00010000
#define USB_GOTGCTL_DBCT                                     0x00020000
#define USB_GOTGCTL_ASVLD                                    0x00040000
#define USB_GOTGCTL_BSVLD                                    0x00080000

/********************  Bit definition forUSB_HCFG     register  ********************/

#define USB_HCFG_FSLSPCS                                     0x00000003
#define USB_HCFG_FSLSPCS_0                                   0x00000001
#define USB_HCFG_FSLSPCS_1                                   0x00000002
#define USB_HCFG_FSLSS                                       0x00000004

/********************  Bit definition for USB_DCFG     register  ********************/

#define USB_DCFG_DSPD                                        0x00000003
#define USB_DCFG_DSPD_0                                      0x00000001
#define USB_DCFG_DSPD_1                                      0x00000002
#define USB_DCFG_NZLSOHSK                                    0x00000004
#define USB_DCFG_DAD                                         0x000007f0
#define USB_DCFG_DAD_0                                       0x00000010
#define USB_DCFG_DAD_1                                       0x00000020
#define USB_DCFG_DAD_2                                       0x00000040
#define USB_DCFG_DAD_3                                       0x00000080
#define USB_DCFG_DAD_4                                       0x00000100
#define USB_DCFG_DAD_5                                       0x00000200
#define USB_DCFG_DAD_6                                       0x00000400
#define USB_DCFG_PFIVL                                       0x00001800
#define USB_DCFG_PFIVL_0                                     0x00000800
#define USB_DCFG_PFIVL_1                                     0x00001000
#define USB_DCFG_XCVRDLY                                     0x00004000
#define USB_DCFG_ERRATIM                                     0x00008000
#define USB_DCFG_PERSCHIVL                                   0x03000000
#define USB_DCFG_PERSCHIVL_0                                 0x01000000
#define USB_DCFG_PERSCHIVL_1                                 0x02000000

/********************  Bit definition for USB_PCGCR     register  ********************/
#define USB_PCGCR_STPPCLK                                    0x00000001
#define USB_PCGCR_GATEHCLK                                   0x00000002
#define USB_PCGCR_PHYSUSP                                    0x00000010

/********************  Bit definition for USB_GOTGINT     register  ********************/
#define USB_GOTGINT_SEDET                                    0x00000004
#define USB_GOTGINT_SRSSCHG                                  0x00000100
#define USB_GOTGINT_HNSSCHG                                  0x00000200
#define USB_GOTGINT_HNGDET                                   0x00020000
#define USB_GOTGINT_ADTOCHG                                  0x00040000
#define USB_GOTGINT_DBCDNE                                   0x00080000

/********************  Bit definition for USB_DCTL     register  ********************/
#define USB_DCTL_RWUSIG                                      0x00000001
#define USB_DCTL_SDIS                                        0x00000002
#define USB_DCTL_GINSTS                                      0x00000004
#define USB_DCTL_GONSTS                                      0x00000008
#define USB_DCTL_TCTL                                        0x00000070
#define USB_DCTL_TCTL_0                                      0x00000010
#define USB_DCTL_TCTL_1                                      0x00000020
#define USB_DCTL_TCTL_2                                      0x00000040
#define USB_DCTL_SGINAK                                      0x00000080
#define USB_DCTL_CGINAK                                      0x00000100
#define USB_DCTL_SGONAK                                      0x00000200
#define USB_DCTL_CGONAK                                      0x00000400
#define USB_DCTL_POPRGDNE                                    0x00000800

/********************  Bit definition for USB_HFIR     register  ********************/
#define USB_HFIR_FRIVL                                       0x0000ffff

/********************  Bit definition for USB_HFNUM     register  ********************/
#define USB_HFNUM_FRNUM                                      0x0000ffff
#define USB_HFNUM_FTREM                                      0xffff0000

/********************  Bit definition for USB_DSTS     register  ********************/
#define USB_DSTS_SUSPSTS                                     0x00000001
#define USB_DSTS_ENUMSPD                                     0x00000006
#define USB_DSTS_ENUMSPD_0                                   0x00000002
#define USB_DSTS_ENUMSPD_1                                   0x00000004
#define USB_DSTS_EERR                                        0x00000008
#define USB_DSTS_FNSOF                                       0x003fff00

/********************  Bit definition for USB_GAHBCFG     register  ********************/
#define USB_GAHBCFG_GINT                                     0x00000001
#define USB_GAHBCFG_HBSTLEN                                  0x0000001e
#define USB_GAHBCFG_HBSTLEN_0                    (0x0L << USB_GAHBCFG_HBSTLEN    )
#define USB_GAHBCFG_HBSTLEN_1                    (0x1L << USB_GAHBCFG_HBSTLEN    )
#define USB_GAHBCFG_HBSTLEN_2                    (0x3L << USB_GAHBCFG_HBSTLEN    )
#define USB_GAHBCFG_HBSTLEN_3                    (0x5L << USB_GAHBCFG_HBSTLEN    )
#define USB_GAHBCFG_HBSTLEN_4                    (0x7L << USB_GAHBCFG_HBSTLEN    )
#define USB_GAHBCFG_DMAEN                                    0x00000020
#define USB_GAHBCFG_TXFELVL                                  0x00000080
#define USB_GAHBCFG_PTXFELVL                                 0x00000100

/********************  Bit definition for USB_GUSBCFG     register  ********************/

#define USB_GUSBCFG_TOCAL                                    0x00000007
#define USB_GUSBCFG_TOCAL_0                                  0x00000001
#define USB_GUSBCFG_TOCAL_1                                  0x00000002
#define USB_GUSBCFG_TOCAL_2                                  0x00000004
#define USB_GUSBCFG_PHYSEL                                   0x00000040
#define USB_GUSBCFG_SRPCAP                                   0x00000100
#define USB_GUSBCFG_HNPCAP                                   0x00000200
#define USB_GUSBCFG_TRDT                                     0x00003c00
#define USB_GUSBCFG_TRDT_0                                   0x00000400
#define USB_GUSBCFG_TRDT_1                                   0x00000800
#define USB_GUSBCFG_TRDT_2                                   0x00001000
#define USB_GUSBCFG_TRDT_3                                   0x00002000
#define USB_GUSBCFG_PHYLPCS                                  0x00008000
#define USB_GUSBCFG_ULPIFSLS                                 0x00020000
#define USB_GUSBCFG_ULPIAR                                   0x00040000
#define USB_GUSBCFG_ULPICSM                                  0x00080000
#define USB_GUSBCFG_ULPIEVBUSD                               0x00100000
#define USB_GUSBCFG_ULPIEVBUSI                               0x00200000
#define USB_GUSBCFG_TSDPS                                    0x00400000
#define USB_GUSBCFG_PCCI                                     0x00800000
#define USB_GUSBCFG_PTCI                                     0x01000000
#define USB_GUSBCFG_ULPIIPD                                  0x02000000
#define USB_GUSBCFG_FHMOD                                    0x20000000
#define USB_GUSBCFG_FDMOD                                    0x40000000
#define USB_GUSBCFG_CTXPKT                                   0x80000000

/********************  Bit definition for USB_GRSTCTL     register  ********************/
#define USB_GRSTCTL_CSRST                                    0x00000001
#define USB_GRSTCTL_HSRST                                    0x00000002
#define USB_GRSTCTL_FCRST                                    0x00000004
#define USB_GRSTCTL_RXFFLSH                                  0x00000010
#define USB_GRSTCTL_TXFFLSH                                  0x00000020
#define USB_GRSTCTL_TXFNUM                                   0x000007c0
#define USB_GRSTCTL_TXFNUM_0                                 0x00000040
#define USB_GRSTCTL_TXFNUM_1                                 0x00000080
#define USB_GRSTCTL_TXFNUM_2                                 0x00000100
#define USB_GRSTCTL_TXFNUM_3                                 0x00000200
#define USB_GRSTCTL_TXFNUM_4                                 0x00000400
#define USB_GRSTCTL_DMAREQ                                   0x40000000
#define USB_GRSTCTL_AHBIDL                                   0x80000000

/********************  Bit definition for USB_DIEPMSK     register  ********************/
#define USB_DIEPMSK_XFRCM                                    0x00000001
#define USB_DIEPMSK_EPDM                                     0x00000002
#define USB_DIEPMSK_TOM                                      0x00000008
#define USB_DIEPMSK_ITTXFEMSK                                0x00000010
#define USB_DIEPMSK_INEPNMM                                  0x00000020
#define USB_DIEPMSK_INEPNEM                                  0x00000040
#define USB_DIEPMSK_TXFURM                                   0x00000100
#define USB_DIEPMSK_BIM                                      0x00000200

/********************  Bit definition for USB_HPTXSTS     register  ********************/
#define USB_HPTXSTS_PTXFSAVL                                 0x0000ffff
#define USB_HPTXSTS_PTXQSAV                                  0x00ff0000
#define USB_HPTXSTS_PTXQSAV_0                                0x00010000
#define USB_HPTXSTS_PTXQSAV_1                                0x00020000
#define USB_HPTXSTS_PTXQSAV_2                                0x00040000
#define USB_HPTXSTS_PTXQSAV_3                                0x00080000
#define USB_HPTXSTS_PTXQSAV_4                                0x00100000
#define USB_HPTXSTS_PTXQSAV_5                                0x00200000
#define USB_HPTXSTS_PTXQSAV_6                                0x00400000
#define USB_HPTXSTS_PTXQSAV_7                                0x00800000
#define USB_HPTXSTS_PTXQTOP                                  0xff000000
#define USB_HPTXSTS_PTXQTOP_0                                0x01000000
#define USB_HPTXSTS_PTXQTOP_1                                0x02000000
#define USB_HPTXSTS_PTXQTOP_2                                0x04000000
#define USB_HPTXSTS_PTXQTOP_3                                0x08000000
#define USB_HPTXSTS_PTXQTOP_4                                0x10000000
#define USB_HPTXSTS_PTXQTOP_5                                0x20000000
#define USB_HPTXSTS_PTXQTOP_6                                0x40000000
#define USB_HPTXSTS_PTXQTOP_7                                0x80000000

/********************  Bit definition for USB_HAINT     register  ********************/
#define USB_HAINT_HAINT                                      0x0000ffff

/********************  Bit definition for USB_DOEPMSK     register  ********************/
#define USB_DOEPMSK_XFRCM                                    0x00000001
#define USB_DOEPMSK_EPDM                                     0x00000002
#define USB_DOEPMSK_AHBERRM                                  0x00000004
#define USB_DOEPMSK_STUPM                                    0x00000008
#define USB_DOEPMSK_OTEPDM                                   0x00000010
#define USB_DOEPMSK_OTEPSPRM                                 0x00000020
#define USB_DOEPMSK_B2BSTUP                                  0x00000040
#define USB_DOEPMSK_OPEM                                     0x00000100
#define USB_DOEPMSK_BOIM                                     0x00000200
#define USB_DOEPMSK_BERRM                                    0x00001000
#define USB_DOEPMSK_NAKM                                     0x00002000
#define USB_DOEPMSK_NYETM                                    0x00004000
/********************  Bit definition for USB_GINTSTS     register  ********************/
#define USB_GINTSTS_CMOD                                     0x00000001
#define USB_GINTSTS_MMIS                                     0x00000002
#define USB_GINTSTS_OTGINT                                   0x00000004
#define USB_GINTSTS_SOF                                      0x00000008
#define USB_GINTSTS_RXFLVL                                   0x00000010
#define USB_GINTSTS_NPTXFE                                   0x00000020
#define USB_GINTSTS_GINAKEFF                                 0x00000040
#define USB_GINTSTS_BOUTNAKEFF                               0x00000080
#define USB_GINTSTS_ESUSP                                    0x00000400
#define USB_GINTSTS_USBSUSP                                  0x00000800
#define USB_GINTSTS_USBRST                                   0x00001000
#define USB_GINTSTS_ENUMDNE                                  0x00002000
#define USB_GINTSTS_ISOODRP                                  0x00004000
#define USB_GINTSTS_EOPF                                     0x00008000
#define USB_GINTSTS_IEPINT                                   0x00040000
#define USB_GINTSTS_OEPINT                                   0x00080000
#define USB_GINTSTS_IISOIXFR                                 0x00100000
#define USB_GINTSTS_PXFR_INCOMPISOOUT                        0x00200000
#define USB_GINTSTS_DATAFSUSP                                0x00400000
#define USB_GINTSTS_HPRTINT                                  0x01000000
#define USB_GINTSTS_HCINT                                    0x02000000
#define USB_GINTSTS_PTXFE                                    0x04000000
#define USB_GINTSTS_CIDSCHG                                  0x10000000
#define USB_GINTSTS_DISCINT                                  0x20000000
#define USB_GINTSTS_SRQINT                                   0x40000000
#define USB_GINTSTS_WKUINT                                   0x80000000

/********************  Bit definition for USB_GINTMSK     register  ********************/
#define USB_GINTMSK_MMISM                                    0x00000002
#define USB_GINTMSK_OTGINT                                   0x00000004
#define USB_GINTMSK_SOFM                                     0x00000008
#define USB_GINTMSK_RXFLVLM                                  0x00000010
#define USB_GINTMSK_NPTXFEM                                  0x00000020
#define USB_GINTMSK_GINAKEFFM                                0x00000040
#define USB_GINTMSK_GONAKEFFM                                0x00000080
#define USB_GINTMSK_ESUSPM                                   0x00000400
#define USB_GINTMSK_USBSUSPM                                 0x00000800
#define USB_GINTMSK_USBRST                                   0x00001000
#define USB_GINTMSK_ENUMDNEM                                 0x00002000
#define USB_GINTMSK_ISOODRPM                                 0x00004000
#define USB_GINTMSK_EOPFM                                    0x00008000
#define USB_GINTMSK_EPMISM                                   0x00020000
#define USB_GINTMSK_IEPINT                                   0x00040000
#define USB_GINTMSK_OEPINT                                   0x00080000
#define USB_GINTMSK_IISOIXFRM                                0x00100000
#define USB_GINTMSK_PXFRM_IISOOXFRM                          0x00200000
#define USB_GINTMSK_FSUSPM                                   0x00400000
#define USB_GINTMSK_PRTIM                                    0x01000000
#define USB_GINTMSK_HCIM                                     0x02000000
#define USB_GINTMSK_PTXFEM                                   0x04000000
#define USB_GINTMSK_CIDSCHGM                                 0x10000000
#define USB_GINTMSK_DISCINT                                  0x20000000
#define USB_GINTMSK_SRQIM                                    0x40000000
#define USB_GINTMSK_WUIM                                     0x80000000

/********************  Bit definition for USB_DAINT     register  ********************/
#define USB_DAINT_IEPINT                                     0x0000ffff
#define USB_DAINT_OEPINT                                     0xffff0000

/********************  Bit definition for USB_HAINTMSK     register  ********************/
#define USB_HAINTMSK_HAINTM                                  0x0000ffff

/********************  Bit definition for USB_GRXSTSP     register  ********************/
#define USB_GRXSTSP_EPNUM                                    0x0000000f
#define USB_GRXSTSP_BCNT                                     0x00007ff0
#define USB_GRXSTSP_DPID                                     0x00018000
#define USB_GRXSTSP_PKTSTS                                   0x001e0000

/********************  Bit definition for USB_DAINTMSK     register  ********************/
#define USB_DAINTMSK_IEPM                                    0x0000ffff
#define USB_DAINTMSK_OEPM                                    0xffff0000

/********************  Bit definition for USB_GRXFSIZ     register  ********************/
#define USB_GRXFSIZ_RXFD                                     0x0000ffff

/********************  Bit definition for USB_DVBUSDIS     register  ********************/
#define USB_DVBUSDIS_VBUSDT                                  0x0000ffff

/********************  Bit definition for OTG register  ********************/
#define USB_NPTXFSA                                          0x0000ffff
#define USB_NPTXFD                                           0xffff0000
#define USB_TX0FSA                                           0x0000ffff
#define USB_TX0FD                                            0xffff0000

/********************  Bit definition forUSB_DVBUSPULSE     register  ********************/
#define USB_DVBUSPULSE_DVBUSP                                0x00000fff

/********************  Bit definition for USB_GNPTXSTS     register  ********************/
#define USB_GNPTXSTS_NPTXFSAV                                0x0000ffff
#define USB_GNPTXSTS_NPTQXSAV                                0x00ff0000
#define USB_GNPTXSTS_NPTQXSAV_0                              0x00010000
#define USB_GNPTXSTS_NPTQXSAV_1                              0x00020000
#define USB_GNPTXSTS_NPTQXSAV_2                              0x00040000
#define USB_GNPTXSTS_NPTQXSAV_3                              0x00080000
#define USB_GNPTXSTS_NPTQXSAV_4                              0x00100000
#define USB_GNPTXSTS_NPTQXSAV_5                              0x00200000
#define USB_GNPTXSTS_NPTQXSAV_6                              0x00400000
#define USB_GNPTXSTS_NPTQXSAV_7                              0x00800000
#define USB_GNPTXSTS_NPTXQTOP                                0x7f000000
#define USB_GNPTXSTS_NPTXQTOP_0                              0x01000000
#define USB_GNPTXSTS_NPTXQTOP_1                              0x02000000
#define USB_GNPTXSTS_NPTXQTOP_2                              0x04000000
#define USB_GNPTXSTS_NPTXQTOP_3                              0x08000000
#define USB_GNPTXSTS_NPTXQTOP_4                              0x10000000
#define USB_GNPTXSTS_NPTXQTOP_5                              0x20000000
#define USB_GNPTXSTS_NPTXQTOP_6                              0x40000000

/********************  Bit definition for USB_DTHRCTL     register  ********************/
#define USB_DTHRCTL_NONISOTHREN                              0x00000001
#define USB_DTHRCTL_ISOTHREN                                 0x00000002
#define USB_DTHRCTL_TXTHRLEN                                 0x000007fc
#define USB_DTHRCTL_TXTHRLEN_0                               0x00000004
#define USB_DTHRCTL_TXTHRLEN_1                               0x00000008
#define USB_DTHRCTL_TXTHRLEN_2                               0x00000010
#define USB_DTHRCTL_TXTHRLEN_3                               0x00000020
#define USB_DTHRCTL_TXTHRLEN_4                               0x00000040
#define USB_DTHRCTL_TXTHRLEN_5                               0x00000080
#define USB_DTHRCTL_TXTHRLEN_6                               0x00000100
#define USB_DTHRCTL_TXTHRLEN_7                               0x00000200
#define USB_DTHRCTL_TXTHRLEN_8                               0x00000400
#define USB_DTHRCTL_RXTHREN                                  0x00010000
#define USB_DTHRCTL_RXTHRLEN                                 0x03fe0000
#define USB_DTHRCTL_RXTHRLEN_0                               0x00020000
#define USB_DTHRCTL_RXTHRLEN_1                               0x00040000
#define USB_DTHRCTL_RXTHRLEN_2                               0x00080000
#define USB_DTHRCTL_RXTHRLEN_3                               0x00100000
#define USB_DTHRCTL_RXTHRLEN_4                               0x00200000
#define USB_DTHRCTL_RXTHRLEN_5                               0x00400000
#define USB_DTHRCTL_RXTHRLEN_6                               0x00800000
#define USB_DTHRCTL_RXTHRLEN_7                               0x01000000
#define USB_DTHRCTL_RXTHRLEN_8                               0x02000000
#define USB_DTHRCTL_ARPEN                                    0x08000000

/********************  Bit definition for USB_DIEPEMPMSK     register  ********************/
#define USB_DIEPEMPMSK_INEPTXFEM                             0x0000ffff

/********************  Bit definition for USB_DEACHINT     register  ********************/
#define USB_DEACHINT_IEP1INT                                 0x00000002
#define USB_DEACHINT_OEP1INT                                 0x00020000

/********************  Bit definition for USB_GCCFG     register  ********************/
#define USB_GCCFG_PWRDWN                                     0x00010000
#define USB_GCCFG_I2CPADEN                                   0x00020000
#define USB_GCCFG_VBUSASEN                                   0x00040000
#define USB_GCCFG_VBUSBSEN                                   0x00080000
#define USB_GCCFG_SOFOUTEN                                   0x00100000
#define USB_GCCFG_NOVBUSSENS                                 0x00200000

/********************  Bit definition forUSB_DEACHINTMSK     register  ********************/
#define USB_DEACHINTMSK_IEP1INTM                             0x00000002
#define USB_DEACHINTMSK_OEP1INTM                             0x00020000

/********************  Bit definition for USB_CID     register  ********************/
#define USB_CID_PRODUCT_ID                                   0xffffffff

/********************  Bit definition for USB_DIEPEACHMSK1     register  ********************/
#define USB_DIEPEACHMSK1_XFRCM                               0x00000001
#define USB_DIEPEACHMSK1_EPDM                                0x00000002
#define USB_DIEPEACHMSK1_TOM                                 0x00000008
#define USB_DIEPEACHMSK1_ITTXFEMSK                           0x00000010
#define USB_DIEPEACHMSK1_INEPNMM                             0x00000020
#define USB_DIEPEACHMSK1_INEPNEM                             0x00000040
#define USB_DIEPEACHMSK1_TXFURM                              0x00000100
#define USB_DIEPEACHMSK1_BIM                                 0x00000200
#define USB_DIEPEACHMSK1_NAKM                                0x00002000

/********************  Bit definition for USB_HPRT     register  ********************/
#define USB_HPRT_PCSTS                                       0x00000001
#define USB_HPRT_PCDET                                       0x00000002
#define USB_HPRT_PENA                                        0x00000004
#define USB_HPRT_PENCHNG                                     0x00000008
#define USB_HPRT_POCA                                        0x00000010
#define USB_HPRT_POCCHNG                                     0x00000020
#define USB_HPRT_PRES                                        0x00000040
#define USB_HPRT_PSUSP                                       0x00000080
#define USB_HPRT_PRST                                        0x00000100
#define USB_HPRT_PLSTS                                       0x00000c00
#define USB_HPRT_PLSTS_0                                     0x00000400
#define USB_HPRT_PLSTS_1                                     0x00000800
#define USB_HPRT_PPWR                                        0x00001000
#define USB_HPRT_PTCTL                                       0x0001e000
#define USB_HPRT_PTCTL_0                                     0x00002000
#define USB_HPRT_PTCTL_1                                     0x00004000
#define USB_HPRT_PTCTL_2                                     0x00008000
#define USB_HPRT_PTCTL_3                                     0x00010000
#define USB_HPRT_PSPD                                        0x00060000
#define USB_HPRT_PSPD_0                                      0x00020000
#define USB_HPRT_PSPD_1                                      0x00040000

/********************  Bit definition for USB_DOEPEACHMSK1     register  ********************/
#define USB_DOEPEACHMSK1_XFRCM                               0x00000001
#define USB_DOEPEACHMSK1_EPDM                                0x00000002
#define USB_DOEPEACHMSK1_TOM                                 0x00000008
#define USB_DOEPEACHMSK1_ITTXFEMSK                           0x00000010
#define USB_DOEPEACHMSK1_INEPNMM                             0x00000020
#define USB_DOEPEACHMSK1_INEPNEM                             0x00000040
#define USB_DOEPEACHMSK1_TXFURM                              0x00000100
#define USB_DOEPEACHMSK1_BIM                                 0x00000200
#define USB_DOEPEACHMSK1_BERRM                               0x00001000
#define USB_DOEPEACHMSK1_NAKM                                0x00002000
#define USB_DOEPEACHMSK1_NYETM                               0x00004000

/********************  Bit definition for USB_HPTXFSIZ     register  ********************/
#define USB_HPTXFSIZ_PTXSA                                   0x0000ffff
#define USB_HPTXFSIZ_PTXFD                                   0xffff0000

/********************  Bit definition for USB_DIEPCTL     register  ********************/
#define USB_DIEPCTL_MPSIZ                                    0x000007ff
#define USB_DIEPCTL_USBAEP                                   0x00008000
#define USB_DIEPCTL_EONUM_DPID                               0x00010000
#define USB_DIEPCTL_NAKSTS                                   0x00020000
#define USB_DIEPCTL_EPTYP                                    0x000c0000
#define USB_DIEPCTL_EPTYP_0                                  0x00040000
#define USB_DIEPCTL_EPTYP_1                                  0x00080000
#define USB_DIEPCTL_STALL                                    0x00200000
#define USB_DIEPCTL_TXFNUM                                   0x03c00000
#define USB_DIEPCTL_TXFNUM_0                                 0x00400000
#define USB_DIEPCTL_TXFNUM_1                                 0x00800000
#define USB_DIEPCTL_TXFNUM_2                                 0x01000000
#define USB_DIEPCTL_TXFNUM_3                                 0x02000000
#define USB_DIEPCTL_CNAK                                     0x04000000
#define USB_DIEPCTL_SNAK                                     0x08000000
#define USB_DIEPCTL_SD0PID_SEVNFRM                           0x10000000
#define USB_DIEPCTL_SODDFRM                                  0x20000000
#define USB_DIEPCTL_EPDIS                                    0x40000000
#define USB_DIEPCTL_EPENA                                    0x80000000

/********************  Bit definition for USB_HCCHAR     register  ********************/
#define USB_HCCHAR_MPSIZ                                     0x000007ff
#define USB_HCCHAR_EPNUM                                     0x00007800
#define USB_HCCHAR_EPNUM_0                                   0x00000800
#define USB_HCCHAR_EPNUM_1                                   0x00001000
#define USB_HCCHAR_EPNUM_2                                   0x00002000
#define USB_HCCHAR_EPNUM_3                                   0x00004000
#define USB_HCCHAR_EPDIR                                     0x00008000
#define USB_HCCHAR_LSDEV                                     0x00020000
#define USB_HCCHAR_EPTYP                                     0x000c0000
#define USB_HCCHAR_EPTYP_0                                   0x00040000
#define USB_HCCHAR_EPTYP_1                                   0x00080000
#define USB_HCCHAR_MC                                        0x00300000
#define USB_HCCHAR_MC_0                                      0x00100000
#define USB_HCCHAR_MC_1                                      0x00200000
#define USB_HCCHAR_DAD                                       0x1fc00000
#define USB_HCCHAR_DAD_0                                     0x00400000
#define USB_HCCHAR_DAD_1                                     0x00800000
#define USB_HCCHAR_DAD_2                                     0x01000000
#define USB_HCCHAR_DAD_3                                     0x02000000
#define USB_HCCHAR_DAD_4                                     0x04000000
#define USB_HCCHAR_DAD_5                                     0x08000000
#define USB_HCCHAR_DAD_6                                     0x10000000
#define USB_HCCHAR_ODDFRM                                    0x20000000
#define USB_HCCHAR_CHDIS                                     0x40000000
#define USB_HCCHAR_CHENA                                     0x80000000

/********************  Bit definition for USB_HCSPLT     register  ********************/

#define USB_HCSPLT_PRTADDR                                   0x0000007f
#define USB_HCSPLT_PRTADDR_0                                 0x00000001
#define USB_HCSPLT_PRTADDR_1                                 0x00000002
#define USB_HCSPLT_PRTADDR_2                                 0x00000004
#define USB_HCSPLT_PRTADDR_3                                 0x00000008
#define USB_HCSPLT_PRTADDR_4                                 0x00000010
#define USB_HCSPLT_PRTADDR_5                                 0x00000020
#define USB_HCSPLT_PRTADDR_6                                 0x00000040
#define USB_HCSPLT_HUBADDR                                   0x00003f80
#define USB_HCSPLT_HUBADDR_0                                 0x00000080
#define USB_HCSPLT_HUBADDR_1                                 0x00000100
#define USB_HCSPLT_HUBADDR_2                                 0x00000200
#define USB_HCSPLT_HUBADDR_3                                 0x00000400
#define USB_HCSPLT_HUBADDR_4                                 0x00000800
#define USB_HCSPLT_HUBADDR_5                                 0x00001000
#define USB_HCSPLT_HUBADDR_6                                 0x00002000
#define USB_HCSPLT_XACTPOS                                   0x0000c000
#define USB_HCSPLT_XACTPOS_0                                 0x00004000
#define USB_HCSPLT_XACTPOS_1                                 0x00008000
#define USB_HCSPLT_COMPLSPLT                                 0x00010000
#define USB_HCSPLT_SPLITEN                                   0x80000000

/********************  Bit definition for USB_HCINT     register  ********************/
#define USB_HCINT_XFRC                                       0x00000001
#define USB_HCINT_CHH                                        0x00000002
#define USB_HCINT_AHBERR                                     0x00000004
#define USB_HCINT_STALL                                      0x00000008
#define USB_HCINT_NAK                                        0x00000010
#define USB_HCINT_ACK                                        0x00000020
#define USB_HCINT_NYET                                       0x00000040
#define USB_HCINT_TXERR                                      0x00000080
#define USB_HCINT_BBERR                                      0x00000100
#define USB_HCINT_FRMOR                                      0x00000200
#define USB_HCINT_DTERR                                      0x00000400

/********************  Bit definition for USB_DIEPINT     register  ********************/
#define USB_DIEPINT_XFRC                                     0x00000001
#define USB_DIEPINT_EPDISD                                   0x00000002
#define USB_DIEPINT_AHBERR                                   0x00000004
#define USB_DIEPINT_TOC                                      0x00000008
#define USB_DIEPINT_ITTXFE                                   0x00000010
#define USB_DIEPINT_INEPNM                                   0x00000004
#define USB_DIEPINT_INEPNE                                   0x00000040
#define USB_DIEPINT_TXFE                                     0x00000080
#define USB_DIEPINT_TXFIFOUDRN                               0x00000100
#define USB_DIEPINT_BNA                                      0x00000200
#define USB_DIEPINT_PKTDRPSTS                                0x00000800
#define USB_DIEPINT_BERR                                     0x00001000
#define USB_DIEPINT_NAK                                      0x00002000

/********************  Bit definition forUSB_HCINTMSK     register  ********************/
#define USB_HCINTMSK_XFRCM                                   0x00000001
#define USB_HCINTMSK_CHHM                                    0x00000002
#define USB_HCINTMSK_AHBERR                                  0x00000004
#define USB_HCINTMSK_STALLM                                  0x00000008
#define USB_HCINTMSK_NAKM                                    0x00000010
#define USB_HCINTMSK_ACKM                                    0x00000020
#define USB_HCINTMSK_NYET                                    0x00000040
#define USB_HCINTMSK_TXERRM                                  0x00000080
#define USB_HCINTMSK_BBERRM                                  0x00000100
#define USB_HCINTMSK_FRMORM                                  0x00000200
#define USB_HCINTMSK_DTERRM                                  0x00000400

/********************  Bit definition for USB_DIEPTSIZ     register  ********************/

#define USB_DIEPTSIZ_XFRSIZ                                  0x0007ffff
#define USB_DIEPTSIZ_PKTCNT                                  0x1ff80000
#define USB_DIEPTSIZ_MULCNT                                  0x60000000
/********************  Bit definition for USB_HCTSIZ     register  ********************/
#define USB_HCTSIZ_XFRSIZ                                    0x0007ffff
#define USB_HCTSIZ_PKTCNT                                    0x1ff80000
#define USB_HCTSIZ_DOPING                                    0x80000000
#define USB_HCTSIZ_DPID                                      0x60000000
#define USB_HCTSIZ_DPID_0                                    0x20000000
#define USB_HCTSIZ_DPID_1                                    0x40000000

/********************  Bit definition for USB_DIEPDMA     register  ********************/
#define USB_DIEPDMA_DMAADDR                                  0xffffffff

/********************  Bit definition for USB_HCDMA     register  ********************/
#define USB_HCDMA_DMAADDR                                    0xffffffff

/********************  Bit definition for USB_DTXFSTS     register  ********************/
#define USB_DTXFSTS_INEPTFSAV                                0x0000ffff

/********************  Bit definition for USB_DIEPTXF     register  ********************/
#define USB_DIEPTXF_INEPTXSA                                 0x0000ffff
#define USB_DIEPTXF_INEPTXFD                                 0xffff0000

/********************  Bit definition for USB_DOEPCTL     register  ********************/

#define USB_DOEPCTL_MPSIZ                                    0x000007ff
#define USB_DOEPCTL_USBAEP                                   0x00008000
#define USB_DOEPCTL_NAKSTS                                   0x00020000
#define USB_DOEPCTL_SD0PID_SEVNFRM                           0x10000000
#define USB_DOEPCTL_SODDFRM                                  0x20000000
#define USB_DOEPCTL_EPTYP                                    0x000c0000
#define USB_DOEPCTL_EPTYP_0                                  0x00040000
#define USB_DOEPCTL_EPTYP_1                                  0x00080000
#define USB_DOEPCTL_SNPM                                     0x00100000
#define USB_DOEPCTL_STALL                                    0x00200000
#define USB_DOEPCTL_CNAK                                     0x04000000
#define USB_DOEPCTL_SNAK                                     0x08000000
#define USB_DOEPCTL_EPDIS                                    0x40000000
#define USB_DOEPCTL_EPENA                                    0x80000000

/********************  Bit definition for USB_DOEPINT     register  ********************/
#define USB_DOEPINT_XFRC                                     0x00000001
#define USB_DOEPINT_EPDISD                                   0x00000002
#define USB_DOEPINT_AHBERR                                   0x00000004
#define USB_DOEPINT_STUP                                     0x00000008
#define USB_DOEPINT_OTEPDIS                                  0x00000010
#define USB_DOEPINT_OTEPSPR                                  0x00000020
#define USB_DOEPINT_B2BSTUP                                  0x00000040
#define USB_DOEPINT_OUTPKTERR                                0x00000100
#define USB_DOEPINT_NAK                                      0x00002000
#define USB_DOEPINT_NYET                                     0x00004000
#define USB_DOEPINT_STPKTRX                                  0x00008000
/********************  Bit definition for USB_DOEPTSIZ     register  ********************/

#define USB_DOEPTSIZ_XFRSIZ                                  0x0007ffff
#define USB_DOEPTSIZ_PKTCNT                                  0x1ff80000
#define USB_DOEPTSIZ_STUPCNT                                 0x60000000
#define USB_DOEPTSIZ_STUPCNT_0                               0x20000000
#define USB_DOEPTSIZ_STUPCNT_1                               0x40000000

/********************  Bit definition for PCGCCTL register  ********************/
#define USB_PCGCCTL_STOPCLK                                  0x00000001
#define USB_PCGCCTL_GATECLK                                  0x00000002
#define USB_PCGCCTL_PHYSUSP                                  0x00000010

/* Legacy define */
/********************  Bit definition for OTG register  ********************/
#define USB_CHNUM                                            0x0000000f
#define USB_CHNUM_0                                          0x00000001
#define USB_CHNUM_1                                          0x00000002
#define USB_CHNUM_2                                          0x00000004
#define USB_CHNUM_3                                          0x00000008
#define USB_BCNT                                             0x00007ff0
#define USB_DPID                                             0x00018000
#define USB_DPID_0                                           0x00008000
#define USB_DPID_1                                           0x00010000
#define USB_PKTSTS                                           0x001e0000
#define USB_PKTSTS_0                                         0x00020000
#define USB_PKTSTS_1                                         0x00040000
#define USB_PKTSTS_2                                         0x00080000
#define USB_PKTSTS_3                                         0x00100000
#define USB_EPNUM                                            0x0000000f
#define USB_EPNUM_0                                          0x00000001
#define USB_EPNUM_1                                          0x00000002
#define USB_EPNUM_2                                          0x00000004
#define USB_EPNUM_3                                          0x00000008
#define USB_FRMNUM                                           0x01e00000
#define USB_FRMNUM_0                                         0x00200000
#define USB_FRMNUM_1                                         0x00400000
#define USB_FRMNUM_2                                         0x00800000
#define USB_FRMNUM_3                                         0x01000000

#endif /* STM32F401_OTG_H */
