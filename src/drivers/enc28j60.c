/** @file enc28j60.c
 *
 * @brief ENC28J60 Ethernet controller driver implementation (STUB VERSION).
 *
 * This is a stubbed implementation for when hardware is not yet available.
 * Functions return success/dummy values but don't communicate with actual chip.
 *
 * TODO: Replace stubs with real SPI communication when ENC28J60 hardware arrives.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2026 Hari. All rights reserved.
 */

#include <stdint.h>
#include "enc28j60.h"
#include "spi.h"
#include "types.h"

/* ✓ BARR PRINCIPLE #2: State Variables (Implementation) */

/* Global state variable */
volatile enc28j60_state_t g_enc28j60_state = ENC28J60_STATE_UNINITIALIZED;
static uint8_t g_mac_address[6] = {0};

/* ✓ BARR PRINCIPLE #3: Hardware Initialization Routine */

/*!
 * @brief Initialize ENC28J60 Ethernet controller.
 *
 * @param[in] p_mac_addr Pointer to 6-byte MAC address.
 *
 * @return 0 on success, -1 on error.
 *
 * @note STUB IMPLEMENTATION - No hardware communication.
 */
int32_t
enc28j60_init (uint8_t const * const p_mac_addr)
{
    if (NULL == p_mac_addr) {
        return -1;
    }
    
    /* Save MAC address */
    for (uint8_t i = 0u; i < 6u; i++) {
        g_mac_address[i] = p_mac_addr[i];
    }
    
    /* TODO: When hardware available:
     * 1. Send soft reset command (SRC)
     * 2. Wait for CLKRDY bit in ESTAT
     * 3. Configure receive buffer (ERXST, ERXND)
     * 4. Configure MAC address registers
     * 5. Enable packet reception (ECON1.RXEN)
     */
    
    g_enc28j60_state = ENC28J60_STATE_READY;
    
    return 0;  /* STUB: Success */
}

/* ✓ BARR PRINCIPLE #4: Driver API Functions */

/*!
 * @brief Read ENC28J60 control register (STUB).
 *
 * @param[in] reg Register address.
 *
 * @return Register value.
 *
 * @note STUB: Returns 0x00.
 */
uint8_t
enc28j60_read_register (uint8_t reg)
{
    /* TODO: When hardware available:
     * 1. Send RCR command with register address
     * 2. Read response byte via SPI
     */
    
    (void)reg;  /* Unused parameter */
    return 0x00u;  /* STUB: Dummy value */
}

/*!
 * @brief Write ENC28J60 control register (STUB).
 *
 * @param[in] reg Register address.
 * @param[in] value Value to write.
 *
 * @note STUB: Does nothing.
 */
void
enc28j60_write_register (uint8_t reg, uint8_t value)
{
    /* TODO: When hardware available:
     * 1. Send WCR command with register address
     * 2. Send value byte via SPI
     */
    
    (void)reg;    /* Unused */
    (void)value;  /* Unused */
}

/*!
 * @brief Send Ethernet packet (STUB).
 *
 * @param[in] p_data Pointer to packet data.
 * @param[in] len Packet length.
 *
 * @return 0 on success, -1 on error.
 *
 * @note STUB: Returns success but doesn't transmit.
 */
int32_t
enc28j60_send_packet (uint8_t const * const p_data, uint16_t len)
{
    if (NULL == p_data) {
        return -1;
    }
    
    if (0u == len) {
        return -1;
    }
    
    /* TODO: When hardware available:
     * 1. Set write pointer (EWRPT)
     * 2. Write control byte + packet data via WBM
     * 3. Set transmit start/end pointers
     * 4. Set ECON1.TXRTS to start transmission
     * 5. Wait for completion interrupt
     */
    
    return 0;  /* STUB: Success */
}

/*!
 * @brief Receive Ethernet packet (STUB).
 *
 * @param[out] p_buffer Buffer for received data.
 * @param[in] max_len Maximum buffer size.
 *
 * @return Number of bytes received (0 = no packet).
 *
 * @note STUB: Always returns 0.
 */
uint16_t
enc28j60_receive_packet (uint8_t * const p_buffer, uint16_t max_len)
{
    if (NULL == p_buffer) {
        return 0u;
    }
    
    /* TODO: When hardware available:
     * 1. Check packet count in EPKTCNT
     * 2. Read packet from buffer via RBM
     * 3. Update receive read pointer
     * 4. Decrement packet count
     */
    
    (void)max_len;  /* Unused */
    return 0u;  /* STUB: No packets */
}

/*!
 * @brief Check link status (STUB).
 *
 * @return TRUE if link up, FALSE otherwise.
 *
 * @note STUB: Always returns FALSE.
 */
bool_t
enc28j60_is_link_up (void)
{
    /* TODO: When hardware available:
     * 1. Read PHY status register (PHSTAT2)
     * 2. Check LSTAT bit
     */
    
    return FALSE;  /* STUB: Link down */
}

/* ✓ BARR PRINCIPLE #5: ISRs */
/* ENC28J60 uses INT pin, but we'll poll for MVP-1 */
/* ISR will be added in MVP-2 if needed */

/*** end of file ***/
