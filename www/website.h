/**
 * @file website.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 */

#ifndef WWW_WEBSITE_H
#define WWW_WEBSITE_H

#include "err.h"

/**
 * @brief website serving server instance
 *
 * @return err_t error code
 */
err_t HTTPSrvWebsite_Init(void);

#endif /* WWW_WEBSITE_H */
