/** @file gpio.c
 *
 * @brief GPIO driver implementation for STM32G071RB.
 *
 * Application-specific GPIO control for LED (PA5).
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2026 Hari. All rights reserved.
 */

#include <stdint.h>
#include "gpio.h"
#include "types.h"

/* ✓ BARR PRINCIPLE #1: Register Definitions (Implementation) */

volatile uint32_t * GPIOA = (uint32_t *) 0x50000000;
volatile uint32_t * GPIOA_MODER   = (uint32_t *) 0x50000000;
volatile uint32_t * GPIOA_OSPEEDR = (uint32_t *) 0x50000008;
volatile uint32_t * GPIOA_IDR     = (uint32_t *) 0x50000010;
volatile uint32_t * GPIOA_ODR     = (uint32_t *) 0x50000014;
volatile uint32_t * GPIOA_BSRR    = (uint32_t *) 0x50000018;
volatile uint32_t * GPIOA_AFRL    = (uint32_t *) 0x50000020;

/* ✓ BARR PRINCIPLE #2: State Tracking Variables */

/* Global state variable */
static volatile bool_t g_gpio_initialized = FALSE;

/* ✓ BARR PRINCIPLE #3: Hardware Initialization Routine */

/*!
 * @brief Initialize GPIO driver and configure PA5 for LED.
 *
 * @return 0 on success.
 */
int32_t
gpio_init (void)
{
    /* RCC must have enabled GPIOA clock already */
    
    /* Configure PA5 as output for LED */
    *GPIOA_MODER &= ~(0x3u << (BITS_PER_PIN * PA5_PIN_NUM));  /* Clear bits */
    *GPIOA_MODER |= (GPIO_MODE_OUTPUT << (BITS_PER_PIN * PA5_PIN_NUM));  /* Set output */
    
    /* Set speed to low */
    *GPIOA_OSPEEDR &= ~(0x3u << (BITS_PER_PIN * PA5_PIN_NUM));
    *GPIOA_OSPEEDR |= (GPIO_SPEED_LOW << (BITS_PER_PIN * PA5_PIN_NUM));
    
    /* LED off by default (using BSRR for atomic operation) */
    *GPIOA_BSRR = (1u << (PA5_PIN_NUM + 16u));  /* Reset bit */
    
    g_gpio_initialized = TRUE;
    
    return 0;
}

/* ✓ BARR PRINCIPLE #4: Driver API Functions */

/*!
 * @brief Turn LED on (PA5 high).
 *
 * Uses BSRR register for atomic bit set operation.
 */
void
gpio_led_on (void)
{
    *GPIOA_BSRR = (1u << PA5_PIN_NUM);  /* Set bit - atomic */
}

/*!
 * @brief Turn LED off (PA5 low).
 *
 * Uses BSRR register for atomic bit reset operation.
 */
void
gpio_led_off (void)
{
    *GPIOA_BSRR = (1u << (PA5_PIN_NUM + 16u));  /* Reset bit - atomic */
}

/*!
 * @brief Toggle LED state (PA5).
 *
 * Uses XOR on ODR register to toggle.
 */
void
gpio_led_toggle (void)
{
    *GPIOA_ODR ^= (1u << PA5_PIN_NUM);  /* XOR toggle */
}

/*!
 * @brief Read LED state.
 *
 * @return TRUE if LED is on, FALSE if off.
 */
bool_t
gpio_led_read (void)
{
    return ((*GPIOA_IDR & (1u << PA5_PIN_NUM)) != 0u) ? TRUE : FALSE;
}

/*!
 * @brief Configure GPIO pins for SPI1 (PA5, PA6, PA7).
 *
 * PA5 = SCK (Alternate Function AF0)
 * PA6 = MISO (Alternate Function AF0)
 * PA7 = MOSI (Alternate Function AF0)
 *
 * Must be called before spi_init().
 */
void
gpio_configure_spi_pins (void)
{
    /* Set PA5, PA6, PA7 to alternate function mode */
    *GPIOA_MODER &= ~((0x3u << (BITS_PER_PIN * PA5_PIN_NUM)) |
                       (0x3u << (BITS_PER_PIN * PA6_PIN_NUM)) |
                       (0x3u << (BITS_PER_PIN * PA7_PIN_NUM)));
    *GPIOA_MODER |= (GPIO_MODE_AF << (BITS_PER_PIN * PA5_PIN_NUM)) |
                     (GPIO_MODE_AF << (BITS_PER_PIN * PA6_PIN_NUM)) |
                     (GPIO_MODE_AF << (BITS_PER_PIN * PA7_PIN_NUM));
    
    /* Set AF0 for SPI1 on PA5, PA6, PA7 */
    /* AFRL handles pins 0-7, each pin uses 4 bits */
    *GPIOA_AFRL &= ~((0xFu << (AFR_BITS_PER_PIN * PA5_PIN_NUM)) |
                      (0xFu << (AFR_BITS_PER_PIN * PA6_PIN_NUM)) |
                      (0xFu << (AFR_BITS_PER_PIN * PA7_PIN_NUM)));
    /* AF0 = 0x0, so no need to set bits (already cleared) */
    
    /* Set pins to high speed for SPI */
    *GPIOA_OSPEEDR |= (GPIO_SPEED_MEDIUM << (BITS_PER_PIN * PA5_PIN_NUM)) |
                       (GPIO_SPEED_MEDIUM << (BITS_PER_PIN * PA6_PIN_NUM)) |
                       (GPIO_SPEED_MEDIUM << (BITS_PER_PIN * PA7_PIN_NUM));
}

/* ✓ BARR PRINCIPLE #5: ISRs */
/* GPIO does not require interrupt service routines for this application */

/*** end of file ***/
