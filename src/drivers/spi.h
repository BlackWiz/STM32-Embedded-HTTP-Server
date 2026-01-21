/** @file spi.h
 *
 * @brief SPI driver interface for STM32G071RB.
 *
 * Application-specific SPI1 driver for ENC28J60 Ethernet controller.
 * Uses interrupt-driven state machine for efficient byte-by-byte transfer.
 * NOT a generic HAL-style SPI library.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2026 Hari. All rights reserved.
 */

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "types.h"

/* ✓ BARR PRINCIPLE #1: Memory-Mapped Register Pointers */

/* Defining SPI Registers used */
/*
In this project SPI1 is used for ENC28J60 Ethernet controller communication.
Pins: PA5 (SCK), PA6 (MISO), PA7 (MOSI)

SPI1_CR1  --> At an Offset of 0x00
SPI1_CR2  --> At an Offset of 0x04
SPI1_SR   --> At an Offset of 0x08
SPI1_DR   --> At an Offset of 0x0C
*/
extern volatile uint32_t * SPI1;
extern volatile uint32_t * SPI1_CR1;
extern volatile uint32_t * SPI1_CR2;
extern volatile uint32_t * SPI1_SR;
extern volatile uint32_t * SPI1_DR;

/* Register bit position constants */
#define SPI_CR1_CPHA_BIT       0u
#define SPI_CR1_CPOL_BIT       1u
#define SPI_CR1_MSTR_BIT       2u
#define SPI_CR1_BR_BIT         3u   /* Baud rate (3 bits starting at position 3) */
#define SPI_CR1_SPE_BIT        6u
#define SPI_CR1_LSBFIRST_BIT   7u
#define SPI_CR1_SSI_BIT        8u
#define SPI_CR1_SSM_BIT        9u
#define SPI_CR1_RXONLY_BIT     10u
#define SPI_CR1_CRCL_BIT       11u
#define SPI_CR1_CRCNEXT_BIT    12u
#define SPI_CR1_CRCEN_BIT      13u
#define SPI_CR1_BIDIOE_BIT     14u
#define SPI_CR1_BIDIMODE_BIT   15u

#define SPI_CR2_RXDMAEN_BIT    0u
#define SPI_CR2_TXDMAEN_BIT    1u
#define SPI_CR2_SSOE_BIT       2u
#define SPI_CR2_NSSP_BIT       3u
#define SPI_CR2_FRF_BIT        4u
#define SPI_CR2_ERRIE_BIT      5u
#define SPI_CR2_RXNEIE_BIT     6u
#define SPI_CR2_TXEIE_BIT      7u
#define SPI_CR2_DS_BIT         8u   /* Data size (4 bits) */
#define SPI_CR2_FRXTH_BIT      12u
#define SPI_CR2_LDMA_RX_BIT    13u
#define SPI_CR2_LDMA_TX_BIT    14u

#define SPI_SR_RXNE_BIT        0u
#define SPI_SR_TXE_BIT         1u
#define SPI_SR_BSY_BIT         7u

/* SPI baud rate prescaler values */
#define SPI_PRESCALER_2        0x0u
#define SPI_PRESCALER_4        0x1u
#define SPI_PRESCALER_8        0x2u
#define SPI_PRESCALER_16       0x3u
#define SPI_PRESCALER_32       0x4u
#define SPI_PRESCALER_64       0x5u
#define SPI_PRESCALER_128      0x6u
#define SPI_PRESCALER_256      0x7u

/* ✓ BARR PRINCIPLE #2: State Tracking Variables */

/* SPI state machine states */
typedef enum {
    SPI_STATE_IDLE = 0,
    SPI_STATE_TX_BUSY,
    SPI_STATE_RX_BUSY,
    SPI_STATE_ERROR
} spi_state_t;

/* Public state access (for testing) */
extern volatile spi_state_t g_spi_state;

/* ✓ BARR PRINCIPLE #4: Public API */

/*!
 * @brief Initialize SPI1 peripheral with interrupt-driven operation.
 *
 * Configures SPI1 as master, CPOL=0, CPHA=0, 8-bit data for ENC28J60.
 * GPIO pins (PA5/PA6/PA7) must be configured before calling this.
 *
 * @param[in] prescaler Baud rate prescaler value (SPI_PRESCALER_x).
 *
 * @return 0 on success, -1 on error.
 */
int32_t spi_init(uint32_t prescaler);

/*!
 * @brief Transmit buffer via SPI1 using interrupt-driven method.
 *
 * Non-blocking. Function returns immediately, ISR handles byte-by-byte transfer.
 *
 * @param[in] p_tx Pointer to transmit buffer (must remain valid until complete).
 * @param[in] len Number of bytes to transmit.
 *
 * @return 0 on success, -1 if p_tx is NULL, -2 if SPI busy.
 */
int32_t spi_transmit_buffer(uint8_t const * const p_tx, uint16_t len);

/*!
 * @brief Synchronous single-byte transfer (blocking).
 *
 * Transmits one byte and returns received byte. Useful for SPI read/write.
 *
 * @param[in] data Byte to transmit.
 *
 * @return Received byte.
 */
uint8_t spi_transmit_receive_byte(uint8_t data);

/*!
 * @brief Check if SPI is currently busy.
 *
 * @return TRUE if busy, FALSE if idle.
 */
bool_t spi_is_busy(void);

#endif /* SPI_H */

/*** end of file ***/
