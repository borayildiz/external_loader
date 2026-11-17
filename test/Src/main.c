/**
 * @file main.c
 * @brief Flash driver test application (runs from internal flash).
 */
#include "main.h"
#include "hspi.h"
#include "is25lp128f.h"

static void SystemClock_Config(void);
int FlashTests_RunAll(void);

/** Non-zero while tests are running; 0 on success, negative on failure. */
volatile int g_flash_test_result = 1;

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  if (HSPI1_Init() != 0) {
    Error_Handler();
  }

  if (IS25_Init(HSPI1_GetHandle()) != IS25_OK) {
    Error_Handler();
  }

  g_flash_test_result = FlashTests_RunAll();

  for (;;) {
    /* Inspect g_flash_test_result in the debugger: 0 = pass. */
  }
}

/**
 * SYSCLK = 24 MHz from HSE, no PLL (same as the planned loader clock).
 */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
    Error_Handler();
  }

  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState       = RCC_HSE_ON;
  osc.PLL.PLLState   = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    Error_Handler();
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
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  for (;;) {
  }
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}
