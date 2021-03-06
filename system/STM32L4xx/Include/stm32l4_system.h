/*
 * Copyright (c) 2016 Thomas Roell.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimers.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimers in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Thomas Roell, nor the names of its contributors
 *     may be used to endorse or promote products derived from this Software
 *     without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 */

#if !defined(_STM32L4_SYSTEM_H)
#define _STM32L4_SYSTEM_H

#include "armv7m.h"

#include "stm32l4xx.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SYSTEM_PERIPH_FLASH = 0,
    SYSTEM_PERIPH_SRAM1,
    SYSTEM_PERIPH_SRAM2,
    SYSTEM_PERIPH_DMA1,
    SYSTEM_PERIPH_DMA2,
    SYSTEM_PERIPH_GPIOA,
    SYSTEM_PERIPH_GPIOB,
#if defined(STM32L433xx) || defined(STM32L476xx)
    SYSTEM_PERIPH_GPIOC,
    SYSTEM_PERIPH_GPIOD,
    SYSTEM_PERIPH_GPIOE,
#endif
#if defined(STM32L476xx)
    SYSTEM_PERIPH_GPIOF,
    SYSTEM_PERIPH_GPIOG,
#endif
    SYSTEM_PERIPH_GPIOH,
    SYSTEM_PERIPH_USB,
    SYSTEM_PERIPH_ADC,
    SYSTEM_PERIPH_QSPI,
    SYSTEM_PERIPH_USART1,
    SYSTEM_PERIPH_USART2,
#if defined(STM32L433xx) || defined(STM32L476xx)
    SYSTEM_PERIPH_USART3,
#endif
#if defined(STM32L476xx)
    SYSTEM_PERIPH_UART4,
    SYSTEM_PERIPH_UART5,
#endif
    SYSTEM_PERIPH_LPUART1,
    SYSTEM_PERIPH_I2C1,
#if defined(STM32L433xx) || defined(STM32L476xx)
    SYSTEM_PERIPH_I2C2,
#endif
    SYSTEM_PERIPH_I2C3,
    SYSTEM_PERIPH_SPI1,
#if defined(STM32L433xx) || defined(STM32L476xx)
    SYSTEM_PERIPH_SPI2,
#endif
    SYSTEM_PERIPH_SPI3,
#if defined(STM32L433xx) || defined(STM32L476xx)
    SYSTEM_PERIPH_SDIO,
#endif
    SYSTEM_PERIPH_SAI1,
#if defined(STM32L476xx)
    SYSTEM_PERIPH_SAI2,
    SYSTEM_PERIPH_DFSDM,
#endif
    SYSTEM_PERIPH_CAN,
    SYSTEM_PERIPH_TIM1,
    SYSTEM_PERIPH_TIM2,
#if defined(STM32L476xx)
    SYSTEM_PERIPH_TIM3,
    SYSTEM_PERIPH_TIM4,
    SYSTEM_PERIPH_TIM5,
#endif
    SYSTEM_PERIPH_TIM6,
    SYSTEM_PERIPH_TIM7,
#if defined(STM32L476xx)
    SYSTEM_PERIPH_TIM8,
#endif
    SYSTEM_PERIPH_TIM15,
    SYSTEM_PERIPH_TIM16,
#if defined(STM32L476xx)
    SYSTEM_PERIPH_TIM17,
#endif
    SYSTEM_PERIPH_LPTIM1,
    SYSTEM_PERIPH_LPTIM2,
    SYSTEM_PERIPH_DAC,
    SYSTEM_PERIPH_COUNT
};

#define SYSTEM_NOTIFY_COUNT 16
#define SYSTEM_EVENT_COUNT  5

#define SYSTEM_LOCK_SLEEP            0
#define SYSTEM_LOCK_STOP_0           1
#define SYSTEM_LOCK_STOP_1           2
#define SYSTEM_LOCK_STOP_2           3
#define SYSTEM_LOCK_STANDBY          4
#define SYSTEM_LOCK_COUNT            5

#define SYSTEM_EVENT_SUSPEND        0x00000001
#define SYSTEM_EVENT_RESUME         0x00000002
#define SYSTEM_EVENT_STANDBY        0x00000004
#define SYSTEM_EVENT_SHUTDOWN       0x00000008
#define SYSTEM_EVENT_RESET          0x00000010

#define SYSTEM_INDEX_SUSPEND        0
#define SYSTEM_INDEX_RESUME         1
#define SYSTEM_INDEX_STANDBY        2
#define SYSTEM_INDEX_SHUTDOWN       3
#define SYSTEM_INDEX_RESET          4

typedef void (*stm32l4_system_callback_t)(void *context, uint32_t events);

extern void     stm32l4_system_periph_reset(unsigned int periph);
extern void     stm32l4_system_periph_enable(unsigned int periph);
extern void     stm32l4_system_periph_disable(unsigned int periph);
extern void     stm32l4_system_periph_wake(unsigned int periph);
extern void     stm32l4_system_periph_sleep(unsigned int periph);
extern void     stm32l4_system_periph_cond_enable(unsigned int periph, volatile uint32_t *p_mask, uint32_t mask);
extern void     stm32l4_system_periph_cond_disable(unsigned int periph, volatile uint32_t *p_mask, uint32_t mask);
extern void     stm32l4_system_periph_cond_wake(unsigned int periph, volatile uint32_t *p_mask, uint32_t mask);
extern void     stm32l4_system_periph_cond_sleep(unsigned int periph, volatile uint32_t *p_mask, uint32_t mask);
extern bool     stm32l4_system_configure(uint32_t lseclk, uint32_t hseclk, uint32_t hclk, uint32_t pclk1, uint32_t pclk2);
extern bool     stm32l4_system_clk48_enable(void);
extern bool     stm32l4_system_clk48_disable(void);
extern void     stm32l4_system_hsi16_enable(void);
extern void     stm32l4_system_hsi16_disable(void);
extern uint32_t stm32l4_system_lseclk(void);
extern uint32_t stm32l4_system_hseclk(void);
extern uint32_t stm32l4_system_sysclk(void);
extern uint32_t stm32l4_system_hclk(void);
extern uint32_t stm32l4_system_pclk1(void);
extern uint32_t stm32l4_system_pclk2(void);
extern void     stm32l4_system_notify(uint32_t slot, stm32l4_system_callback_t callback, void *context, uint32_t events); 
extern void     stm32l4_system_lock(uint32_t lock); 
extern void     stm32l4_system_unlock(uint32_t lock);
extern bool     stm32l4_system_stop(void);
extern bool     stm32l4_system_standby(void);
extern bool     stm32l4_system_shutdown(void);
extern void     stm32l4_system_reset(void);
extern void     stm32l4_system_bootloader(void);

#ifdef __cplusplus
}
#endif

#endif /* _STM32L4_SYSTEM_H */
