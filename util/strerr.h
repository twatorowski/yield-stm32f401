/**
 * @file strerr.h
 * 
 * @date 2019-09-09
 * @author twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief Translate error code to human readable form
 */

#ifndef UTIL_STRERR_H_
#define UTIL_STRERR_H_

/**
 * @brief return the error string for given error code (or empty string 
 * if not found)
 * 
 * @param ec error code (@ref ERR_ERROR_CODES)
 * 
 * @return const char* pointer to an ansi string with error explanation
 */
const char * strerr(err_t ec);


#endif /* UTIL_STRERR_H_ */
