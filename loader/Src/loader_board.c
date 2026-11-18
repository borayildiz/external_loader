/**
 * @file loader_board.c
 * @brief Clock, GPIO and HSPI1 bring-up for the external loader (milestone 4.3).
 *
 * SYSCLK = 24 MHz HSE, no PLL. HSPI1 kernel clock = SYSCLK, prescaler 0 → 24 MHz.
 * Pins: PH9 NCS, PH10–PH13 IO0–IO3, PI3 CLK.
 */
#include "loader_board.h"

static XSPI_HandleTypeDef hxspi1;

static int Loader_SystemClock_Config(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
    return -1;
  }

  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState       = RCC_HSE_ON;
  osc.PLL.PLLState   = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    return -1;
  }

  clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                       RCC_CLOCKTYPE_PCLK3;
  clk.SYSCLKSource   = RCC_SYSCLKSOURCE_HSE;
  clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV1;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  clk.APB3CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0) != HAL_OK) {
    return -1;
  }

  return 0;
}

static int Loader_HSPI1_Init(void)
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
  hxspi1.Init.ClockPrescaler          = 0; /* f_hspi = SYSCLK / 1 = 24 MHz */
  hxspi1.Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi1.Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_DISABLE;
  hxspi1.Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_NONE;
  hxspi1.Init.DelayBlockBypass        = HAL_XSPI_DELAY_BLOCK_BYPASS;
  hxspi1.Init.MaxTran                 = 0;
  hxspi1.Init.Refresh                 = 0;

  /* Clean slate if Init() is called again after bootloader/app use. */
  __HAL_RCC_HSPI1_FORCE_RESET();
  __HAL_RCC_HSPI1_RELEASE_RESET();

  if (HAL_XSPI_Init(&hxspi1) != HAL_OK) {
    return -1;
  }
  return 0;
}

int Loader_DisableCaches(void)
{
  DCACHE_HandleTypeDef hdcache = {0};

  /* Disable auto-starts ICACHE invalidation. */
  if (HAL_ICACHE_Disable() != HAL_OK) {
    return -1;
  }

  __HAL_RCC_DCACHE1_CLK_ENABLE();

  hdcache.Instance           = DCACHE1;
  hdcache.Init.ReadBurstType = DCACHE_READ_BURST_INCR;
  hdcache.State              = HAL_DCACHE_STATE_READY;

  /* Invalidate while still on (drops stale 0xA0… lines), then leave off. */
  if (HAL_DCACHE_IsEnabled(&hdcache) != 0U) {
    if (HAL_DCACHE_Invalidate(&hdcache) != HAL_OK) {
      return -1;
    }
  }
  if (HAL_DCACHE_Disable(&hdcache) != HAL_OK) {
    return -1;
  }

  return 0;
}

int Loader_BoardInit(void)
{
  if (Loader_SystemClock_Config() != 0) {
    return -1;
  }
  if (Loader_HSPI1_Init() != 0) {
    return -1;
  }
  return 0;
}

XSPI_HandleTypeDef *Loader_GetXspiHandle(void)
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
    return;
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
