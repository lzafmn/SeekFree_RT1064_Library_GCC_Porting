/*
** ###################################################################
**     Processors:          MIMXRT1064CVJ5A
**                          MIMXRT1064CVL5A
**                          MIMXRT1064DVJ6A
**                          MIMXRT1064DVL6A
**
**     Compilers:           Freescale C/C++ for Embedded ARM
**                          GNU C Compiler
**                          IAR ANSI C/C++ Compiler for ARM
**                          Keil ARM C/C++ Compiler
**                          MCUXpresso Compiler
**
**     Reference manual:    IMXRT1064RM Rev.0.1, 12/2018 | IMXRT1064SRM Rev.3
**     Version:             rev. 1.2, 2019-04-29
**     Build:               b191113
**
**     Abstract:
**         Provides a system configuration function and a global variable that
**         contains the system frequency. It configures the device and initializes
**         the oscillator (PLL) that is part of the microcontroller device.
**
**     Copyright 2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2019 NXP
**     All rights reserved.
**
**     SPDX-License-Identifier: BSD-3-Clause
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
**     Revisions:
**     - rev. 0.1 (2018-06-22)
**         Initial version.
**     - rev. 1.0 (2018-11-16)
**         Update header files to align with IMXRT1064RM Rev.0.
**     - rev. 1.1 (2018-11-27)
**         Update header files to align with IMXRT1064RM Rev.0.1.
**     - rev. 1.2 (2019-04-29)
**         Add SET/CLR/TOG register group to register CTRL, STAT, CHANNELCTRL, CH0STAT, CH0OPTS, CH1STAT, CH1OPTS, CH2STAT, CH2OPTS, CH3STAT, CH3OPTS of DCP module.
**
** ###################################################################
*/

/*!
 * @file MIMXRT1064
 * @version 1.2
 * @date 2019-04-29
 * @brief Device specific configuration file for MIMXRT1064 (implementation file)
 *
 * Provides a system configuration function and a global variable that contains
 * the system frequency. It configures the device and initializes the oscillator
 * (PLL) that is part of the microcontroller device.
 */

#include <stdint.h>
#include "fsl_device_registers.h"



/* ----------------------------------------------------------------------------
   -- Core clock
   ---------------------------------------------------------------------------- */

uint32_t SystemCoreClock = DEFAULT_SYSTEM_CLOCK;

/* ----------------------------------------------------------------------------
   -- SystemInit()
   ---------------------------------------------------------------------------- */

#if(defined(__GNUC__))
__attribute__((section(".init_text")))
#endif
void SystemInit (void) {
#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
  SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));    /* set CP10, CP11 Full Access */
#endif /* ((__FPU_PRESENT == 1) && (__FPU_USED == 1)) */

    //拷贝中断向量表到SDRAM
#if (defined(__ICCARM__))
    extern uint32_t __VECTOR_RAM[];  //获取在RAM中的中断向量表起始地址
    extern uint32_t __VECTOR_TABLE[];//获取在FLASH中的中断向量表起始地址
    extern uint32_t __RAM_VECTOR_TABLE_SIZE[];

    uint32_t vector_size = ((uint32_t)__RAM_VECTOR_TABLE_SIZE)/4;//获取中断向量表大小
    
    while(vector_size--)
    {
        __VECTOR_RAM[vector_size] = __VECTOR_TABLE[vector_size];
    }
    
    SCB->VTOR = (uint32_t)__VECTOR_RAM;
	
#elif(defined(__CC_ARM) || defined(__ARMCC_VERSION))
	uint32_t * VECTOR_RAM ;  //获取在RAM中的中断向量表起始地址
    uint32_t * VECTOR_TABLE;//获取在FLASH中的中断向量表起始地址
    uint32_t RAM_VECTOR_TABLE_SIZE = 0x400;

	VECTOR_RAM = (uint32_t *)0x80000000;
	VECTOR_TABLE = (uint32_t *)0x70002000;
	
    uint32_t vector_size = ((uint32_t)RAM_VECTOR_TABLE_SIZE)/4;//获取中断向量表大小
    
    while(vector_size--)
    {
        VECTOR_RAM[vector_size] = VECTOR_TABLE[vector_size];
    }
    
    SCB->VTOR = (uint32_t)VECTOR_RAM;
#elif(defined(__GNUC__))
    /*
    * SeekFree RT1064 Library GCC Porting
    * Copyright 2021 lookas <i@18kas.com>
    * All rights reserved.
    * Licensed under MIT 
    */

    extern uint32_t __VECTOR_RAM[]; // 获取在 RAM 中的中断向量表起始地址
    extern uint32_t __VECTOR_TABLE[]; // 获取在 FLASH 中的中断向量表起始地址
    extern uint32_t __RAM_VECTOR_TABLE_SIZE_BYTES;
	
    uint32_t vector_size = ((uint32_t)&__RAM_VECTOR_TABLE_SIZE_BYTES) / 4; // 获取中断向量表大小
    
    while(vector_size--)
    {
        __VECTOR_RAM[vector_size] = __VECTOR_TABLE[vector_size];
    }
    
    SCB->VTOR = (uint32_t)__VECTOR_RAM;

    // copy .text
    extern uint32_t __TEXT_ROM, __text_start__, __text_end__;
    uint32_t *src = &__TEXT_ROM;
    uint32_t *dst = &__text_start__;
    while (dst < &__text_end__) *dst++ = *src++;

    // copy .data
    extern uint32_t __DATA_ROM, __data_start__, __data_end__;
    src = &__DATA_ROM;
    dst = &__data_start__;
    while (dst < &__data_end__) *dst++ = *src++;
#endif

/* Disable Watchdog Power Down Counter */
WDOG1->WMCR &= ~WDOG_WMCR_PDE_MASK;
WDOG2->WMCR &= ~WDOG_WMCR_PDE_MASK;

/* Watchdog disable */

#if (DISABLE_WDOG)
    if ((WDOG1->WCR & WDOG_WCR_WDE_MASK) != 0U)
    {
        WDOG1->WCR &= ~WDOG_WCR_WDE_MASK;
    }
    if ((WDOG2->WCR & WDOG_WCR_WDE_MASK) != 0U)
    {
        WDOG2->WCR &= ~WDOG_WCR_WDE_MASK;
    }
    if ((RTWDOG->CS & RTWDOG_CS_CMD32EN_MASK) != 0U)
    {
        RTWDOG->CNT = 0xD928C520U; /* 0xD928C520U is the update key */
    }
    else
    {
        RTWDOG->CNT = 0xC520U;
        RTWDOG->CNT = 0xD928U;
    }
    RTWDOG->TOVAL = 0xFFFF;
    RTWDOG->CS = (uint32_t) ((RTWDOG->CS) & ~RTWDOG_CS_EN_MASK) | RTWDOG_CS_UPDATE_MASK;
#endif /* (DISABLE_WDOG) */

    /* Disable Systick which might be enabled by bootrom */
    if ((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) != 0U)
    {
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    }

/* Enable instruction and data caches */
#if defined(__ICACHE_PRESENT) && __ICACHE_PRESENT
    if (SCB_CCR_IC_Msk != (SCB_CCR_IC_Msk & SCB->CCR)) {
        SCB_EnableICache();
    }
#endif
#if defined(__DCACHE_PRESENT) && __DCACHE_PRESENT
    if (SCB_CCR_DC_Msk != (SCB_CCR_DC_Msk & SCB->CCR)) {
        SCB_EnableDCache();
    }
#endif

  SystemInitHook();
}

/* ----------------------------------------------------------------------------
   -- SystemCoreClockUpdate()
   ---------------------------------------------------------------------------- */

void SystemCoreClockUpdate (void) {

    uint32_t freq;
    uint32_t PLL1MainClock;
    uint32_t PLL2MainClock;

    /* Periph_clk2_clk ---> Periph_clk */
    if ((CCM->CBCDR & CCM_CBCDR_PERIPH_CLK_SEL_MASK) != 0U)
    {
        switch (CCM->CBCMR & CCM_CBCMR_PERIPH_CLK2_SEL_MASK)
        {
            /* Pll3_sw_clk ---> Periph_clk2_clk ---> Periph_clk */
            case CCM_CBCMR_PERIPH_CLK2_SEL(0U):
                if((CCM_ANALOG->PLL_USB1 & CCM_ANALOG_PLL_USB1_BYPASS_MASK) != 0U)
                {
                    freq = (((CCM_ANALOG->PLL_USB1 & CCM_ANALOG_PLL_USB1_BYPASS_CLK_SRC_MASK) >> CCM_ANALOG_PLL_USB1_BYPASS_CLK_SRC_SHIFT) == 0U) ?
                           CPU_XTAL_CLK_HZ : CPU_CLK1_HZ;
                }
                else
                {
                    freq = (CPU_XTAL_CLK_HZ * (((CCM_ANALOG->PLL_USB1 & CCM_ANALOG_PLL_USB1_DIV_SELECT_MASK) != 0U) ? 22U : 20U));
                }
                break;

            /* Osc_clk ---> Periph_clk2_clk ---> Periph_clk */
            case CCM_CBCMR_PERIPH_CLK2_SEL(1U):
                freq = CPU_XTAL_CLK_HZ;
                break;

            case CCM_CBCMR_PERIPH_CLK2_SEL(2U):
                freq = (((CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_BYPASS_CLK_SRC_MASK) >> CCM_ANALOG_PLL_SYS_BYPASS_CLK_SRC_SHIFT) == 0U) ?
                   CPU_XTAL_CLK_HZ : CPU_CLK1_HZ;
                break;

            case CCM_CBCMR_PERIPH_CLK2_SEL(3U):
            default:
                freq = 0U;
                break;
        }

        freq /= (((CCM->CBCDR & CCM_CBCDR_PERIPH_CLK2_PODF_MASK) >> CCM_CBCDR_PERIPH_CLK2_PODF_SHIFT) + 1U);
    }
    /* Pre_Periph_clk ---> Periph_clk */
    else
    {
        /* check if pll is bypassed */
        if((CCM_ANALOG->PLL_ARM & CCM_ANALOG_PLL_ARM_BYPASS_MASK) != 0U)
        {
            PLL1MainClock = (((CCM_ANALOG->PLL_ARM & CCM_ANALOG_PLL_ARM_BYPASS_CLK_SRC_MASK) >> CCM_ANALOG_PLL_ARM_BYPASS_CLK_SRC_SHIFT) == 0U) ?
                   CPU_XTAL_CLK_HZ : CPU_CLK1_HZ;
        }
        else
        {
            PLL1MainClock = ((CPU_XTAL_CLK_HZ * ((CCM_ANALOG->PLL_ARM & CCM_ANALOG_PLL_ARM_DIV_SELECT_MASK) >>
                                             CCM_ANALOG_PLL_ARM_DIV_SELECT_SHIFT)) >> 1U);
        }

        /* check if pll is bypassed */
        if((CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_BYPASS_MASK) != 0U)
        {
            PLL2MainClock = (((CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_BYPASS_CLK_SRC_MASK) >> CCM_ANALOG_PLL_SYS_BYPASS_CLK_SRC_SHIFT) == 0U) ?
                   CPU_XTAL_CLK_HZ : CPU_CLK1_HZ;
        }
        else
        {
            PLL2MainClock = (CPU_XTAL_CLK_HZ * (((CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_DIV_SELECT_MASK) != 0U) ? 22U : 20U));
        }
        PLL2MainClock += (uint32_t)(((uint64_t)CPU_XTAL_CLK_HZ * ((uint64_t)(CCM_ANALOG->PLL_SYS_NUM))) / ((uint64_t)(CCM_ANALOG->PLL_SYS_DENOM)));


        switch (CCM->CBCMR & CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK)
        {
            /* PLL2 ---> Pre_Periph_clk ---> Periph_clk */
            case CCM_CBCMR_PRE_PERIPH_CLK_SEL(0U):
                freq = PLL2MainClock;
                break;

            /* PLL2 PFD2 ---> Pre_Periph_clk ---> Periph_clk */
            case CCM_CBCMR_PRE_PERIPH_CLK_SEL(1U):
                freq = PLL2MainClock / ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD2_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD2_FRAC_SHIFT) * 18U;
                break;

            /* PLL2 PFD0 ---> Pre_Periph_clk ---> Periph_clk */
            case CCM_CBCMR_PRE_PERIPH_CLK_SEL(2U):
                freq = PLL2MainClock / ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD0_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD0_FRAC_SHIFT) * 18U;
                break;

            /* PLL1 divided(/2) ---> Pre_Periph_clk ---> Periph_clk */
            case CCM_CBCMR_PRE_PERIPH_CLK_SEL(3U):
                freq = PLL1MainClock / (((CCM->CACRR & CCM_CACRR_ARM_PODF_MASK) >> CCM_CACRR_ARM_PODF_SHIFT) + 1U);
                break;

            default:
                freq = 0U;
                break;
        }
    }

    SystemCoreClock = (freq / (((CCM->CBCDR & CCM_CBCDR_AHB_PODF_MASK) >> CCM_CBCDR_AHB_PODF_SHIFT) + 1U));

}

/* ----------------------------------------------------------------------------
   -- SystemInitHook()
   ---------------------------------------------------------------------------- */

__attribute__ ((weak)) void SystemInitHook (void) {
  /* Void implementation of the weak function. */
}
