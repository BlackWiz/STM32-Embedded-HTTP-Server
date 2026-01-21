/** @file main.c
 *
 * @brief HTTP-JSON-Assistant main application entry point.
 *
 * Phase-1 MVP: Simple LED blink to verify basic system operation.
 * Will expand to full HTTP server with JSON command processing.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2026 Hari. All rights reserved.
 */

#include <stdint.h>
#include "types.h"
#include "drivers/rcc.h"
#include "drivers/gpio.h"
#include "drivers/delay.h"

/*!
 * @brief Main application entry point.
 *
 * Current functionality: Blink onboard LED at 1 Hz.
 *
 * @return Never returns (embedded main loop).
 */
int
main (void)
{
    /* Initialize RCC - must be first! */
    rcc_init();
    
    /* Initialize delay subsystem (SysTick) */
    delay_init();
    
    /* Initialize GPIO for LED control */
    gpio_init();
    
    /* Main loop - blink LED at 1 Hz */
    for (;;)
    {
        gpio_led_toggle();
        delay_ms(500u);
    }
    
    return 0;
}

/*** end of file ***/
