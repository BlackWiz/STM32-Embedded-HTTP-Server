/** @file spi.c
 *
 * @brief SPI driver implementation for STM32G071RB.
 *
 * Interrupt-driven SPI1 driver following Serial-JSON-Bridge UART pattern.
 * Application-specific for ENC28J60 Ethernet controller communication.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2026 Hari. All rights reserved.
 */

#include <stdint.h>
#include "spi.h"
#include "types.h"

/* Inline assembly for critical sections */
static inline void __enable_irq(void) {
    __asm volatile ("cpsie i" : : : "memory");
}

static inline void __disable_irq(void) {
    __asm volatile ("cpsid i" : : : "memory");
}

/* ✓ BARR PRINCIPLE #1: Register Definitions (Implementation) */

volatile uint32_t * SPI1 = (uint32_t *) 0x40013000;
volatile uint32_t * SPI1_CR1 = (uint32_t *) 0x40013000;
volatile uint32_t * SPI1_CR2 = (uint32_t *) 0x40013004;
volatile uint32_t * SPI1_SR  = (uint32_t *) 0x40013008;
volatile uint32_t * SPI1_DR  = (uint32_t *) 0x4001300C;

/* ✓ BARR PRINCIPLE #2: State Variables (Implementation) */

/* Global state variables for SPI state machine */
volatile uint8_t const * g_p_spi_tx_buffer = NULL;
volatile uint16_t g_spi_tx_length = 0u;
volatile uint16_t g_spi_tx_index = 0u;
volatile spi_state_t g_spi_state = SPI_STATE_IDLE;
static volatile bool_t g_spi_initialized = FALSE;

/* ✓ BARR PRINCIPLE #3: Hardware Initialization Routine */

/*!
 * @brief Initialize SPI1 peripheral with interrupt-driven operation.
 *
 * Configuration: Master mode, CPOL=0, CPHA=0, 8-bit data, MSB first.
 * For ENC28J60: Max SPI clock = 20 MHz, but we'll use slower for reliability.
 *
 * @param[in] prescaler Baud rate prescaler value.
 *
 * @return 0 on success, -1 on error.
 */
int32_t
spi_init (uint32_t prescaler)
{
    /* RCC must have enabled SPI1 clock already */
    /* GPIO pins (PA5/PA6/PA7) must be configured as AF */
    
    /* Disable SPI first for configuration */
    *SPI1_CR1 &= ~(1u << SPI_CR1_SPE_BIT);
    
    /* Configure CR1: Master mode, CPOL=0, CPHA=0, MSB first, Software NSS */
    *SPI1_CR1 = 0u;  /* Reset all bits */
    *SPI1_CR1 |= (1u << SPI_CR1_MSTR_BIT);  /* Master mode */
    *SPI1_CR1 |= (prescaler << SPI_CR1_BR_BIT);  /* Set baud rate */
    *SPI1_CR1 |= (1u << SPI_CR1_SSM_BIT);  /* Software slave management */
    *SPI1_CR1 |= (1u << SPI_CR1_SSI_BIT);  /* Internal slave select high */
    
    /* Configure CR2: 8-bit data, RXNE threshold for 8-bit */
    *SPI1_CR2 = 0u;
    *SPI1_CR2 |= (0x7u << SPI_CR2_DS_BIT);  /* 8-bit data size (0111b = 8 bits) */
    *SPI1_CR2 |= (1u << SPI_CR2_FRXTH_BIT);  /* RXNE event at 8-bit */
    
    /* Enable SPI */
    *SPI1_CR1 |= (1u << SPI_CR1_SPE_BIT);
    
    g_spi_initialized = TRUE;
    g_spi_state = SPI_STATE_IDLE;
    
    return 0;
}

/* ✓ BARR PRINCIPLE #4: Driver API Functions */

/*!
 * @brief Transmit buffer via SPI1 using interrupt-driven method.
 *
 * @param[in] p_tx Pointer to transmit buffer.
 * @param[in] len Number of bytes to transmit.
 *
 * @return 0 on success, -1 if NULL pointer, -2 if busy.
 */
int32_t
spi_transmit_buffer (uint8_t const * const p_tx, uint16_t len)
{
    if (NULL == p_tx) {
        return -1;
    }
    
    if (0u == len) {
        return -1;
    }
    
    /* Critical section: check and update state atomically */
    __disable_irq();
    
    if (SPI_STATE_IDLE != g_spi_state) {
        __enable_irq();
        return -2;  /* Busy */
    }
    
    g_spi_state = SPI_STATE_TX_BUSY;
    __enable_irq();
    
    /* Setup transmission parameters */
    g_p_spi_tx_buffer = p_tx;
    g_spi_tx_length = len;
    g_spi_tx_index = 0u;
    
    /* Enable TXE interrupt to start transmission */
    *SPI1_CR2 |= (1u << SPI_CR2_TXEIE_BIT);
    
    return 0;
}

/*!
 * @brief Synchronous single-byte transfer (blocking).
 *
 * Used for simple SPI read/write operations.
 *
 * @param[in] data Byte to transmit.
 *
 * @return Received byte.
 */
uint8_t
spi_transmit_receive_byte (uint8_t data)
{
    /* Wait for TX empty */
    while ((*SPI1_SR & (1u << SPI_SR_TXE_BIT)) == 0u) {
        /* Busy wait */
    }
    
    /* Send data */
    *SPI1_DR = (uint32_t)data;
    
    /* Wait for RX not empty */
    while ((*SPI1_SR & (1u << SPI_SR_RXNE_BIT)) == 0u) {
        /* Busy wait */
    }
    
    /* Read and return received data */
    return (uint8_t)(*SPI1_DR);
}

/*!
 * @brief Check if SPI is currently busy.
 *
 * @return TRUE if busy, FALSE if idle.
 */
bool_t
spi_is_busy (void)
{
    return (SPI_STATE_IDLE != g_spi_state) ? TRUE : FALSE;
}

/* ✓ BARR PRINCIPLE #5: Interrupt Service Routine */

/*!
 * @brief Process SPI transmit interrupt.
 *
 * Called from SPI1_IRQHandler when TXE flag is set.
 * Sends next byte or completes transmission.
 *
 * @note Static inline for efficiency (called only from ISR).
 */
static inline void
spi_process_tx (void)
{
    if ((NULL != g_p_spi_tx_buffer) && (g_spi_tx_index < g_spi_tx_length))
    {
        /* Send next byte */
        *SPI1_DR = (uint32_t)g_p_spi_tx_buffer[g_spi_tx_index];
        g_spi_tx_index++;
    }
    else
    {
        /* Transmission complete - disable interrupt */
        *SPI1_CR2 &= ~(1u << SPI_CR2_TXEIE_BIT);
        g_p_spi_tx_buffer = NULL;
        g_spi_state = SPI_STATE_IDLE;
    }
}

/*!
 * @brief SPI1 interrupt service routine.
 *
 * Handles TX and RX interrupts for interrupt-driven communication.
 * Keeps ISR minimal - just calls inline processing functions.
 */
void
SPI1_IRQHandler (void)
{
    /* Handle transmit interrupt - TXE flag set */
    if (((*SPI1_SR & (1u << SPI_SR_TXE_BIT)) != 0u) && 
        ((*SPI1_CR2 & (1u << SPI_CR2_TXEIE_BIT)) != 0u))
    {
        spi_process_tx();
    }
    
    /* Future: Handle receive interrupt if needed */
    /* For now, ENC28J60 uses synchronous byte transfer */
}

/*** end of file ***/
