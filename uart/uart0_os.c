/*
 * uart0_os.c
 *
 *  Created on: Apr 9, 2014
 *      Author: Max Zhao
 */
#include <mss_uart.h>
#include <mss_pdma.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <stdlib.h>
#include <core_cm3.h>
#include <string.h>
#include "os.h"
#include "./uart0_os.h"
#include "utilities.h"

SemaphoreHandle_t xSemUART0TXComplete;
SemaphoreHandle_t xMutexUART0;

static void sUART0DMACompleteHandler() {
	PDMA_clear_irq(PDMA_CHANNEL_0);
	if (osSchedulerStarted) {
		xSemaphoreGiveFromISR(xSemUART0TXComplete, NULL);
	}
}

void uart0_init() {
	PDMA_configure
	(
		PDMA_CHANNEL_0,
		PDMA_TO_UART_0,
		PDMA_LOW_PRIORITY | PDMA_BYTE_TRANSFER | PDMA_INC_SRC_ONE_BYTE,
		PDMA_DEFAULT_WRITE_ADJ
	);

	xSemUART0TXComplete = xSemaphoreCreateBinary();
	xMutexUART0 = xSemaphoreCreateMutex();

	// FreeRTOS bug: sets Enters critical but doesn't exit when there is no task (IDIOTS!)
	PDMA_set_irq_handler(PDMA_CHANNEL_0, sUART0DMACompleteHandler);
	PDMA_enable_irq(PDMA_CHANNEL_0);
}

static uint8_t envm_translate_buffer[UART0_ENVM_BUFFER_SIZE];

void uart0_write_internal(const void* buffer, int length, bool envm) {
	xSemaphoreTake(xMutexUART0, portMAX_DELAY);

	if (envm) {
		bool overflowed = false;
		if (length > UART0_ENVM_BUFFER_SIZE) {
			length = UART0_ENVM_BUFFER_SIZE;
			overflowed = true;
		}
		memcpy(envm_translate_buffer, buffer, length);

		if (overflowed) {
			memcpy(envm_translate_buffer + UART0_ENVM_BUFFER_SIZE - 4, " OVF", 4);
		}

		buffer = envm_translate_buffer;
	}

	PDMA_start(PDMA_CHANNEL_0, (uintptr_t) buffer, PDMA_UART0_TX_REGISTER, length);

	if (osSchedulerStarted) {
		// wait semaphore
		if (xSemaphoreTake(xSemUART0TXComplete, 1000 * portTICK_PERIOD_MS) != pdTRUE) {
			// failed to obtained semaphore within 1 second. Nope. Can't happen. May be interrupt wasn't enabled?
			exit(40);
		}
	} else {
		//polling wait
		while (PDMA_status(PDMA_CHANNEL_0) == 0);
		PDMA_clear_irq(PDMA_CHANNEL_0);
	}
	xSemaphoreGive(xMutexUART0);
}

int _write_r( void * reent, int file, char * ptr, int len )
{
	if (file == 1 || file == 2) { // stdout and stderr
		uart0_write(ptr, len);
		//MSS_UART_polled_tx( &g_mss_uart0, (uint8_t *)ptr, len );
		return len;
	} else {
		return 0;
	}
}

void uart0_write_string(const char* str) {
	uart0_write_internal(str, strlen(str), true);
}

void uart0_write_hex32(uint32_t num) {
	char buffer[11];
	itoa_hex32(num, buffer);
	uart0_write(buffer, 10);
}
