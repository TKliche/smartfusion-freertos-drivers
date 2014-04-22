/*
 * uart0_os.h
 *
 *  Created on: Apr 9, 2014
 *      Author: Max Zhao
 * Interrupt based UART0 driver
 *
 * USES PDMA_CHANNEL_0 for UART Write
 */

#ifndef UART0_OS_H_
#define UART0_OS_H_
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <semphr.h>

#define UART0_ENVM_BUFFER_SIZE 50

extern SemaphoreHandle_t xSemUART0TXComplete;
extern SemaphoreHandle_t xMutexUART0;
void uart0_init();

/**
 * @param envm  is the buffer in envm. If so, it must be less than UART0_ENVM_BUFFER_SIZE in length,
 *              and will be copied into the sram.
 */
void uart0_write_internal(const void* buffer, int length, bool envm);
#define uart0_write(buffer, length) uart0_write_internal(buffer, length, false)
void uart0_write_string(const char* str);
void uart0_write_hex32(uint32_t num);

#endif /* UART0_OS_H_ */
