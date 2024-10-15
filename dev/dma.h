/**
 * @file dma.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-08-26
 * 
 * @brief Dma driver
 */
#ifndef DEV_DMA_H
#define DEV_DMA_H

#include <stdint.h>
#include <stddef.h>

#include "assert.h"
#include "compiler.h"
#include "err.h"
#include "stm32f401/dma.h"
#include "util/bit.h"
#include "util/msblsb.h"

/** dma block number */
typedef enum dma_num {
    DMA_1 = 0,
    DMA_2 = 1,
} dma_num_t;

/** dma stream numbers */
typedef enum dma_stream_num {
    DMA_STREAM_0 = 0,
    DMA_STREAM_1,
    DMA_STREAM_2,
    DMA_STREAM_3,
    DMA_STREAM_4,
    DMA_STREAM_5,
    DMA_STREAM_6,
    DMA_STREAM_7,
} dma_stream_num_t;

/** dma channel numbers */
typedef enum dma_channel_num {
    DMA_CHANNEL_0 = 0,
    DMA_CHANNEL_1,
    DMA_CHANNEL_2,
    DMA_CHANNEL_3,
    DMA_CHANNEL_4,
    DMA_CHANNEL_5,
    DMA_CHANNEL_6,
    DMA_CHANNEL_7,
} dma_channel_num_t;

/** dma status flags */
typedef enum dma_status_flags {
    /** fifo error */
    DMA_STATUS_FLAG_FIFO_ERR = DMA_LISR_FEIF0,
    /** direct transfer error */
    DMA_STATUS_FLAG_DIR_ERR = DMA_LISR_DMEIF0,
    /** general tranfser error  */
    DMA_STATUS_FLAG_TFER_ERR = DMA_LISR_TEIF0,
    /** half transfer complete */
    DMA_STATUS_FLAG_HALF_TFER = DMA_LISR_HTIF0,
    /** full transfer complete */
    DMA_STATUS_FLAG_FULL_TFER = DMA_LISR_TCIF0,

    /** all status flags */
    DMA_STATUS_FLAG_ALL = DMA_STATUS_FLAG_FIFO_ERR | DMA_STATUS_FLAG_DIR_ERR |
        DMA_STATUS_FLAG_TFER_ERR | DMA_STATUS_FLAG_HALF_TFER |
        DMA_STATUS_FLAG_FULL_TFER,
} dma_status_flags_t;

/** transfer option flags */
typedef enum dma_tfer_flags {
    /** peripheral-to-memory transfer direction */
    DMA_TFER_FLAG_DIR_P2M = 0,
    /** memory-to-peripheral transfer direction */
    DMA_TFER_FLAG_DIR_M2P = DMA_CR_DIR_0,
    /** memory-to-memory transfer direction */
    DMA_TFER_FLAG_DIR_M2M = DMA_CR_DIR_1,
    /** circular buffer mode */
    DMA_TFER_FLAG_CIRC = DMA_CR_CIRC,
    /** increment memory address after each transfer */
    DMA_TFER_FLAG_MINC = DMA_CR_MINC,
    /** memory transfer width: 1 byte */
    DMA_TFER_FLAG_MSIZE_1B = 0,
    /** memory transfer width: 2 bytes */
    DMA_TFER_FLAG_MSIZE_2B = DMA_CR_MSIZE_0,
    /** memory transfer width: 4 bytes */
    DMA_TFER_FLAG_MSIZE_4B = DMA_CR_MSIZE_1,
    /** increment peripheral address after each transfer */
    DMA_TFER_FLAG_PINC = DMA_CR_PINC,
    /** peripheral transfer width: 1 byte */
    DMA_TFER_FLAG_PSIZE_1B = 0,
    /** peripheral transfer width: 2 bytes */
    DMA_TFER_FLAG_PSIZE_2B = DMA_CR_PSIZE_0,
    /** peripheral transfer width: 4 bytes */
    DMA_TFER_FLAG_PSIZE_4B = DMA_CR_PSIZE_1,

    /** interrupt flags: full transfer complete */
    DMA_TFER_FLAG_INT_FULL = DMA_CR_TCIE,
    /** half transfer complete */
    DMA_TFER_FLAG_INT_HALF = DMA_CR_HTIE,

    /** priority levels: low */
    DMA_TFER_FLAG_PRI_LOW = 0,
    /** priority levels: medium */
    DMA_TFER_FLAG_PRI_MED = DMA_CR_PL_0,
    /** priority levels: high */
    DMA_TFER_FLAG_PRI_HIGH = DMA_CR_PL_1,
    /** priority levels: very high */
    DMA_TFER_FLAG_PRI_VERY_HIGH = DMA_CR_PL,

    /** all flags */
    DMA_TFER_FLAG_ALL = DMA_TFER_FLAG_DIR_P2M | DMA_TFER_FLAG_DIR_M2P |
        DMA_TFER_FLAG_DIR_M2M | DMA_TFER_FLAG_CIRC | DMA_TFER_FLAG_MINC |
        DMA_TFER_FLAG_MSIZE_1B | DMA_TFER_FLAG_MSIZE_2B | DMA_TFER_FLAG_MSIZE_4B |
        DMA_TFER_FLAG_PINC | DMA_TFER_FLAG_PSIZE_1B | DMA_TFER_FLAG_PSIZE_2B |
        DMA_TFER_FLAG_PSIZE_4B | DMA_TFER_FLAG_INT_FULL |
        DMA_TFER_FLAG_INT_HALF  | DMA_TFER_FLAG_PRI_LOW | DMA_TFER_FLAG_PRI_MED |
        DMA_TFER_FLAG_PRI_HIGH | DMA_TFER_FLAG_PRI_VERY_HIGH,
} dma_tfer_flags_t;

/**
 * @brief initialize dmas
 *
 * @return err_t error code
 */
err_t DMA_Init(void);



/**
 * @brief Returns the dma peripheral address
 *
 * @param dma_num
 * @return dma_t * dma peripheral address
 */
static inline dma_t * ALWAYS_INLINE DMA_GetDMA(dma_num_t dma_num)
{
    /* switch on dma number */
    switch (dma_num){
    /* valid numbers */
    case DMA_1: return DMA1;
    case DMA_2: return DMA2;
    /* invalid ones */
    default: assert(0, "invalid dma_num");
    }
}

/**
 * @brief Returns the peripheral number based on the address provided
 *
 * @param dma dma address
 *
 * @return dma_num_t dma number
 */
static inline dma_num_t ALWAYS_INLINE DMA_GetDMANum(dma_t *dma)
{
    /* switch on dma address */
    switch ((uintptr_t)dma) {
    /* valid dma peripherals */
    case DMA1_BASE: return DMA_1;
    case DMA2_BASE: return DMA_2;
    /* invalid address */
    default: assert(0, "invalid dma address"); return 0;
    }
}

/**
 * @brief Get stream with given number
 *
 * @param dma dma peripheral
 * @param stream_num stream number
 *
 * @return dma_stream_t * pointer to stream register bank
 */
static inline dma_stream_t * ALWAYS_INLINE DMA_GetStream(dma_t *dma,
    dma_stream_num_t stream_num)
{
    /* return the stream block address */
    return (dma_stream_t *)((uintptr_t)dma + 0x10 + 0x18 * stream_num);
}

/**
 * @brief Returns the dma block that the stream belongs to
 *
 * @param stream stream pointer
 *
 * @return dma_t *dma block
 */
static inline dma_t * ALWAYS_INLINE DMA_GetStreamDMA(dma_stream_t *stream)
{
    /* convert to a number for the ease of comparison */
    uintptr_t addr = (uintptr_t)stream;
    /* look by the address range */
    if (addr >= DMA1S0_BASE && addr <= DMA1S7_BASE) return DMA1;
    if (addr >= DMA2S0_BASE && addr <= DMA2S7_BASE) return DMA2;
    /* unknown stream address*/
    assert(0, "invelid stream address");
}

/* return the stream number from the stream address */
static inline dma_num_t ALWAYS_INLINE DMA_GetStreamNum(dma_stream_t *stream)
{
    /* obtain dma peripheral address */
    dma_t *dma = DMA_GetStreamDMA(stream);
    /* get the difference in the addresses */
    uintptr_t diff = (uintptr_t)stream - (uintptr_t)dma;

    /* covnert the offset to stream number */
    return (diff - 0x10) / 0x18;
}

/**
 * @brief Returns status flags for any given dma stream
 *
 * @param stream stream pointer
 *
 * @return dma_status_flags_t flags
 */
static inline dma_status_flags_t ALWAYS_INLINE DMA_GetStatus (
    dma_stream_t *stream)
{
    /* get the dma block */
    dma_t *dma = DMA_GetStreamDMA(stream);
    /* get the stream number */
    dma_stream_num_t stream_num = DMA_GetStreamNum(stream);
    /* get the value of the status register */
    uint32_t isr = stream_num >= DMA_STREAM_4 ? dma->HISR : dma->LISR;

    /* do the shifting according to the stream number */
    switch (stream_num) {
    case DMA_STREAM_0 : case DMA_STREAM_4 : isr >>= LSB(DMA_LISR_FEIF0); break;
    case DMA_STREAM_1 : case DMA_STREAM_5 : isr >>= LSB(DMA_LISR_FEIF1); break;
    case DMA_STREAM_2 : case DMA_STREAM_6 : isr >>= LSB(DMA_LISR_FEIF2); break;
    case DMA_STREAM_3 : case DMA_STREAM_7 : isr >>= LSB(DMA_LISR_FEIF3); break;
    }

    /* and with all flags mask */
    return (dma_status_flags_t)(isr &= DMA_STATUS_FLAG_ALL);
}

/**
 * @brief Clear DMA's status flags
 *
 * @param dma_stream dma stream
 * @param flags flags to be cleared
 */
static inline void ALWAYS_INLINE DMA_ClearStatus(dma_stream_t *stream,
    dma_status_flags_t flags)
{
    /* get the dma block */
    dma_t *dma = DMA_GetStreamDMA(stream);
    /* get the stream number */
    dma_stream_num_t stream_num = DMA_GetStreamNum(stream);
    /* get the destination register address */
    reg32_t *ifcr = stream_num >= DMA_STREAM_4 ? &dma->HIFCR : &dma->LIFCR;
    /* prepare the value to be written to the interrupt clear register */
    uint32_t bits = flags;

    /* do the shifts */
    switch (stream_num) {
    case DMA_STREAM_0 : case DMA_STREAM_4 : bits <<= LSB(DMA_LISR_FEIF0); break;
    case DMA_STREAM_1 : case DMA_STREAM_5 : bits <<= LSB(DMA_LISR_FEIF1); break;
    case DMA_STREAM_2 : case DMA_STREAM_6 : bits <<= LSB(DMA_LISR_FEIF2); break;
    case DMA_STREAM_3 : case DMA_STREAM_7 : bits <<= LSB(DMA_LISR_FEIF3); break;
    }

    /* write the value */
    *ifcr = bits;
}

/**
 * @brief Enable/disable stream
 *
 * @param stream stream to be configured
 * @param enable 1 - enable, 0 - disable
 */
static inline void ALWAYS_INLINE DMA_CfgEnable(dma_stream_t *stream,
    int enable)
{
    /* set or clear the enable bit */
    enable ? (stream->CR |= DMA_CR_EN) : (stream->CR &= ~DMA_CR_EN);
}

/**
 * @brief Configure transfer parameters
 *
 * @param stream stream to be configured
 * @param flags transfer option flags
 */
static inline void ALWAYS_INLINE DMA_CfgTransfer(dma_stream_t *stream,
    dma_tfer_flags_t flags)
{
    /* mask of flags and channel selection */
    const uint32_t mask = (uint32_t)DMA_TFER_FLAG_ALL;
    /* write the transfer flags */
    stream->CR = (stream->CR & ~(mask)) | (uint32_t)flags;
}

/**
 * @brief Route given peripheral (channel) to stream
 *
 * @param stream stream
 * @param channel_num channel to be routed
 */
static inline void DMA_CfgChannel(dma_stream_t *stream, int channel_num)
{
    /* update the channel number */
    stream->CR = (stream->CR & ~(DMA_CR_CHSEL)) | (channel_num << LSB(DMA_CR_CHSEL));
}

/**
 * @brief Configure stream memory and peripheral address
 *
 * @param stream stream register bank pointer
 * @param addr memory address
 */
static inline void ALWAYS_INLINE DMA_CfgMemAddr(dma_stream_t *stream,
    void *addr)
{
    /* setup addresses */
    stream->M0AR = (uint32_t)addr;
}

/**
 * @brief Configure stream memory and peripheral address
 *
 * @param stream stream register bank pointer
 * @param addr peripheral address
 */
static inline void ALWAYS_INLINE DMA_CfgPeriphAddr(dma_stream_t *stream,
    void *addr)
{
    /* setup addresses */
    stream->PAR = (uint32_t)addr;
}

/**
 * @brief Set Transfer size
 *
 * @param stream stream register bank pointer
 * @param tfer_num number of transfers to be performed
 */
static inline void ALWAYS_INLINE DMA_CfgSize(dma_stream_t *stream,
    size_t tfer_num)
{
    /* set the transfer size */
    stream->NDTR = tfer_num;
}

/**
 * @brief Get Transfer size
 *
 * @param stream stream register bank pointer
 *
 * @return size_t number of transfers left
 */
static inline  ALWAYS_INLINE size_t DMA_GetSize(dma_stream_t *stream)
{
    /* set the transfer size */
    return stream->NDTR;
}


#endif /* DEV_DMA_H */
