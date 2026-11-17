/**
 * @file hspi.c
 * @brief HSPI1 bring-up for IS25LP128F on STM32U5G9ZJ (test app only).
 *
 * Pins: PH9 NCS, PH10–PH13 IO0–IO3, PI3 CLK.
 */
#include "hspi.h"
#include "main.h"

static XSPI_HandleTypeDef hxspi1;

int HSPI1_Init(void)
{
  hxspi1.Instance = HSPI1;

  hxspi1.Init.FifoThresholdByte       = 4;
  hxspi1.Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  hxspi1.Init.MemoryType              = HAL_XSPI_MEMTYPE_MICRON;
  hxspi1.Init.MemorySize              = HAL_XSPI_SIZE_128MB;
  hxspi1.Init.ChipSelectHighTimeCycle = 2;
  hxspi1.Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi1.Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  hxspi1.Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hxspi1.Init.ClockPrescaler          = 0; /* f_hspi = f_kernel / (PRESCALER+1) → 24 MHz */
  hxspi1.Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi1.Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_DISABLE;
  hxspi1.Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_NONE;
  hxspi1.Init.DelayBlockBypass        = HAL_XSPI_DELAY_BLOCK_BYPASS;
  hxspi1.Init.MaxTran                 = 0;
  hxspi1.Init.Refresh                 = 0;

  if (HAL_XSPI_Init(&hxspi1) != HAL_OK) {
    return -1;
  }
  return 0;
}

XSPI_HandleTypeDef *HSPI1_GetHandle(void)
{
  return &hxspi1;
}

void HAL_XSPI_MspInit(XSPI_HandleTypeDef *hxspi)
{
  GPIO_InitTypeDef gpio = {0};
  RCC_PeriphCLKInitTypeDef periph = {0};

  if (hxspi->Instance != HSPI1) {
    return;
  }

  periph.PeriphClockSelection = RCC_PERIPHCLK_HSPI;
  periph.HspiClockSelection   = RCC_HSPICLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&periph) != HAL_OK) {
    Error_Handler();
  }

  __HAL_RCC_HSPI1_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  /* PH9 NCS, PH10 IO0, PH11 IO1, PH12 IO2, PH13 IO3 */
  gpio.Pin       = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
  gpio.Mode      = GPIO_MODE_AF_PP;
  gpio.Pull      = GPIO_NOPULL;
  gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF8_HSPI1;
  HAL_GPIO_Init(GPIOH, &gpio);

  /* PI3 CLK */
  gpio.Pin       = GPIO_PIN_3;
  gpio.Alternate = GPIO_AF8_HSPI1;
  HAL_GPIO_Init(GPIOI, &gpio);
}

void HAL_XSPI_MspDeInit(XSPI_HandleTypeDef *hxspi)
{
  if (hxspi->Instance != HSPI1) {
    return;
  }

  __HAL_RCC_HSPI1_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOH, GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
  HAL_GPIO_DeInit(GPIOI, GPIO_PIN_3);
}
