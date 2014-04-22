/*
 * utilities.c
 *
 *  Created on: Apr 9, 2014
 *      Author: Max Zhao
 */
#include "./utilities.h"
static inline char digitToHex(int digit) {
	if (digit < 10) {
		return digit + '0';
	} else {
		return (digit - 10) + 'a';
	}
}

void itoa_hex32(uint32_t val, char* buffer) {
	int i;
	buffer[0] = '0';
	buffer[1] = 'x';
	buffer[10] = 0;
	for (i = 2; i < 10; i++) {
		buffer[i] = digitToHex(val >> 28);
		val <<= 4;
	}
}
