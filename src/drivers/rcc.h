/** @file rcc.h
 *
 * @brief RCC (Reset and Clock Control) driver interface for STM32G071RB.
 *
 * Configures system clock to 64 MHz using HSI oscillator and PLL.
 * Provides clock enable functions for peripheral drivers.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2026 Hari. All rights reserved.
 */

#ifndef RCC_H
#define RCC_H

#include <stdint.h>
#include "types.h"

/* ✓ BARR PRINCIPLE #1: Memory-Mapped Register Pointers */

/* Defining RCC Registers used */
/*
RCC_CR      --> At an Offset of 0x00 (Clock control register)
RCC_CFGR    --> At an Offset of 0x08 (Clock configuration register)
RCC_PLLCFGR --> At an Offset of 0x0C (PLL configuration register)
RCC_IOPENR  --> At an Offset of 0x34 (GPIO clock enable)
RCC_APBENR1 --> At an Offset of 0x3C (APB peripheral clock enable 1)
RCC_APBENR2 --> At an Offset of 0x40 (APB peripheral clock enable 2)
*/
extern volatile uint32_t * RCC;
extern volatile uint32_t * RCC_CR;
extern volatile uint32_t * RCC_CFGR;
extern volatile uint32_t * RCC_PLLCFGR;
extern volatile uint32_t * RCC_IOPENR;
extern volatile uint32_t * RCC_APBENR1;
extern volatile uint32_t * RCC_APBENR2;

/* Register bit position constants */
#define RCC_CR_HSION_BIT        8u   /* HSI oscillator enable */
#define RCC_CR_HSIRDY_BIT       10u  /* HSI ready flag */
#define RCC_CR_PLLON_BIT        24u  /* PLL enable */
#define RCC_CR_PLLRDY_BIT       25u  /* PLL ready flag */

#define RCC_CFGR_SW_BIT         0u   /* System clock switch (2 bits) */
#define RCC_CFGR_SWS_BIT        3u   /* System clock switch status (2 bits) */

/* GPIO clock enable bits */
#define RCC_IOPENR_GPIOAEN_BIT  0u
#define RCC_IOPENR_GPIOBEN_BIT  1u
#define RCC_IOPENR_GPIOCEN_BIT  2u

/* APB peripheral clock enable bits */
#define RCC_APBENR2_SPI1EN_BIT  12u
#define RCC_APBENR1_TIM3EN_BIT  1u

/* Clock source values for CFGR */
#define RCC_CFGR_SW_HSI         0x0u  /* HSI as system clock */
#define RCC_CFGR_SW_PLL         0x2u  /* PLL as system clock */

/* ✓ BARR PRINCIPLE #4: Public API */

/*!
 * @brief Initialize RCC and configure system clock to 64 MHz.
 *
 * Configures HSI (16 MHz) -> PLL -> 64 MHz system clock.
 * Enables GPIO port A, B, C clocks.
 *
 * @return 0 on success, -1 on timeout/error.
 */
int32_t rcc_init(void);

/*!
 * @brief Get current system clock frequency.
 *
 * @return System clock frequency in Hz.
 */
uint32_t rcc_get_system_clock(void);

#endif /* RCC_H */

/*** end of file ***/
