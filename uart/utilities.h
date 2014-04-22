/*
 * utilities.h
 *
 *  Created on: Apr 9, 2014
 *      Author: Max Zhao
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_
#include <stdint.h>

/**
 * Fills buffer with exactly 11 bytes in the form of 0x12345678 (with null byte)
 */
void itoa_hex32(uint32_t val, char* buffer);

#endif /* UTILITIES_H_ */
