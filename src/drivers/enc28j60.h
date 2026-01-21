/** @file enc28j60.h
 *
 * @brief ENC28J60 Ethernet controller driver interface.
 *
 * Application-specific driver for ENC28J60 SPI Ethernet controller.
 * Communicates via SPI1 at up to 20 MHz (ENC28J60 max).
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2026 Hari. All rights reserved.
 */

#ifndef ENC28J60_H
#define ENC28J60_H

#include <stdint.h>
#include "types.h"

/* ✓ BARR PRINCIPLE #1: ENC28J60 Register Definitions */

/* ENC28J60 SPI Commands (Opcode in upper 3 bits) */
#define ENC28J60_CMD_RCR       0x00u  /* Read Control Register */
#define ENC28J60_CMD_RBM       0x3Au  /* Read Buffer Memory */
#define ENC28J60_CMD_WCR       0x40u  /* Write Control Register */
#define ENC28J60_CMD_WBM       0x7Au  /* Write Buffer Memory */
#define ENC28J60_CMD_BFS       0x80u  /* Bit Field Set */
#define ENC28J60_CMD_BFC       0xA0u  /* Bit Field Clear */
#define ENC28J60_CMD_SRC       0xFFu  /* System Reset Command (soft reset) */

/* Common ENC28J60 Registers (Bank 0) */
#define ENC28J60_REG_ERDPTL    0x00u
#define ENC28J60_REG_ERDPTH    0x01u
#define ENC28J60_REG_EWRPTL    0x02u
#define ENC28J60_REG_EWRPTH    0x03u

/* Common ENC28J60 Registers (All banks) */
#define ENC28J60_REG_EIE       0x1Bu  /* Ethernet Interrupt Enable */
#define ENC28J60_REG_EIR       0x1Cu  /* Ethernet Interrupt Request */
#define ENC28J60_REG_ESTAT     0x1Du  /* Ethernet Status */
#define ENC28J60_REG_ECON2     0x1Eu  /* Ethernet Control 2 */
#define ENC28J60_REG_ECON1     0x1Fu  /* Ethernet Control 1 */

/* ECON1 Register bits */
#define ENC28J60_ECON1_BSEL0   0x01u  /* Bank Select bit 0 */
#define ENC28J60_ECON1_BSEL1   0x02u  /* Bank Select bit 1 */
#define ENC28J60_ECON1_RXEN    0x04u  /* Receive Enable */
#define ENC28J60_ECON1_TXRTS   0x08u  /* Transmit Request to Send */

/* ✓ BARR PRINCIPLE #2: State Tracking Variables */

/* ENC28J60 state */
typedef enum {
    ENC28J60_STATE_UNINITIALIZED = 0,
    ENC28J60_STATE_READY,
    ENC28J60_STATE_TX_BUSY,
    ENC28J60_STATE_ERROR
} enc28j60_state_t;

extern volatile enc28j60_state_t g_enc28j60_state;

/* ✓ BARR PRINCIPLE #4: Public API */

/*!
 * @brief Initialize ENC28J60 Ethernet controller.
 *
 * Performs soft reset, configures MAC, PHY, and buffer pointers.
 * SPI must be initialized before calling this.
 *
 * @param[in] p_mac_addr Pointer to 6-byte MAC address.
 *
 * @return 0 on success, -1 on error.
 *
 * @note STUB: Returns success but doesn't communicate with hardware yet.
 */
int32_t enc28j60_init(uint8_t const * const p_mac_addr);

/*!
 * @brief Read ENC28J60 control register.
 *
 * @param[in] reg Register address.
 *
 * @return Register value (8-bit).
 *
 * @note STUB: Returns dummy value 0x00.
 */
uint8_t enc28j60_read_register(uint8_t reg);

/*!
 * @brief Write ENC28J60 control register.
 *
 * @param[in] reg Register address.
 * @param[in] value Value to write.
 *
 * @note STUB: Does nothing (hardware not connected).
 */
void enc28j60_write_register(uint8_t reg, uint8_t value);

/*!
 * @brief Send Ethernet packet.
 *
 * @param[in] p_data Pointer to packet data.
 * @param[in] len Packet length in bytes.
 *
 * @return 0 on success, -1 on error.
 *
 * @note STUB: Returns success but doesn't transmit.
 */
int32_t enc28j60_send_packet(uint8_t const * const p_data, uint16_t len);

/*!
 * @brief Receive Ethernet packet.
 *
 * @param[out] p_buffer Buffer to store received packet.
 * @param[in] max_len Maximum buffer size.
 *
 * @return Number of bytes received, 0 if no packet available.
 *
 * @note STUB: Always returns 0 (no packets).
 */
uint16_t enc28j60_receive_packet(uint8_t * const p_buffer, uint16_t max_len);

/*!
 * @brief Check if link is up.
 *
 * @return TRUE if link up, FALSE otherwise.
 *
 * @note STUB: Always returns FALSE.
 */
bool_t enc28j60_is_link_up(void);

#endif /* ENC28J60_H */

/*** end of file ***/
