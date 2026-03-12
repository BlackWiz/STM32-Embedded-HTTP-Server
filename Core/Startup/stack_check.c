/** @file stack_check.c
 *
 * @brief Runtime stack overflow detection and usage monitoring.
 *
 * Works in conjunction with startup.c which paints the stack
 * region with STACK_CANARY_VALUE at boot time.
 *
 * Memory layout (from linker script):
 *
 *   _estack            ┌──────────────┐  (top of RAM)
 *                      │  Stack ↓     │
 *                      │              │
 *   __stack_bottom     ├──────────────┤
 *   __mpu_guard_end    │  Guard Band  │  32 bytes
 *   __mpu_guard_start  ├──────────────┤
 *                      │  Heap ↑      │
 *                      │              │
 *   _end               ├──────────────┤
 *                      │  .bss        │
 *                      │  .data       │
 *                      └──────────────┘  (bottom of RAM)
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 Your Name. All rights reserved.
 */

#include <stdint.h>
#include <stddef.h>
#include "stack_check.h"

/* ══════════════════════════════════════════════════════════
 *  Linker Script Symbols
 * ══════════════════════════════════════════════════════════ */
extern uint32_t __mpu_guard_start;
extern uint32_t __mpu_guard_end;
extern uint32_t __stack_bottom;
extern uint32_t _estack;

/*!
 * @brief Check stack integrity and measure usage.
 *
 * Two-phase check:
 *   Phase 1: Verify guard band is intact (overflow detection)
 *   Phase 2: Scan painted region for high water mark (usage measurement)
 *
 * @param[out] p_used_bytes If not NULL, receives number of stack bytes used.
 *
 * @return 0 if stack OK, 1 if overflow detected.
 */
uint32_t stack_check(uint32_t *p_used_bytes)
{
    uint32_t *guard;
    uint32_t *scan;

    /* Phase 1: Check guard band integrity
     * If ANY word in the guard band is corrupted, the stack
     * has overflowed past its entire region.
     */
    guard = &__mpu_guard_start;
    while (guard < &__mpu_guard_end)
    {
        if (*guard != STACK_CANARY_VALUE)
        {
            return 1u;   /* OVERFLOW — guard band corrupted */
        }
        guard++;
    }

    /* Phase 2: Find high water mark
     * Scan upward from stack bottom. The first word that
     * is NOT 0xDEADBEEF marks the deepest stack usage.
     *
     *   _estack  ┌────────────────┐
     *            │ Used by stack  │ ← overwritten by function calls
     *            │ Used by stack  │
     *     scan → │ 0xDEADBEEF     │ ← high water mark (deepest usage)
     *            │ 0xDEADBEEF     │ ← still untouched
     *            │ 0xDEADBEEF     │
     *   bottom   └────────────────┘
     */
    scan = &__stack_bottom;
    while ((scan < &_estack) && (*scan == STACK_CANARY_VALUE))
    {
        scan++;
    }

    /* Calculate bytes used: distance from high water mark to top */
    if (NULL != p_used_bytes)
    {
        *p_used_bytes = (uint32_t)((uint8_t *)&_estack - (uint8_t *)scan);
    }

    return 0u;   /* Stack OK */
}

/*** end of file ***/