/**
 * @file api.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef WWW_API_H
#define WWW_API_H

#include "err.h"

/**
 * @brief create api serving server instance
 *
 * @return err_t error code
 */
err_t HTTPSrvApi_Init(void);


#endif /* WWW_API_H */
