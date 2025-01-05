/**
 * @file jenkins.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-03
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef UTIL_JENKINS_H
#define UTIL_JENKINS_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Implementation of Bob Jenkins' One At A Time hashing function
 *
 * @param iv initial value
 * @param key data pointer
 * @param size size of the data
 *
 * @return uint32_t hash value
 */
uint32_t Jenkins_OAAT(uint32_t iv, const uint8_t* key, size_t size);

#endif /* UTIL_JENKINS_H */
