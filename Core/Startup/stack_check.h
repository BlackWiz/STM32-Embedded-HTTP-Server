/** @file stack_check.h
 *
 * @brief Runtime stack overflow detection and usage monitoring.
 *
 * Uses stack painting (0xDEADBEEF canary) combined with a guard
 * band region to detect stack overflow and measure high water mark.
 *
 * Dependencies:
 *   - Linker script must define __mpu_guard_start, __mpu_guard_end,
 *     __stack_bottom, and _estack symbols.
 *   - startup.c must paint the stack with STACK_CANARY_VALUE at boot.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 Your Name. All rights reserved.
 */

#ifndef STACK_CHECK_H
#define STACK_CHECK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Stack canary magic value — shared between startup.c and stack_check.c */
#define STACK_CANARY_VALUE  0xDEADBEEFu

/*!
 * @brief Check stack integrity and measure usage.
 *
 * Examines the MPU guard band for corruption (overflow detection)
 * and scans the painted stack region to determine high water mark.
 *
 * Usage:
 *   uint32_t used = 0;
 *   if (stack_check(&used) != 0u) {
 *       // PANIC — stack overflow detected!
 *   }
 *   // used = number of stack bytes consumed so far
 *
 * @param[out] p_used_bytes If not NULL, receives number of stack bytes used.
 *
 * @return 0 if stack OK, 1 if overflow detected.
 */
uint32_t stack_check(uint32_t *p_used_bytes);

#ifdef __cplusplus
}
#endif

#endif /* STACK_CHECK_H */

/*** end of file ***/