/*
 * corespi0_os.c
 *
 *  Created on: Apr 10, 2014
 *      Author: Max Zhao
 *
 */

#include <stdlib.h>
#include <a2fxxxm3.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include "core_spi.h"
#include "corespi_regs.h"
#include "hal.h"
#include "hal_assert.h"
#include "mp3.h"
#include "./corespi0_os.h"

SemaphoreHandle_t xSemCoreSPI0;

static uint8_t* txBuffer;
static uint8_t* rxBuffer;
static uint8_t* bufferEnd;
static bool readIntoBuffer;
static bool sRestartTransmission(void);
static void sTakeRXData(void);

#if defined(__GNUC__)
__attribute__((__interrupt__))
#endif
void Fabric_IRQHandler(void) {
	if (!HAL_get_8bit_reg_field( g_core_spi0.base_addr, INTMASK_TXDONE )) {
		// Wrong interrupt. Did you disable all interrupts except TXDONE?
		exit(20);
	}

	// Clear INT
	bool done = HAL_get_8bit_reg_field(g_core_spi0.base_addr, STATUS_DONE);
	HAL_set_8bit_reg_field( g_core_spi0.base_addr, INTCLR_TXDONE, 1 );
	NVIC_ClearPendingIRQ(Fabric_IRQn); // Clear early, just in case.

	if (!done) {
		// Incomplete transmission. Oops, check if proper last frames are used.
		exit(21);
	}

	sTakeRXData();
	if (!sRestartTransmission()) {
		// Done, check if received everything
		if (txBuffer != rxBuffer || txBuffer != bufferEnd) {
			// Incomplete read!!!
			exit(22);
		}

		BaseType_t wokeHighPriority = pdTRUE;
		// Restart function
		xSemaphoreGiveFromISR(xSemCoreSPI0, &wokeHighPriority);

		txBuffer = rxBuffer = bufferEnd = NULL; // Clear buffer pointer to allow fault detection.

		// Allow higher priority tasks to wakeup immediately
		portYIELD_FROM_ISR(wokeHighPriority);
	}
}

static void sTakeRXData(void) {
	if (rxBuffer == NULL) { // Error
		exit(23);
	}

	if (HAL_get_8bit_reg_field(g_core_spi0.base_addr, STATUS_RXOVFLOW)) {
		// Overflowed
		exit(24);
	}
	// Take data until empty
	while (!HAL_get_8bit_reg_field(g_core_spi0.base_addr, STATUS_RXEMPTY)) {
		if (rxBuffer >= bufferEnd) {
			// Oops, somehow the read overflowed the buffer size.
			exit(25);
		}

		if (readIntoBuffer) {
			*(rxBuffer++) = HAL_get_32bit_reg( g_core_spi0.base_addr, RXDATA );
		} else {
			uint32_t dummy = HAL_get_32bit_reg( g_core_spi0.base_addr, RXDATA );
			dummy = dummy;
			rxBuffer ++;
		}
	}
}

static bool sRestartTransmission(void) {
	if (txBuffer >= bufferEnd) {
		return false;
	}

	HAL_set_8bit_reg_field( g_core_spi0.base_addr, CTRL1_ENABLE, 0 );
	// Fill tx buffer if there is more data and that there is more room in the fifo
	while (txBuffer < bufferEnd && !HAL_get_8bit_reg_field(g_core_spi0.base_addr, STATUS_TXFULL )) {
		if (txBuffer == bufferEnd - 1 || (txBuffer - rxBuffer) == g_core_spi0.fifo_depth - 1) {
			// Last byte in buffer or fifo, use TXLAST
			HAL_set_32bit_reg( g_core_spi0.base_addr, TXLAST, *(txBuffer++));
		} else {
			// Otherwise, just keep the SS pin asserted.
			HAL_set_32bit_reg( g_core_spi0.base_addr, TXDATA, *(txBuffer++));
		}
	}
	HAL_set_8bit_reg_field( g_core_spi0.base_addr, CTRL1_ENABLE, 1 );

	return true;
}

void corespi0_init() {
	xSemCoreSPI0 = xSemaphoreCreateBinary();
	NVIC_EnableIRQ(Fabric_IRQn);
}

// Make sure this function is used in ONLY one thread at a time.
void corespi0_transmit_internal(void* buffer, int length, bool read) {
	if (rxBuffer != NULL || txBuffer != NULL) {
		// Inconsistent state. .bss ensures zero initialization
		exit(26);
	}

	rxBuffer = txBuffer = bufferEnd = buffer;
	bufferEnd += length;
	readIntoBuffer = read;

	// Clear RX TX fifos
	HAL_set_8bit_reg(g_core_spi0.base_addr, CMD, CMD_TXFIFORST_MASK | CMD_RXFIFORST_MASK);

	// Clear Semaphore
	xSemaphoreTake(xSemCoreSPI0, 0);

	// Enable INT (Not IRQ)
	HAL_set_8bit_reg_field(g_core_spi0.base_addr, CTRL1_INTTXDONE, 1);

	// Start transmission
	if (!sRestartTransmission()) {
		// You shouldn't be sending empty, this will deadlock. Probably a bug.
		exit(27);
	}

	// Wait for transimission end
	if (xSemaphoreTake(xSemCoreSPI0, 1000 * portTICK_PERIOD_MS) != pdTRUE) {
		// failed to obtained semaphore within 1 second. Nope. Can't happen. May be interrupt wasn't enabled?
		exit(28);
	}
}

uint8_t corespi0_transmit_byte(uint8_t data) {
	corespi0_transmit(&data, 1);
	return data;
}
