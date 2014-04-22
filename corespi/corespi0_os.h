/*
 * corespi1_os.h
 *
 *  Created on: Apr 10, 2014
 *      Author: Max Zhao
 *
 * Interrupt based CoreSPI0 handling. This module does not handle CoreSPI initialization, or slaveselect handling.
 * Please handle those properly outside.
 *
 * To improve performance, this module does not have mutex protection. In any case, CoreSPI0 should only be used
 * for the MP3 controller.
 */

#ifndef CORESPI1_OS_H_
#define CORESPI1_OS_H_

#include <FreeRTOS.h>
#include <semphr.h>

extern SemaphoreHandle_t xSemCoreSPI0;
void corespi0_init();

void corespi0_transmit_internal(void* buffer, int length, bool read);
#define corespi0_transmit(buffer, length) corespi0_transmit_internal(buffer, length, true);
#define corespi0_send(buffer, length) corespi0_transmit_internal((void*) buffer, length, false);
uint8_t corespi0_transmit_byte(uint8_t data);
#define corespi0_send_byte(data) corespi0_transmit_byte(data) // No performance benefit here, so yeah.

#endif /* CORESPI1_OS_H_ */
