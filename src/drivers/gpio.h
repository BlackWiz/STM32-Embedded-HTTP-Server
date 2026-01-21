/** @file gpio.h
 *
 * @brief GPIO driver interface for STM32G071RB.
 *
 * Application-specific GPIO control for LED and SPI pins.
 * NOT a generic HAL-style GPIO library.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2026 Hari. All rights reserved.
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "types.h"

/* ✓ BARR PRINCIPLE #1: Memory-Mapped Register Pointers */

/* Defining GPIO Registers used */
/*
In this project, GPIO is used for:
1. PA5 --> Onboard LED (output)
2. PA5, PA6, PA7 --> SPI1 pins (alternate function) - will configure later

GPIOx_MODER   --> At an Offset of 0x00
GPIOx_OSPEEDR --> At an Offset of 0x08
GPIOx_IDR     --> At an Offset of 0x10
GPIOx_ODR     --> At an Offset of 0x14
GPIOx_BSRR    --> At an Offset of 0x18
GPIOx_AFRL    --> At an Offset of 0x20
*/
extern volatile uint32_t * GPIOA;
extern volatile uint32_t * GPIOA_MODER;
extern volatile uint32_t * GPIOA_OSPEEDR;
extern volatile uint32_t * GPIOA_IDR;
extern volatile uint32_t * GPIOA_ODR;
extern volatile uint32_t * GPIOA_BSRR;
extern volatile uint32_t * GPIOA_AFRL;

/* Pin configuration constants */
#define PA5_PIN_NUM            5u
#define PA6_PIN_NUM            6u
#define PA7_PIN_NUM            7u
#define BITS_PER_PIN           2u
#define AFR_BITS_PER_PIN       4u

/* Mode values */
#define GPIO_MODE_OUTPUT       0x1u
#define GPIO_MODE_AF           0x2u

/* Speed values */
#define GPIO_SPEED_LOW         0x0u
#define GPIO_SPEED_MEDIUM      0x1u

/* ✓ BARR PRINCIPLE #4: Public API (Application-Specific) */

/*!
 * @brief Initialize GPIO driver and configure PA5 for LED.
 *
 * Configures PA5 as output, low speed, LED off by default.
 * RCC must be initialized first.
 *
 * @return 0 on success.
 */
int32_t gpio_init(void);

/*!
 * @brief Turn LED on (PA5 high).
 */
void gpio_led_on(void);

/*!
 * @brief Turn LED off (PA5 low).
 */
void gpio_led_off(void);

/*!
 * @brief Toggle LED state (PA5).
 */
void gpio_led_toggle(void);

/*!
 * @brief Read LED state.
 *
 * @return TRUE if LED is on, FALSE if off.
 */
bool_t gpio_led_read(void);

/*!
 * @brief Configure GPIO pins for SPI1 (PA5, PA6, PA7).
 *
 * Must be called before spi_init().
 */
void gpio_configure_spi_pins(void);

#endif /* GPIO_H */

/*** end of file ***/
