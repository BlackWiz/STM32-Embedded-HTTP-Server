/** @file rcc.c
 *
 * @brief RCC (Reset and Clock Control) driver for STM32G071RB.
 *
 * Implements system clock configuration with support for:
 *   - HSI16 (16 MHz, default at reset)
 *   - HSE (external crystal/clock)
 *   - PLL (up to 64 MHz from HSI16 or HSE)
 *
 * Critical sequencing: Flash wait states MUST be configured
 * BEFORE increasing clock frequency. Failure to do so causes
 * HardFault due to Flash access violations.
 *
 * Reference: RM0444 Rev 5
 *   - Section 5.2: RCC registers
 *   - Section 3.3.4: Flash ACR / wait states
 *   - Section 5.4.3: PLL configuration
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 Your Name. All rights reserved.
 */

#include <stdint.h>
#include <stddef.h>
#include "rcc.h"

/* ══════════════════════════════════════════════════════════
 *  Driver State (Barr Rule 2)
 *
 *  Tracks current clock frequencies. Initialized to reset
 *  defaults (HSI16 with no prescalers).
 * ══════════════════════════════════════════════════════════ */
static rcc_clocks_t g_clocks = {
    .sysclk_freq = HSI16_VALUE,
    .hclk_freq   = HSI16_VALUE,
    .pclk_freq   = HSI16_VALUE
};

/* ══════════════════════════════════════════════════════════
 *  Prescaler Lookup Tables
 *
 *  Indexed by register field value to get actual divisor.
 *  Avoids complex switch statements in frequency calculation.
 * ══════════════════════════════════════════════════════════ */

/*!
 * AHB prescaler: HPRE field bits [11:8] of RCC_CFGR
 *   0xxx → ÷1 (not divided)
 *   1000 → ÷2,  1001 → ÷4,  1010 → ÷8,   1011 → ÷16
 *   1100 → ÷64, 1101 → ÷128, 1110 → ÷256, 1111 → ÷512
 */
static const uint16_t g_ahb_prescaler_table[16] = {
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
    2u, 4u, 8u, 16u, 64u, 128u, 256u, 512u
};

/*!
 * APB prescaler: PPRE field bits [14:12] of RCC_CFGR
 *   0xx → ÷1 (not divided)
 *   100 → ÷2, 101 → ÷4, 110 → ÷8, 111 → ÷16
 */
static const uint8_t g_apb_prescaler_table[8] = {
    1u, 1u, 1u, 1u,
    2u, 4u, 8u, 16u
};

/* ══════════════════════════════════════════════════════════
 *  Static Helper Declarations
 * ══════════════════════════════════════════════════════════ */
static int32_t  rcc_config_flash_latency(uint32_t target_hclk_hz);
static int32_t  rcc_config_pll(const rcc_pll_config_t *p_pll);
static int32_t  rcc_switch_sysclk(rcc_sysclk_src_t source);
static void     rcc_compute_clocks(const rcc_config_t *p_config);
static uint32_t rcc_calc_pll_output(const rcc_pll_config_t *p_pll);
static int32_t  rcc_validate_pll_config(const rcc_pll_config_t *p_pll);

/* ══════════════════════════════════════════════════════════
 *  Flash Wait State Configuration
 *
 *  ┌──────────────────────────────────────┐
 *  │  HCLK Range          Wait States     │
 *  ├──────────────────────────────────────┤
 *  │  0  - 24 MHz         0 WS           │
 *  │  24 - 48 MHz         1 WS           │
 *  │  48 - 64 MHz         2 WS           │
 *  └──────────────────────────────────────┘
 *
 *  CRITICAL: Must be called BEFORE increasing clock speed!
 *  More wait states than needed is safe (just slower).
 *  Fewer wait states than needed causes HardFault!
 * ══════════════════════════════════════════════════════════ */
static int32_t rcc_config_flash_latency(uint32_t target_hclk_hz)
{
    uint32_t latency;
    uint32_t timeout = RCC_TIMEOUT_VALUE;

    if (target_hclk_hz <= 24000000UL)
    {
        latency = FLASH_ACR_LATENCY_0WS;
    }
    else if (target_hclk_hz <= 48000000UL)
    {
        latency = FLASH_ACR_LATENCY_1WS;
    }
    else
    {
        latency = FLASH_ACR_LATENCY_2WS;
    }

    /* Modify only the latency bits, preserve prefetch/cache settings */
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | latency;

    /* Readback verify — critical for flash configuration */
    while (((FLASH->ACR & FLASH_ACR_LATENCY_Msk) != latency) && (timeout > 0u))
    {
        timeout--;
    }

    if (0u == timeout)
    {
        return -1;
    }

    return 0;
}

/* ══════════════════════════════════════════════════════════
 *  PLL Parameter Validation
 *
 *  Checks:
 *  - Valid source (HSI16 or HSE)
 *  - M divider range: 1-8
 *  - N multiplier range: 8-86
 *  - R divider range: 2-8
 *  - VCO input frequency: 2.66 - 16 MHz
 *  - VCO output frequency: 64 - 344 MHz
 *  - Final output: ≤ 64 MHz
 * ══════════════════════════════════════════════════════════ */
static int32_t rcc_validate_pll_config(const rcc_pll_config_t *p_pll)
{
    uint32_t vco_input;
    uint32_t vco_output;
    uint32_t pll_output;
    uint32_t source_freq;

    /* Validate source */
    if (RCC_PLL_SRC_HSI16 == p_pll->source)
    {
        source_freq = HSI16_VALUE;
    }
    else if (RCC_PLL_SRC_HSE == p_pll->source)
    {
        source_freq = HSE_VALUE;
    }
    else
    {
        return -1;
    }

    /* Validate M range */
    if ((p_pll->m < 1u) || (p_pll->m > 8u))
    {
        return -1;
    }

    /* Validate N range */
    if ((p_pll->n < 8u) || (p_pll->n > 86u))
    {
        return -1;
    }

    /* Validate R range */
    if ((p_pll->r < 2u) || (p_pll->r > 8u))
    {
        return -1;
    }

    /* Validate VCO input: must be 2.66 - 16 MHz */
    vco_input = source_freq / p_pll->m;
    if ((vco_input < 2660000UL) || (vco_input > 16000000UL))
    {
        return -1;
    }

    /* Validate VCO output: must be 64 - 344 MHz */
    vco_output = vco_input * p_pll->n;
    if ((vco_output < 64000000UL) || (vco_output > 344000000UL))
    {
        return -1;
    }

    /* Validate final output: must be ≤ 64 MHz */
    pll_output = vco_output / p_pll->r;
    if (pll_output > 64000000UL)
    {
        return -1;
    }

    return 0;
}

/* ══════════════════════════════════════════════════════════
 *  Calculate PLL Output Frequency
 *
 *  Formula: PLL_out = (source / M) × N / R
 *
 *  Example for 64 MHz from HSI16:
 *    (16 MHz / 1) × 8 / 2 = 64 MHz
 * ══════════════════════════════════════════════════════════ */
static uint32_t rcc_calc_pll_output(const rcc_pll_config_t *p_pll)
{
    uint32_t source_freq;

    if (RCC_PLL_SRC_HSI16 == p_pll->source)
    {
        source_freq = HSI16_VALUE;
    }
    else
    {
        source_freq = HSE_VALUE;
    }

    return ((source_freq / p_pll->m) * p_pll->n) / p_pll->r;
}

/* ══════════════════════════════════════════════════════════
 *  PLL Configuration
 *
 *  Sequence (from RM0444 Section 5.4.3):
 *  1. Disable PLL (cannot modify while running)
 *  2. Wait for PLLRDY to clear
 *  3. Enable PLL source clock (HSI16 or HSE)
 *  4. Wait for source ready
 *  5. Configure M, N, R dividers
 *  6. Enable PLLR output
 *  7. Enable PLL
 *  8. Wait for PLLRDY
 * ══════════════════════════════════════════════════════════ */
static int32_t rcc_config_pll(const rcc_pll_config_t *p_pll)
{
    uint32_t pllcfgr;
    uint32_t timeout;

    /* Validate PLL parameters before touching any registers */
    if (rcc_validate_pll_config(p_pll) != 0)
    {
        return -1;
    }

    /* Step 1: Disable PLL */
    RCC->CR &= ~RCC_CR_PLLON;

    /* Step 2: Wait for PLL to fully stop */
    timeout = RCC_TIMEOUT_VALUE;
    while ((RCC->CR & RCC_CR_PLLRDY) && (timeout > 0u))
    {
        timeout--;
    }
    if (0u == timeout)
    {
        return -1;
    }

    /* Step 3-4: Enable PLL source clock and wait for ready */
    if (RCC_PLL_SRC_HSI16 == p_pll->source)
    {
        RCC->CR |= RCC_CR_HSION;

        timeout = RCC_TIMEOUT_VALUE;
        while (!(RCC->CR & RCC_CR_HSIRDY) && (timeout > 0u))
        {
            timeout--;
        }
        if (0u == timeout)
        {
            return -1;
        }
    }
    else /* RCC_PLL_SRC_HSE */
    {
        RCC->CR |= RCC_CR_HSEON;

        timeout = RCC_TIMEOUT_VALUE;
        while (!(RCC->CR & RCC_CR_HSERDY) && (timeout > 0u))
        {
            timeout--;
        }
        if (0u == timeout)
        {
            return -1;
        }
    }

    /* Step 5: Build PLLCFGR register value
     *
     * Register field encoding:
     *   PLLSRC [1:0]  : 10 = HSI16, 11 = HSE
     *   PLLM   [6:4]  : actual_M - 1 (0 means ÷1, 7 means ÷8)
     *   PLLN   [14:8]  : actual_N directly (8-86)
     *   PLLR   [31:29] : actual_R - 1 (1 means ÷2, 7 means ÷8)
     */
    pllcfgr = 0;

    /* Source */
    if (RCC_PLL_SRC_HSI16 == p_pll->source)
    {
        pllcfgr |= RCC_PLLCFGR_PLLSRC_HSI;
    }
    else
    {
        pllcfgr |= RCC_PLLCFGR_PLLSRC_HSE;
    }

    /* M divider */
    pllcfgr |= ((p_pll->m - 1u) << RCC_PLLCFGR_PLLM_Pos);

    /* N multiplier */
    pllcfgr |= (p_pll->n << RCC_PLLCFGR_PLLN_Pos);

    /* R divider */
    pllcfgr |= ((p_pll->r - 1u) << RCC_PLLCFGR_PLLR_Pos);

    /* Step 6: Enable PLLR output */
    pllcfgr |= RCC_PLLCFGR_PLLREN;

    /* Write complete configuration in one shot */
    RCC->PLLCFGR = pllcfgr;

    /* Step 7: Enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Step 8: Wait for PLL lock */
    timeout = RCC_TIMEOUT_VALUE;
    while (!(RCC->CR & RCC_CR_PLLRDY) && (timeout > 0u))
    {
        timeout--;
    }
    if (0u == timeout)
    {
        return -1;
    }

    return 0;
}

/* ══════════════════════════════════════════════════════════
 *  System Clock Switch
 *
 *  Writes SW bits in CFGR, then verifies by reading SWS.
 *  Times out if switch doesn't complete.
 * ══════════════════════════════════════════════════════════ */
static int32_t rcc_switch_sysclk(rcc_sysclk_src_t source)
{
    uint32_t sw_value;
    uint32_t sws_value;
    uint32_t timeout;

    switch (source)
    {
        case RCC_SYSCLK_SRC_HSI16:
            sw_value  = RCC_CFGR_SW_HSISYS;
            sws_value = RCC_CFGR_SWS_HSISYS;
            break;

        case RCC_SYSCLK_SRC_HSE:
            sw_value  = RCC_CFGR_SW_HSE;
            sws_value = RCC_CFGR_SWS_HSE;
            break;

        case RCC_SYSCLK_SRC_PLL:
            sw_value  = RCC_CFGR_SW_PLLRCLK;
            sws_value = RCC_CFGR_SWS_PLLRCLK;
            break;

        case RCC_SYSCLK_SRC_LSI:
            sw_value  = RCC_CFGR_SW_LSI;
            sws_value = RCC_CFGR_SWS_LSI;
            break;

        case RCC_SYSCLK_SRC_LSE:
            sw_value  = RCC_CFGR_SW_LSE;
            sws_value = RCC_CFGR_SWS_LSE;
            break;

        default:
            return -1;
    }

    /* Set SW bits — clear old value first, then set new */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | sw_value;

    /* Wait for SWS to confirm switch */
    timeout = RCC_TIMEOUT_VALUE;
    while (((RCC->CFGR & RCC_CFGR_SWS_Msk) != sws_value) && (timeout > 0u))
    {
        timeout--;
    }

    if (0u == timeout)
    {
        return -1;
    }

    return 0;
}

/* ══════════════════════════════════════════════════════════
 *  Compute and Store Clock Frequencies
 *
 *  Called after successful clock configuration to update
 *  the driver state with actual frequencies.
 * ══════════════════════════════════════════════════════════ */
static void rcc_compute_clocks(const rcc_config_t *p_config)
{
    uint32_t ahb_div_index;
    uint32_t apb_div_index;

    /* Determine SYSCLK */
    switch (p_config->sysclk_source)
    {
        case RCC_SYSCLK_SRC_HSI16:
            g_clocks.sysclk_freq = HSI16_VALUE;
            break;

        case RCC_SYSCLK_SRC_HSE:
            g_clocks.sysclk_freq = HSE_VALUE;
            break;

        case RCC_SYSCLK_SRC_PLL:
            g_clocks.sysclk_freq = rcc_calc_pll_output(&p_config->pll_config);
            break;

        case RCC_SYSCLK_SRC_LSI:
            g_clocks.sysclk_freq = LSI_VALUE;
            break;

        case RCC_SYSCLK_SRC_LSE:
            g_clocks.sysclk_freq = LSE_VALUE;
            break;

        default:
            g_clocks.sysclk_freq = HSI16_VALUE;
            break;
    }

    /* Compute HCLK = SYSCLK / AHB prescaler */
    ahb_div_index = (uint32_t)p_config->ahb_prescaler & 0xFu;
    g_clocks.hclk_freq = g_clocks.sysclk_freq / g_ahb_prescaler_table[ahb_div_index];

    /* Compute PCLK = HCLK / APB prescaler */
    apb_div_index = (uint32_t)p_config->apb_prescaler & 0x7u;
    g_clocks.pclk_freq = g_clocks.hclk_freq / g_apb_prescaler_table[apb_div_index];
}

/* ══════════════════════════════════════════════════════════
 *  PUBLIC API — Initialization (Barr Rule 3)
 *
 *  Complete clock configuration sequence:
 *
 *  ┌────────────────────────────────────────┐
 *  │ 1. Calculate target HCLK frequency     │
 *  │ 2. Set flash wait states (BEFORE!)     │
 *  │ 3. Configure prescalers (AHB, APB)     │
 *  │ 4. Configure clock source (PLL if used)│
 *  │ 5. Switch SYSCLK to new source         │
 *  │ 6. Update driver state                 │
 *  │ 7. Enable prefetch + instruction cache │
 *  └────────────────────────────────────────┘
 * ══════════════════════════════════════════════════════════ */

/*!
 * @brief Initialize system clocks per configuration.
 *
 * Example — 64 MHz from HSI16 via PLL:
 * @code
 *   const rcc_config_t clk_cfg = {
 *       .sysclk_source = RCC_SYSCLK_SRC_PLL,
 *       .pll_config = {
 *           .source = RCC_PLL_SRC_HSI16,
 *           .m = 1,     // 16 / 1 = 16 MHz VCO input
 *           .n = 8,     // 16 * 8 = 128 MHz VCO output
 *           .r = 2      // 128 / 2 = 64 MHz SYSCLK
 *       },
 *       .ahb_prescaler = RCC_AHB_DIV_1,
 *       .apb_prescaler = RCC_APB_DIV_1
 *   };
 *   rcc_init(&clk_cfg);
 * @endcode
 *
 * @param[in] p_config Pointer to clock configuration. NULL uses HSI16 default.
 * @return 0 on success, -1 on error/timeout.
 */
int32_t rcc_init(const rcc_config_t *p_config)
{
    uint32_t target_sysclk;
    uint32_t target_hclk;
    uint32_t ahb_div_index;

    /* Default configuration: HSI16, no prescalers */
    static const rcc_config_t default_config = {
        .sysclk_source = RCC_SYSCLK_SRC_HSI16,
        .pll_config    = { .source = RCC_PLL_SRC_NONE, .m = 1u, .n = 8u, .r = 2u },
        .ahb_prescaler = RCC_AHB_DIV_1,
        .apb_prescaler = RCC_APB_DIV_1
    };

    /* Use default if NULL provided */
    if (NULL == p_config)
    {
        p_config = &default_config;
    }

    /* ── Step 1: Calculate target frequencies ── */
    switch (p_config->sysclk_source)
    {
        case RCC_SYSCLK_SRC_HSI16:
            target_sysclk = HSI16_VALUE;
            break;

        case RCC_SYSCLK_SRC_HSE:
            target_sysclk = HSE_VALUE;
            break;

        case RCC_SYSCLK_SRC_PLL:
            target_sysclk = rcc_calc_pll_output(&p_config->pll_config);
            break;

        case RCC_SYSCLK_SRC_LSI:
            target_sysclk = LSI_VALUE;
            break;

        case RCC_SYSCLK_SRC_LSE:
            target_sysclk = LSE_VALUE;
            break;

        default:
            return -1;
    }

    ahb_div_index = (uint32_t)p_config->ahb_prescaler & 0xFu;
    target_hclk = target_sysclk / g_ahb_prescaler_table[ahb_div_index];

    /* ── Step 2: Set flash wait states for TARGET frequency ──
     *
     * ALWAYS set wait states BEFORE increasing clock.
     * Extra wait states are safe (just slightly slower).
     * Too few wait states → HardFault!
     */
    if (rcc_config_flash_latency(target_hclk) != 0)
    {
        return -1;
    }

    /* ── Step 3: Configure bus prescalers ── */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE_Msk) |
                ((uint32_t)p_config->ahb_prescaler << RCC_CFGR_HPRE_Pos);

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE_Msk) |
                ((uint32_t)p_config->apb_prescaler << RCC_CFGR_PPRE_Pos);

    /* ── Step 4: Configure clock source ── */
    switch (p_config->sysclk_source)
    {
        case RCC_SYSCLK_SRC_HSI16:
        {
            /* HSI16 is already running at reset — just make sure */
            RCC->CR |= RCC_CR_HSION;
            uint32_t timeout = RCC_TIMEOUT_VALUE;
            while (!(RCC->CR & RCC_CR_HSIRDY) && (timeout > 0u))
            {
                timeout--;
            }
            if (0u == timeout)
            {
                return -1;
            }
            break;
        }

        case RCC_SYSCLK_SRC_HSE:
        {
            /* Enable HSE and wait for stabilization */
            RCC->CR |= RCC_CR_HSEON;
            uint32_t timeout = RCC_TIMEOUT_VALUE;
            while (!(RCC->CR & RCC_CR_HSERDY) && (timeout > 0u))
            {
                timeout--;
            }
            if (0u == timeout)
            {
                return -1;
            }
            break;
        }

        case RCC_SYSCLK_SRC_PLL:
        {
            /* Full PLL configuration sequence */
            if (rcc_config_pll(&p_config->pll_config) != 0)
            {
                return -1;
            }
            break;
        }

        case RCC_SYSCLK_SRC_LSI:
        {
            /* Enable LSI */
            RCC->CSR |= RCC_CSR_LSION;
            uint32_t timeout = RCC_TIMEOUT_VALUE;
            while (!(RCC->CSR & RCC_CSR_LSIRDY) && (timeout > 0u))
            {
                timeout--;
            }
            if (0u == timeout)
            {
                return -1;
            }
            break;
        }

        case RCC_SYSCLK_SRC_LSE:
        {
            /* Enable LSE */
            RCC->BDCR |= RCC_BDCR_LSEON;
            uint32_t timeout = RCC_TIMEOUT_VALUE;
            while (!(RCC->BDCR & RCC_BDCR_LSERDY) && (timeout > 0u))
            {
                timeout--;
            }
            if (0u == timeout)
            {
                return -1;
            }
            break;
        }

        default:
            return -1;
    }

    /* ── Step 5: Switch SYSCLK to new source ── */
    if (rcc_switch_sysclk(p_config->sysclk_source) != 0)
    {
        return -1;
    }

    /* ── Step 6: Update driver state ── */
    rcc_compute_clocks(p_config);

    /* ── Step 7: Enable Flash prefetch + instruction cache ── */
    FLASH->ACR |= (FLASH_ACR_PRFTEN | FLASH_ACR_ICEN);

    return 0;
}

/* ══════════════════════════════════════════════════════════
 *  PUBLIC API — Clock Queries (Barr Rule 4)
 * ══════════════════════════════════════════════════════════ */

/*!
 * @brief Get current SYSCLK frequency.
 * @return SYSCLK frequency in Hz.
 */
uint32_t rcc_get_sysclk(void)
{
    return g_clocks.sysclk_freq;
}

/*!
 * @brief Get current AHB bus clock (HCLK) frequency.
 * @return HCLK frequency in Hz.
 */
uint32_t rcc_get_hclk(void)
{
    return g_clocks.hclk_freq;
}

/*!
 * @brief Get current APB bus clock (PCLK) frequency.
 * @return PCLK frequency in Hz.
 */
uint32_t rcc_get_pclk(void)
{
    return g_clocks.pclk_freq;
}

/*!
 * @brief Get all current clock frequencies.
 * @param[out] p_clocks Pointer to receive clock frequencies.
 */
void rcc_get_clocks(rcc_clocks_t *p_clocks)
{
    if (NULL != p_clocks)
    {
        p_clocks->sysclk_freq = g_clocks.sysclk_freq;
        p_clocks->hclk_freq   = g_clocks.hclk_freq;
        p_clocks->pclk_freq   = g_clocks.pclk_freq;
    }
}

/* ══════════════════════════════════════════════════════════
 *  PUBLIC API — Peripheral Clock Enable Helpers
 *
 *  Each function:
 *  1. Sets the enable bit in the appropriate register
 *  2. Reads back the register (dummy read)
 *
 *  The dummy read is REQUIRED per STM32 errata:
 *  There is a 2-clock-cycle delay between enabling a
 *  peripheral clock and being able to access its registers.
 *  The read-back ensures the enable has propagated.
 * ══════════════════════════════════════════════════════════ */

void rcc_gpioa_clk_enable(void)
{
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    (void)RCC->IOPENR;  /* Dummy read — wait for clock to stabilize */
}

void rcc_gpiob_clk_enable(void)
{
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    (void)RCC->IOPENR;
}

void rcc_gpioc_clk_enable(void)
{
    RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
    (void)RCC->IOPENR;
}

void rcc_usart2_clk_enable(void)
{
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;
    (void)RCC->APBENR1;
}

void rcc_i2c1_clk_enable(void)
{
    RCC->APBENR1 |= RCC_APBENR1_I2C1EN;
    (void)RCC->APBENR1;
}

void rcc_spi1_clk_enable(void)
{
    RCC->APBENR2 |= RCC_APBENR2_SPI1EN;
    (void)RCC->APBENR2;
}

/*** end of file ***/