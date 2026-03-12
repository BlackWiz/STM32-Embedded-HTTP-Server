/** @file startup.c
 *
 * @brief Startup code and vector table for STM32G071RB (Cortex-M0+).
 *
 * Provides:
 *   - Reset handler with boot sequence (.data copy, .bss clear,
 *     stack painting, .init_array, main)
 *   - Complete vector table with weak aliases for all handlers
 *   - HardFault handler with debug register capture
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025. All rights reserved.
 */

#include <stdint.h>
#include <stddef.h>
#include "stack_check.h"  /* For STACK_CANARY_VALUE */

/* ══════════════════════════════════════════════════════════
 *  Linker Script Symbol Declarations
 * ══════════════════════════════════════════════════════════ */
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

/* .init_array support — static constructors */
extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);

/* Stack guard region from linker */
extern uint32_t __mpu_guard_start;
extern uint32_t __mpu_guard_end;

/* ══════════════════════════════════════════════════════════
 *  Forward Declarations
 * ══════════════════════════════════════════════════════════ */
extern int main(void);
void Reset_Handler(void)   __attribute__((noreturn));
void Default_Handler(void) __attribute__((noreturn));

/* ══════════════════════════════════════════════════════════
 *  Weak Aliases — All default to Default_Handler
 *
 *  If a handler is defined elsewhere (e.g., SysTick_Handler
 *  in delay.c), the linker uses that strong definition.
 *  If NOT defined, falls back to Default_Handler safely.
 * ══════════════════════════════════════════════════════════ */

/* Core Cortex-M0+ Exception Handlers */
void NMI_Handler(void)          __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void);   /* NOT weak — defined below */
void SVCall_Handler(void)       __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)       __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)      __attribute__((weak, alias("Default_Handler")));

/* STM32G071 Peripheral Interrupt Handlers */
void WWDG_IRQHandler(void)                 __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler(void)                  __attribute__((weak, alias("Default_Handler")));
void RTC_TAMP_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void)                __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void)                  __attribute__((weak, alias("Default_Handler")));
void EXTI0_1_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void EXTI2_3_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void EXTI4_15_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void DMA_Channel1_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void DMA_Channel2_3_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void DMA_Channel4_5_6_7_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void ADC_COMP_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_UP_TRG_COM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void)                 __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void)                 __attribute__((weak, alias("Default_Handler")));
void TIM6_DAC_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void TIM7_IRQHandler(void)                 __attribute__((weak, alias("Default_Handler")));
void TIM14_IRQHandler(void)                __attribute__((weak, alias("Default_Handler")));
void TIM15_IRQHandler(void)                __attribute__((weak, alias("Default_Handler")));
void TIM16_IRQHandler(void)                __attribute__((weak, alias("Default_Handler")));
void TIM17_IRQHandler(void)                __attribute__((weak, alias("Default_Handler")));
void I2C1_IRQHandler(void)                 __attribute__((weak, alias("Default_Handler")));
void I2C2_IRQHandler(void)                 __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void)                 __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void)                 __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void USART3_4_LPUART1_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void CEC_IRQHandler(void)                  __attribute__((weak, alias("Default_Handler")));
void AES_RNG_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));

/* ══════════════════════════════════════════════════════════
 *  HardFault Debug Variables
 *  Visible in debugger when fault occurs
 * ══════════════════════════════════════════════════════════ */
static volatile uint32_t g_fault_pc  = 0;  /* Instruction that caused fault */
static volatile uint32_t g_fault_lr  = 0;  /* Return address */
static volatile uint32_t g_fault_psr = 0;  /* Program status at fault */

/* ══════════════════════════════════════════════════════════
 *  Reset Handler
 *
 *  Boot sequence:
 *  1. Copy .data from Flash to RAM
 *  2. Clear .bss to zero
 *  3. Paint stack + guard band with canary
 *  4. Call static constructors (.init_array)
 *  5. Call main()
 * ══════════════════════════════════════════════════════════ */
void Reset_Handler(void)
{
    uint32_t *src;
    uint32_t *dst;

    /* Step 1: Copy .data section from Flash to RAM */
    src = &_sidata;
    dst = &_sdata;
    while (dst < &_edata)
    {
        *dst++ = *src++;
    }

    /* Step 2: Clear .bss section in RAM */
    dst = &_sbss;
    while (dst < &_ebss)
    {
        *dst++ = 0;
    }

    /* Step 3: Paint entire stack + guard band with canary
     *
     * Paints from __mpu_guard_start up to (current SP - safety margin).
     * Cannot paint above SP because this function's own local
     * variables live there.
     *
     * At this point:
     *   SP = _estack (set by hardware from vector table index 0)
     *   SP has descended a few words for src, dst, etc.
     *   We leave 32 bytes (8 words) below SP untouched.
     */
    dst = &__mpu_guard_start;
    {
        register uint32_t *current_sp __asm("sp");
        uint32_t *safe_limit = current_sp - 8u;

        while (dst < safe_limit)
        {
            *dst++ = STACK_CANARY_VALUE;
        }
    }

    /* Step 4: Call static constructors (.init_array) */
    {
        void (**p_func)(void);
        for (p_func = __init_array_start;
             p_func < __init_array_end;
             p_func++)
        {
            (*p_func)();
        }
    }

    /* Step 5: Call main */
    main();

    /* Should never reach here — halt for debugging */
    while (1);
}

/* ══════════════════════════════════════════════════════════
 *  Vector Table
 *
 *  Placed in .isr_vector section (Flash start: 0x08000000)
 *  Cortex-M0+ fetches SP from index 0, PC from index 1.
 *
 *  Index 0      = Initial Stack Pointer (not an exception)
 *  Index 1..15  = Core exceptions (Exception Number = Index)
 *  Index 16..47 = Peripheral IRQs (IRQ# = Index - 16)
 * ══════════════════════════════════════════════════════════ */
__attribute__((section(".isr_vector"), used))
const uint32_t g_pfnVectors[] = {

    /* Index 0: Initial Stack Pointer */
    (uint32_t)&_estack,

    /* Core Cortex-M0+ Exceptions (Index = Exception Number) */
    (uint32_t)&Reset_Handler,        /*  1: Reset                        */
    (uint32_t)&NMI_Handler,          /*  2: Non-Maskable Interrupt       */
    (uint32_t)&HardFault_Handler,    /*  3: Hard Fault                   */
    0,                               /*  4: Reserved                     */
    0,                               /*  5: Reserved                     */
    0,                               /*  6: Reserved                     */
    0,                               /*  7: Reserved                     */
    0,                               /*  8: Reserved                     */
    0,                               /*  9: Reserved                     */
    0,                               /* 10: Reserved                     */
    (uint32_t)&SVCall_Handler,       /* 11: Supervisor Call              */
    0,                               /* 12: Reserved                     */
    0,                               /* 13: Reserved                     */
    (uint32_t)&PendSV_Handler,       /* 14: Pendable Service Request     */
    (uint32_t)&SysTick_Handler,      /* 15: SysTick Timer                */

    /* STM32G071 Peripheral Interrupts (IRQ# = Array Index - 16) */
    (uint32_t)&WWDG_IRQHandler,                /* IRQ  0: Window Watchdog         */
    (uint32_t)&PVD_IRQHandler,                 /* IRQ  1: PVD through EXTI        */
    (uint32_t)&RTC_TAMP_IRQHandler,            /* IRQ  2: RTC / Tamper            */
    (uint32_t)&FLASH_IRQHandler,               /* IRQ  3: Flash                   */
    (uint32_t)&RCC_IRQHandler,                 /* IRQ  4: RCC                     */
    (uint32_t)&EXTI0_1_IRQHandler,             /* IRQ  5: EXTI Line 0-1           */
    (uint32_t)&EXTI2_3_IRQHandler,             /* IRQ  6: EXTI Line 2-3           */
    (uint32_t)&EXTI4_15_IRQHandler,            /* IRQ  7: EXTI Line 4-15          */
    0,                                         /* IRQ  8: Reserved                */
    (uint32_t)&DMA_Channel1_IRQHandler,        /* IRQ  9: DMA Channel 1           */
    (uint32_t)&DMA_Channel2_3_IRQHandler,      /* IRQ 10: DMA Channel 2-3         */
    (uint32_t)&DMA_Channel4_5_6_7_IRQHandler,  /* IRQ 11: DMA Channel 4-5-6-7     */
    (uint32_t)&ADC_COMP_IRQHandler,            /* IRQ 12: ADC / Comparator        */
    (uint32_t)&TIM1_BRK_UP_TRG_COM_IRQHandler,/* IRQ 13: TIM1 Break/Update/Trig  */
    (uint32_t)&TIM1_CC_IRQHandler,             /* IRQ 14: TIM1 Capture Compare    */
    (uint32_t)&TIM2_IRQHandler,                /* IRQ 15: TIM2                    */
    (uint32_t)&TIM3_IRQHandler,                /* IRQ 16: TIM3                    */
    (uint32_t)&TIM6_DAC_IRQHandler,            /* IRQ 17: TIM6 / DAC              */
    (uint32_t)&TIM7_IRQHandler,                /* IRQ 18: TIM7                    */
    (uint32_t)&TIM14_IRQHandler,               /* IRQ 19: TIM14                   */
    (uint32_t)&TIM15_IRQHandler,               /* IRQ 20: TIM15                   */
    (uint32_t)&TIM16_IRQHandler,               /* IRQ 21: TIM16                   */
    (uint32_t)&TIM17_IRQHandler,               /* IRQ 22: TIM17                   */
    (uint32_t)&I2C1_IRQHandler,                /* IRQ 23: I2C1                    */
    (uint32_t)&I2C2_IRQHandler,                /* IRQ 24: I2C2                    */
    (uint32_t)&SPI1_IRQHandler,                /* IRQ 25: SPI1                    */
    (uint32_t)&SPI2_IRQHandler,                /* IRQ 26: SPI2                    */
    (uint32_t)&USART1_IRQHandler,              /* IRQ 27: USART1                  */
    (uint32_t)&USART2_IRQHandler,              /* IRQ 28: USART2                  */
    (uint32_t)&USART3_4_LPUART1_IRQHandler,    /* IRQ 29: USART3/4/LPUART1       */
    (uint32_t)&CEC_IRQHandler,                 /* IRQ 30: CEC                     */
    (uint32_t)&AES_RNG_IRQHandler,             /* IRQ 31: AES / RNG               */
};

/* ══════════════════════════════════════════════════════════
 *  HardFault Handler
 *
 *  Two-stage handler:
 *    Stage 1 (ASM): Capture MSP in r0 without modifying stack
 *    Stage 2 (C):   Extract stacked PC/LR/xPSR for debugging
 *
 *  On Cortex-M0+ exception entry, hardware auto-pushes:
 *    [SP+0]  R0
 *    [SP+4]  R1
 *    [SP+8]  R2
 *    [SP+12] R3
 *    [SP+16] R12
 *    [SP+20] LR    — return address of caller
 *    [SP+24] PC    — instruction that caused fault
 *    [SP+28] xPSR  — program status at fault
 *
 *  naked attribute prevents compiler prologue from
 *  modifying SP before we can capture the fault frame.
 * ══════════════════════════════════════════════════════════ */
void HardFault_Handler(void) __attribute__((naked));
void HardFault_Handler(void)
{
    __asm volatile (
        "MRS  r0, MSP          \n"  /* Get Main Stack Pointer           */
        "B    HardFault_Debug   \n"  /* Branch to C handler with SP in r0*/
    );
}

/*!
 * @brief C-level HardFault handler — receives stacked frame.
 *
 * After hitting this, connect debugger and inspect:
 *   g_fault_pc  — instruction that caused the fault
 *   g_fault_lr  — where it was called from
 *   g_fault_psr — processor state at fault time
 *
 * @param[in] stack_frame Pointer to auto-stacked exception frame.
 */
__attribute__((used, noreturn))
void HardFault_Debug(uint32_t *stack_frame)
{
    g_fault_lr  = stack_frame[5];   /* Stacked LR  */
    g_fault_pc  = stack_frame[6];   /* Stacked PC  */
    g_fault_psr = stack_frame[7];   /* Stacked xPSR */

    /* Halt here — inspect variables in debugger */
    while (1);
}

/* ══════════════════════════════════════════════════════════
 *  Default Interrupt Handler
 *  Catches any unhandled interrupt — halts for debugging
 * ══════════════════════════════════════════════════════════ */
void Default_Handler(void)
{
    while (1);
}

/*** end of file ***/