/**
 * @file endian.h
 * 
 * @date 2020-05-14
 * twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief Endiannes manipulation
 */
#ifndef UTIL_ENDIAN
#define UTIL_ENDIAN

/** @brief convert 16-bit host number to big endian */
#define HTOBE16(x)                          \
    (((uint16_t)(x) & 0xff) << 8 | ((uint16_t)(x) & 0xff00) >> 8)
/** @brief convert 16-bit host number to host format */
#define BETOH16(x)                          HTOBE16(x)  

/** @brief convert 32-bit host number to big endian */
#define HTOBE32(x)                          \
    ( ((uint32_t)(x) & 0x000000ff) << 24 |  \
      ((uint32_t)(x) & 0x0000ff00) <<  8 |  \
      ((uint32_t)(x) & 0x00ff0000) >>  8 |  \
      ((uint32_t)(x) & 0xff000000) >> 24 )
/** @brief convert 32-bit host number to host format */
#define BETOH32(x)                          HTOBE32(x)


/** @brief convert 16-bit host number to big endian */
#define HTOLE16(x)                          \
    ( (x) )
/** @brief convert 16-bit host number to host format */
#define LETOH16(x)                          HTOLE16(x)

/** @brief convert 32-bit host number to big endian */
#define HTOLE32(x)                          \
    ( (x) )
/** @brief convert 32-bit host number to host format */
#define LETOH32(x)                          HTOLE32(x)

#endif /* UTIL_ENDIAN */
