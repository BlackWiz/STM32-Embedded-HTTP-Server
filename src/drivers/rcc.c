/** @file rcc.c
 *
 * @brief RCC driver implementation for STM32G071RB.
 *
 * Configures system clock using HSI and PLL.
 * Follows Michael Barr's 5 driver design principles.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2026 Hari. All rights reserved.
 */

#include <stdint.h>
#include "rcc.h"
#include "types.h"

/* ✓ BARR PRINCIPLE #1: Register Definitions (Implementation) */

volatile uint32_t * RCC = (uint32_t *) 0x40021000;
volatile uint32_t * RCC_CR      = (uint32_t *) 0x40021000;
volatile uint32_t * RCC_CFGR    = (uint32_t *) 0x40021008;
volatile uint32_t * RCC_PLLCFGR = (uint32_t *) 0x4002100C;
volatile uint32_t * RCC_IOPENR  = (uint32_t *) 0x40021034;
volatile uint32_t * RCC_APBENR1 = (uint32_t *) 0x4002103C;
volatile uint32_t * RCC_APBENR2 = (uint32_t *) 0x40021040;

/* ✓ BARR PRINCIPLE #2: State Tracking Variables */

/* Global state variables for RCC driver */
static volatile bool_t g_rcc_initialized = FALSE;
static volatile uint32_t g_system_clock_hz = 16000000u;  /* Default HSI */

/* Timeout for clock ready flags (in loop iterations) */
#define RCC_TIMEOUT_COUNT  100000u

/* ✓ BARR PRINCIPLE #3: Hardware Initialization Routine */

/*!
 * @brief Initialize RCC and configure system clock to 64 MHz.
 *
 * Configuration: HSI (16 MHz) -> PLL (x8, /2) -> 64 MHz
 *
 * @return 0 on success, -1 on timeout.
 */
int32_t
rcc_init (void)
{
    uint32_t timeout;
    
    /* Already initialized - avoid reconfiguration */
    if (g_rcc_initialized) {
        return 0;
    }
    
    /* Step 1: Enable HSI oscillator */
    *RCC_CR |= (1u << RCC_CR_HSION_BIT);
    
    /* Wait for HSI ready */
    timeout = 0u;
    while (((*RCC_CR & (1u << RCC_CR_HSIRDY_BIT)) == 0u) && (timeout < RCC_TIMEOUT_COUNT))
    {
        timeout++;
    }
    
    if (timeout >= RCC_TIMEOUT_COUNT) {
        return -1;  /* HSI failed to start */
    }
    
    /* Step 2: Configure PLL (HSI / 2 * 8 = 64 MHz) */
    /* TODO: PLL configuration - for now, use HSI directly (16 MHz) */
    /* Will implement PLL after basic system works */
    
    /* Step 3: Enable GPIO clocks (needed for LED and SPI pins) */
    *RCC_IOPENR |= (1u << RCC_IOPENR_GPIOAEN_BIT);  /* GPIOA */
    *RCC_IOPENR |= (1u << RCC_IOPENR_GPIOBEN_BIT);  /* GPIOB */
    *RCC_IOPENR |= (1u << RCC_IOPENR_GPIOCEN_BIT);  /* GPIOC */
    
    /* Step 4: Enable SPI1 clock (needed for ENC28J60) */
    *RCC_APBENR2 |= (1u << RCC_APBENR2_SPI1EN_BIT);
    
    /* Update state */
    g_system_clock_hz = 16000000u;  /* HSI frequency */
    g_rcc_initialized = TRUE;
    
    return 0;
}

/* ✓ BARR PRINCIPLE #4: Driver API Functions */

/*!
 * @brief Get current system clock frequency.
 *
 * @return System clock frequency in Hz.
 */
uint32_t
rcc_get_system_clock (void)
{
    return g_system_clock_hz;
}

/* ✓ BARR PRINCIPLE #5: ISRs */
/* RCC does not require interrupt service routines */

/*** end of file ***/
