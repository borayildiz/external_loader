/**
 * @file stm32u5xx_hal_conf.h
 * @brief Minimal HAL config for the IS25LP128F flash driver test app.
 */
#ifndef STM32U5XX_HAL_CONF_H
#define STM32U5XX_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_XSPI_MODULE_ENABLED

#define USE_HAL_XSPI_REGISTER_CALLBACKS 0U

#if !defined(HSE_VALUE)
#define HSE_VALUE 24000000UL
#endif
#if !defined(HSI_VALUE)
#define HSI_VALUE 16000000UL
#endif
#if !defined(MSI_VALUE)
#define MSI_VALUE 4000000UL
#endif
#if !defined(HSI48_VALUE)
#define HSI48_VALUE 48000000UL
#endif
#if !defined(LSI_VALUE)
#define LSI_VALUE 32000UL
#endif
#if !defined(LSE_VALUE)
#define LSE_VALUE 32768UL
#endif
#if !defined(EXTERNAL_SAI1_CLOCK_VALUE)
#define EXTERNAL_SAI1_CLOCK_VALUE 48000UL
#endif
#if !defined(EXTERNAL_SAI2_CLOCK_VALUE)
#define EXTERNAL_SAI2_CLOCK_VALUE 48000UL
#endif

#define VDD_VALUE                3300UL
#define TICK_INT_PRIORITY        15U
#define USE_RTOS                 0U
#define PREFETCH_ENABLE          1U

#define USE_SPI_CRC              0U

#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32u5xx_hal_rcc.h"
#endif
#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32u5xx_hal_gpio.h"
#endif
#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32u5xx_hal_dma.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32u5xx_hal_cortex.h"
#endif
#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32u5xx_hal_flash.h"
#endif
#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32u5xx_hal_pwr.h"
#endif
#ifdef HAL_XSPI_MODULE_ENABLED
#include "stm32u5xx_hal_xspi.h"
#endif

#define assert_param(expr) ((void)0U)

#ifdef __cplusplus
}
#endif

#endif /* STM32U5XX_HAL_CONF_H */
