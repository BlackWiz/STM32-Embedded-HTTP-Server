/** @file rcc.h
 *
 * @brief RCC (Reset and Clock Control) driver for STM32G071RB.
 *
 * Provides:
 *   - Register map struct overlay (Barr Rule 1)
 *   - Bit-level definitions for all RCC registers
 *   - Clock configuration types and enumerations
 *   - Driver state tracking (Barr Rule 2)
 *   - API for clock initialization and queries (Barr Rules 3-4)
 *
 * Reference: RM0444 Rev 5, Sections 5.2 (RCC) and 3.3.4 (Flash)
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025. All rights reserved.
 */

#ifndef RCC_H
#define RCC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ══════════════════════════════════════════════════════════
 *  Oscillator Frequency Constants
 * ══════════════════════════════════════════════════════════ */
#define HSI16_VALUE    (16000000UL)
#define HSE_VALUE      (8000000UL)    /* Nucleo board: 8 MHz from ST-Link */
#define LSI_VALUE      (32000UL)
#define LSE_VALUE      (32768UL)

/* ══════════════════════════════════════════════════════════
 *  RCC Register Map (Barr Rule 1)
 *  Base Address: 0x40021000
 * ══════════════════════════════════════════════════════════ */
typedef struct
{
    volatile uint32_t CR;          /*!< Clock Control,                        Offset: 0x00 */
    volatile uint32_t ICSCR;       /*!< Internal Clock Calibration,           Offset: 0x04 */
    volatile uint32_t CFGR;        /*!< Clock Configuration,                  Offset: 0x08 */
    volatile uint32_t PLLCFGR;     /*!< PLL Configuration,                    Offset: 0x0C */
    volatile uint32_t RESERVED0;   /*!< Reserved,                             Offset: 0x10 */
    volatile uint32_t RESERVED1;   /*!< Reserved,                             Offset: 0x14 */
    volatile uint32_t CIER;        /*!< Clock Interrupt Enable,               Offset: 0x18 */
    volatile uint32_t CIFR;        /*!< Clock Interrupt Flag,                 Offset: 0x1C */
    volatile uint32_t CICR;        /*!< Clock Interrupt Clear,                Offset: 0x20 */
    volatile uint32_t IOPRSTR;     /*!< IO Port Reset,                        Offset: 0x24 */
    volatile uint32_t AHBRSTR;     /*!< AHB Peripheral Reset,                 Offset: 0x28 */
    volatile uint32_t APBRSTR1;    /*!< APB Peripheral Reset 1,               Offset: 0x2C */
    volatile uint32_t APBRSTR2;    /*!< APB Peripheral Reset 2,               Offset: 0x30 */
    volatile uint32_t IOPENR;      /*!< IO Port Clock Enable,                 Offset: 0x34 */
    volatile uint32_t AHBENR;      /*!< AHB Peripheral Clock Enable,          Offset: 0x38 */
    volatile uint32_t APBENR1;     /*!< APB Peripheral Clock Enable 1,        Offset: 0x3C */
    volatile uint32_t APBENR2;     /*!< APB Peripheral Clock Enable 2,        Offset: 0x40 */
    volatile uint32_t IOPSMENR;    /*!< IO Port Clock Enable (Sleep),         Offset: 0x44 */
    volatile uint32_t AHBSMENR;    /*!< AHB Clock Enable (Sleep),             Offset: 0x48 */
    volatile uint32_t APBSMENR1;   /*!< APB Clock Enable 1 (Sleep),           Offset: 0x4C */
    volatile uint32_t APBSMENR2;   /*!< APB Clock Enable 2 (Sleep),           Offset: 0x50 */
    volatile uint32_t CCIPR;       /*!< Peripheral Independent Clock Config,  Offset: 0x54 */
    volatile uint32_t RESERVED2;   /*!< Reserved,                             Offset: 0x58 */
    volatile uint32_t BDCR;        /*!< Backup Domain Control,                Offset: 0x5C */
    volatile uint32_t CSR;         /*!< Control/Status,                       Offset: 0x60 */
} RCC_TypeDef;

#define RCC_BASE    (0x40021000UL)
#define RCC         ((RCC_TypeDef *)RCC_BASE)

/* ══════════════════════════════════════════════════════════
 *  Flash Register Map (Required for wait state config)
 *  Base Address: 0x40022000
 * ══════════════════════════════════════════════════════════ */
typedef struct
{
    volatile uint32_t ACR;         /*!< Access Control,        Offset: 0x00 */
    volatile uint32_t RESERVED0;   /*!< Reserved,              Offset: 0x04 */
    volatile uint32_t KEYR;        /*!< Key Register,          Offset: 0x08 */
    volatile uint32_t OPTKEYR;     /*!< Option Key Register,   Offset: 0x0C */
    volatile uint32_t SR;          /*!< Status Register,       Offset: 0x10 */
    volatile uint32_t CR;          /*!< Control Register,      Offset: 0x14 */
    volatile uint32_t ECCR;        /*!< ECC Register,          Offset: 0x18 */
    volatile uint32_t RESERVED1;   /*!< Reserved,              Offset: 0x1C */
    volatile uint32_t OPTR;        /*!< Option Register,       Offset: 0x20 */
} FLASH_TypeDef;

#define FLASH_R_BASE   (0x40022000UL)
#define FLASH          ((FLASH_TypeDef *)FLASH_R_BASE)

/* Flash ACR bits */
#define FLASH_ACR_LATENCY_Pos      (0U)
#define FLASH_ACR_LATENCY_Msk      (0x7UL << FLASH_ACR_LATENCY_Pos)
#define FLASH_ACR_LATENCY          FLASH_ACR_LATENCY_Msk
#define FLASH_ACR_LATENCY_0WS     (0x00000000UL)
#define FLASH_ACR_LATENCY_1WS     (0x00000001UL)
#define FLASH_ACR_LATENCY_2WS     (0x00000002UL)
#define FLASH_ACR_PRFTEN_Pos       (8U)
#define FLASH_ACR_PRFTEN_Msk       (0x1UL << FLASH_ACR_PRFTEN_Pos)
#define FLASH_ACR_PRFTEN           FLASH_ACR_PRFTEN_Msk
#define FLASH_ACR_ICEN_Pos         (9U)
#define FLASH_ACR_ICEN_Msk         (0x1UL << FLASH_ACR_ICEN_Pos)
#define FLASH_ACR_ICEN             FLASH_ACR_ICEN_Msk

/* ══════════════════════════════════════════════════════════
 *  RCC_CR Bit Definitions
 * ══════════════════════════════════════════════════════════ */
#define RCC_CR_HSION_Pos           (8U)
#define RCC_CR_HSION_Msk           (0x1UL << RCC_CR_HSION_Pos)
#define RCC_CR_HSION               RCC_CR_HSION_Msk
#define RCC_CR_HSIKERON_Pos        (9U)
#define RCC_CR_HSIKERON_Msk        (0x1UL << RCC_CR_HSIKERON_Pos)
#define RCC_CR_HSIKERON            RCC_CR_HSIKERON_Msk
#define RCC_CR_HSIRDY_Pos          (10U)
#define RCC_CR_HSIRDY_Msk          (0x1UL << RCC_CR_HSIRDY_Pos)
#define RCC_CR_HSIRDY              RCC_CR_HSIRDY_Msk
#define RCC_CR_HSIDIV_Pos          (11U)
#define RCC_CR_HSIDIV_Msk          (0x7UL << RCC_CR_HSIDIV_Pos)
#define RCC_CR_HSIDIV              RCC_CR_HSIDIV_Msk
#define RCC_CR_HSEON_Pos           (16U)
#define RCC_CR_HSEON_Msk           (0x1UL << RCC_CR_HSEON_Pos)
#define RCC_CR_HSEON               RCC_CR_HSEON_Msk
#define RCC_CR_HSERDY_Pos          (17U)
#define RCC_CR_HSERDY_Msk          (0x1UL << RCC_CR_HSERDY_Pos)
#define RCC_CR_HSERDY              RCC_CR_HSERDY_Msk
#define RCC_CR_HSEBYP_Pos          (18U)
#define RCC_CR_HSEBYP_Msk          (0x1UL << RCC_CR_HSEBYP_Pos)
#define RCC_CR_HSEBYP              RCC_CR_HSEBYP_Msk
#define RCC_CR_CSSON_Pos           (19U)
#define RCC_CR_CSSON_Msk           (0x1UL << RCC_CR_CSSON_Pos)
#define RCC_CR_CSSON               RCC_CR_CSSON_Msk
#define RCC_CR_PLLON_Pos           (24U)
#define RCC_CR_PLLON_Msk           (0x1UL << RCC_CR_PLLON_Pos)
#define RCC_CR_PLLON               RCC_CR_PLLON_Msk
#define RCC_CR_PLLRDY_Pos          (25U)
#define RCC_CR_PLLRDY_Msk          (0x1UL << RCC_CR_PLLRDY_Pos)
#define RCC_CR_PLLRDY              RCC_CR_PLLRDY_Msk

/* ══════════════════════════════════════════════════════════
 *  RCC_CFGR Bit Definitions
 * ══════════════════════════════════════════════════════════ */

/* SW: System clock switch — bits [2:0] */
#define RCC_CFGR_SW_Pos            (0U)
#define RCC_CFGR_SW_Msk            (0x7UL << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SW                RCC_CFGR_SW_Msk
#define RCC_CFGR_SW_HSISYS        (0x00000000UL)
#define RCC_CFGR_SW_HSE           (0x00000001UL)
#define RCC_CFGR_SW_PLLRCLK       (0x00000002UL)
#define RCC_CFGR_SW_LSI           (0x00000003UL)
#define RCC_CFGR_SW_LSE           (0x00000004UL)

/* SWS: System clock switch status — bits [5:3] */
#define RCC_CFGR_SWS_Pos           (3U)
#define RCC_CFGR_SWS_Msk           (0x7UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_SWS               RCC_CFGR_SWS_Msk
#define RCC_CFGR_SWS_HSISYS        (0x00000000UL)
#define RCC_CFGR_SWS_HSE           (0x00000008UL)
#define RCC_CFGR_SWS_PLLRCLK       (0x00000010UL)
#define RCC_CFGR_SWS_LSI           (0x00000018UL)
#define RCC_CFGR_SWS_LSE           (0x00000020UL)   /* ← FIXED from 0x100 */

/* HPRE: AHB prescaler — bits [11:8] */
#define RCC_CFGR_HPRE_Pos          (8U)
#define RCC_CFGR_HPRE_Msk          (0xFUL << RCC_CFGR_HPRE_Pos)
#define RCC_CFGR_HPRE              RCC_CFGR_HPRE_Msk

/* PPRE: APB prescaler — bits [14:12] */
#define RCC_CFGR_PPRE_Pos          (12U)
#define RCC_CFGR_PPRE_Msk          (0x7UL << RCC_CFGR_PPRE_Pos)
#define RCC_CFGR_PPRE              RCC_CFGR_PPRE_Msk

/* ══════════════════════════════════════════════════════════
 *  RCC_PLLCFGR Bit Definitions
 * ══════════════════════════════════════════════════════════ */
#define RCC_PLLCFGR_PLLSRC_Pos     (0U)
#define RCC_PLLCFGR_PLLSRC_Msk     (0x3UL << RCC_PLLCFGR_PLLSRC_Pos)
#define RCC_PLLCFGR_PLLSRC         RCC_PLLCFGR_PLLSRC_Msk
#define RCC_PLLCFGR_PLLSRC_NONE    (0x00000000UL)
#define RCC_PLLCFGR_PLLSRC_HSI     (0x00000002UL)
#define RCC_PLLCFGR_PLLSRC_HSE     (0x00000003UL)

#define RCC_PLLCFGR_PLLM_Pos       (4U)
#define RCC_PLLCFGR_PLLM_Msk       (0x7UL << RCC_PLLCFGR_PLLM_Pos)
#define RCC_PLLCFGR_PLLM           RCC_PLLCFGR_PLLM_Msk

#define RCC_PLLCFGR_PLLN_Pos       (8U)
#define RCC_PLLCFGR_PLLN_Msk       (0x7FUL << RCC_PLLCFGR_PLLN_Pos)
#define RCC_PLLCFGR_PLLN           RCC_PLLCFGR_PLLN_Msk

#define RCC_PLLCFGR_PLLPEN_Pos     (16U)
#define RCC_PLLCFGR_PLLPEN_Msk     (0x1UL << RCC_PLLCFGR_PLLPEN_Pos)
#define RCC_PLLCFGR_PLLPEN         RCC_PLLCFGR_PLLPEN_Msk

#define RCC_PLLCFGR_PLLP_Pos       (17U)
#define RCC_PLLCFGR_PLLP_Msk       (0x1FUL << RCC_PLLCFGR_PLLP_Pos)
#define RCC_PLLCFGR_PLLP           RCC_PLLCFGR_PLLP_Msk

#define RCC_PLLCFGR_PLLQEN_Pos     (24U)
#define RCC_PLLCFGR_PLLQEN_Msk     (0x1UL << RCC_PLLCFGR_PLLQEN_Pos)
#define RCC_PLLCFGR_PLLQEN         RCC_PLLCFGR_PLLQEN_Msk

#define RCC_PLLCFGR_PLLQ_Pos       (25U)
#define RCC_PLLCFGR_PLLQ_Msk       (0x7UL << RCC_PLLCFGR_PLLQ_Pos)
#define RCC_PLLCFGR_PLLQ           RCC_PLLCFGR_PLLQ_Msk

#define RCC_PLLCFGR_PLLREN_Pos     (28U)
#define RCC_PLLCFGR_PLLREN_Msk     (0x1UL << RCC_PLLCFGR_PLLREN_Pos)
#define RCC_PLLCFGR_PLLREN         RCC_PLLCFGR_PLLREN_Msk

#define RCC_PLLCFGR_PLLR_Pos       (29U)
#define RCC_PLLCFGR_PLLR_Msk       (0x7UL << RCC_PLLCFGR_PLLR_Pos)
#define RCC_PLLCFGR_PLLR           RCC_PLLCFGR_PLLR_Msk

/* ══════════════════════════════════════════════════════════
 *  RCC_IOPENR — GPIO Port Clock Enable
 * ══════════════════════════════════════════════════════════ */
#define RCC_IOPENR_GPIOAEN_Pos     (0U)
#define RCC_IOPENR_GPIOAEN_Msk     (0x1UL << RCC_IOPENR_GPIOAEN_Pos)
#define RCC_IOPENR_GPIOAEN         RCC_IOPENR_GPIOAEN_Msk
#define RCC_IOPENR_GPIOBEN_Pos     (1U)
#define RCC_IOPENR_GPIOBEN_Msk     (0x1UL << RCC_IOPENR_GPIOBEN_Pos)
#define RCC_IOPENR_GPIOBEN         RCC_IOPENR_GPIOBEN_Msk
#define RCC_IOPENR_GPIOCEN_Pos     (2U)
#define RCC_IOPENR_GPIOCEN_Msk     (0x1UL << RCC_IOPENR_GPIOCEN_Pos)
#define RCC_IOPENR_GPIOCEN         RCC_IOPENR_GPIOCEN_Msk
#define RCC_IOPENR_GPIODEN_Pos     (3U)
#define RCC_IOPENR_GPIODEN_Msk     (0x1UL << RCC_IOPENR_GPIODEN_Pos)
#define RCC_IOPENR_GPIODEN         RCC_IOPENR_GPIODEN_Msk
#define RCC_IOPENR_GPIOFEN_Pos     (5U)
#define RCC_IOPENR_GPIOFEN_Msk     (0x1UL << RCC_IOPENR_GPIOFEN_Pos)
#define RCC_IOPENR_GPIOFEN         RCC_IOPENR_GPIOFEN_Msk

/* ══════════════════════════════════════════════════════════
 *  RCC_APBENR1 — APB Peripheral Clock Enable 1
 *  (Most commonly used peripherals)
 * ══════════════════════════════════════════════════════════ */
#define RCC_APBENR1_TIM2EN_Pos     (0U)
#define RCC_APBENR1_TIM2EN_Msk     (0x1UL << RCC_APBENR1_TIM2EN_Pos)
#define RCC_APBENR1_TIM2EN         RCC_APBENR1_TIM2EN_Msk
#define RCC_APBENR1_TIM3EN_Pos     (1U)
#define RCC_APBENR1_TIM3EN_Msk     (0x1UL << RCC_APBENR1_TIM3EN_Pos)
#define RCC_APBENR1_TIM3EN         RCC_APBENR1_TIM3EN_Msk
#define RCC_APBENR1_SPI2EN_Pos     (14U)
#define RCC_APBENR1_SPI2EN_Msk     (0x1UL << RCC_APBENR1_SPI2EN_Pos)
#define RCC_APBENR1_SPI2EN         RCC_APBENR1_SPI2EN_Msk
#define RCC_APBENR1_USART2EN_Pos   (17U)
#define RCC_APBENR1_USART2EN_Msk   (0x1UL << RCC_APBENR1_USART2EN_Pos)
#define RCC_APBENR1_USART2EN       RCC_APBENR1_USART2EN_Msk
#define RCC_APBENR1_I2C1EN_Pos     (21U)
#define RCC_APBENR1_I2C1EN_Msk     (0x1UL << RCC_APBENR1_I2C1EN_Pos)
#define RCC_APBENR1_I2C1EN         RCC_APBENR1_I2C1EN_Msk
#define RCC_APBENR1_I2C2EN_Pos     (22U)
#define RCC_APBENR1_I2C2EN_Msk     (0x1UL << RCC_APBENR1_I2C2EN_Pos)
#define RCC_APBENR1_I2C2EN         RCC_APBENR1_I2C2EN_Msk
#define RCC_APBENR1_PWREN_Pos      (28U)
#define RCC_APBENR1_PWREN_Msk      (0x1UL << RCC_APBENR1_PWREN_Pos)
#define RCC_APBENR1_PWREN          RCC_APBENR1_PWREN_Msk

/* ══════════════════════════════════════════════════════════
 *  RCC_APBENR2 — APB Peripheral Clock Enable 2
 * ══════════════════════════════════════════════════════════ */
#define RCC_APBENR2_SYSCFGEN_Pos   (0U)
#define RCC_APBENR2_SYSCFGEN_Msk   (0x1UL << RCC_APBENR2_SYSCFGEN_Pos)
#define RCC_APBENR2_SYSCFGEN       RCC_APBENR2_SYSCFGEN_Msk
#define RCC_APBENR2_TIM1EN_Pos     (11U)
#define RCC_APBENR2_TIM1EN_Msk     (0x1UL << RCC_APBENR2_TIM1EN_Pos)
#define RCC_APBENR2_TIM1EN         RCC_APBENR2_TIM1EN_Msk
#define RCC_APBENR2_SPI1EN_Pos     (12U)
#define RCC_APBENR2_SPI1EN_Msk     (0x1UL << RCC_APBENR2_SPI1EN_Pos)
#define RCC_APBENR2_SPI1EN         RCC_APBENR2_SPI1EN_Msk
#define RCC_APBENR2_USART1EN_Pos   (14U)
#define RCC_APBENR2_USART1EN_Msk   (0x1UL << RCC_APBENR2_USART1EN_Pos)
#define RCC_APBENR2_USART1EN       RCC_APBENR2_USART1EN_Msk
#define RCC_APBENR2_ADCEN_Pos      (20U)
#define RCC_APBENR2_ADCEN_Msk      (0x1UL << RCC_APBENR2_ADCEN_Pos)
#define RCC_APBENR2_ADCEN          RCC_APBENR2_ADCEN_Msk

/* ══════════════════════════════════════════════════════════
 *  RCC_AHBENR — AHB Peripheral Clock Enable
 * ══════════════════════════════════════════════════════════ */
#define RCC_AHBENR_DMA1EN_Pos      (0U)
#define RCC_AHBENR_DMA1EN_Msk      (0x1UL << RCC_AHBENR_DMA1EN_Pos)
#define RCC_AHBENR_DMA1EN          RCC_AHBENR_DMA1EN_Msk
#define RCC_AHBENR_FLASHEN_Pos     (8U)
#define RCC_AHBENR_FLASHEN_Msk     (0x1UL << RCC_AHBENR_FLASHEN_Pos)
#define RCC_AHBENR_FLASHEN         RCC_AHBENR_FLASHEN_Msk
#define RCC_AHBENR_CRCEN_Pos       (12U)
#define RCC_AHBENR_CRCEN_Msk       (0x1UL << RCC_AHBENR_CRCEN_Pos)
#define RCC_AHBENR_CRCEN           RCC_AHBENR_CRCEN_Msk

/* ══════════════════════════════════════════════════════════
 *  Clock Configuration Types (Barr Rule 2)
 * ══════════════════════════════════════════════════════════ */

/* System clock source selection */
typedef enum {
    RCC_SYSCLK_SRC_HSI16 = 0,
    RCC_SYSCLK_SRC_HSE,
    RCC_SYSCLK_SRC_PLL,
    RCC_SYSCLK_SRC_LSI,
    RCC_SYSCLK_SRC_LSE
} rcc_sysclk_src_t;

/* PLL input source */
typedef enum {
    RCC_PLL_SRC_NONE = 0,
    RCC_PLL_SRC_HSI16,
    RCC_PLL_SRC_HSE
} rcc_pll_src_t;

/* PLL configuration */
typedef struct {
    rcc_pll_src_t source;    /*!< PLL input clock source           */
    uint32_t      m;         /*!< Input divider   (1-8)            */
    uint32_t      n;         /*!< VCO multiplier  (8-86)           */
    uint32_t      r;         /*!< Output divider  (2,4,6,8)        */
} rcc_pll_config_t;

/* AHB prescaler */
typedef enum {
    RCC_AHB_DIV_1   = 0x0,
    RCC_AHB_DIV_2   = 0x8,
    RCC_AHB_DIV_4   = 0x9,
    RCC_AHB_DIV_8   = 0xA,
    RCC_AHB_DIV_16  = 0xB,
    RCC_AHB_DIV_64  = 0xC,
    RCC_AHB_DIV_128 = 0xD,
    RCC_AHB_DIV_256 = 0xE,
    RCC_AHB_DIV_512 = 0xF
} rcc_ahb_prescaler_t;

/* APB prescaler */
typedef enum {
    RCC_APB_DIV_1  = 0x0,
    RCC_APB_DIV_2  = 0x4,
    RCC_APB_DIV_4  = 0x5,
    RCC_APB_DIV_8  = 0x6,
    RCC_APB_DIV_16 = 0x7
} rcc_apb_prescaler_t;

/* Full system clock configuration */
typedef struct {
    rcc_sysclk_src_t    sysclk_source;   /*!< SYSCLK source selection      */
    rcc_pll_config_t    pll_config;      /*!< PLL params (if source = PLL) */
    rcc_ahb_prescaler_t ahb_prescaler;   /*!< AHB bus prescaler            */
    rcc_apb_prescaler_t apb_prescaler;   /*!< APB bus prescaler            */
} rcc_config_t;

/* Current clock frequencies — updated by rcc_init() */
typedef struct {
    uint32_t sysclk_freq;   /*!< SYSCLK frequency in Hz */
    uint32_t hclk_freq;     /*!< AHB bus clock in Hz    */
    uint32_t pclk_freq;     /*!< APB bus clock in Hz    */
} rcc_clocks_t;

/* ══════════════════════════════════════════════════════════
 *  Timeout for clock stabilization
 * ══════════════════════════════════════════════════════════ */
#define RCC_TIMEOUT_VALUE  (5000U)   /* Iterations to wait for ready flag */

/* ══════════════════════════════════════════════════════════
 *  API Function Prototypes (Barr Rules 3-4)
 * ══════════════════════════════════════════════════════════ */

/*!
 * @brief Initialize system clocks per configuration.
 * @param[in] p_config Pointer to clock configuration.
 * @return 0 on success, -1 on timeout/error.
 */
int32_t rcc_init(const rcc_config_t *p_config);

/*!
 * @brief Get current SYSCLK frequency.
 * @return SYSCLK frequency in Hz.
 */
uint32_t rcc_get_sysclk(void);

/*!
 * @brief Get current AHB bus clock (HCLK) frequency.
 * @return HCLK frequency in Hz.
 */
uint32_t rcc_get_hclk(void);

/*!
 * @brief Get current APB bus clock (PCLK) frequency.
 * @return PCLK frequency in Hz.
 */
uint32_t rcc_get_pclk(void);

/*!
 * @brief Get all current clock frequencies.
 * @param[out] p_clocks Pointer to receive clock frequencies.
 */
void rcc_get_clocks(rcc_clocks_t *p_clocks);

/* Peripheral clock enable helpers */
void rcc_gpioa_clk_enable(void);
void rcc_gpiob_clk_enable(void);
void rcc_gpioc_clk_enable(void);
void rcc_usart2_clk_enable(void);
void rcc_i2c1_clk_enable(void);
void rcc_spi1_clk_enable(void);

#ifdef __cplusplus
}
#endif

#endif /* RCC_H */

/*** end of file ***/