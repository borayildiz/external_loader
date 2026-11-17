/**
 * @file loader_timebase.c
 * @brief Polled HAL time base for the external loader (milestone 4.4).
 *
 * CubeProgrammer runs the loader with no usable vector table, so SysTick
 * interrupts must not be relied on. SysTick still runs as a free-running
 * down-counter; HAL_GetTick() samples it and advances uwTick in software.
 *
 * HAL_RCC_ClockConfig() re-calls HAL_InitTick() after SYSCLK changes so
 * cycles-per-ms tracks the 24 MHz HSE setup from loader_board.c.
 */
#include "stm32u5xx_hal.h"

static uint32_t s_last_val;
static uint32_t s_cycle_acc;
static uint32_t s_cycles_per_ms;

/**
 * @brief Start SysTick as a free-running HCLK counter with no interrupt.
 */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  if ((uint32_t)uwTickFreq == 0u) {
    return HAL_ERROR;
  }
  if (SystemCoreClock == 0u) {
    return HAL_ERROR;
  }

  s_cycles_per_ms = SystemCoreClock / (1000u / (uint32_t)uwTickFreq);
  if (s_cycles_per_ms == 0u) {
    return HAL_ERROR;
  }

  /* 24-bit free-run, processor clock, counter enabled, TICKINT clear. */
  SysTick->LOAD = 0x00FFFFFFu;
  SysTick->VAL  = 0u;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

  s_last_val  = SysTick->VAL;
  s_cycle_acc = 0u;
  uwTickPrio  = TickPriority;

  return HAL_OK;
}

static void Loader_PollTick(void)
{
  uint32_t val = SysTick->VAL;
  uint32_t load = SysTick->LOAD + 1u;
  uint32_t delta;

  if (s_cycles_per_ms == 0u) {
    return;
  }

  /* SysTick counts down; handle wrap across LOAD. */
  if (s_last_val >= val) {
    delta = s_last_val - val;
  } else {
    delta = s_last_val + load - val;
  }
  s_last_val = val;

  s_cycle_acc += delta;
  while (s_cycle_acc >= s_cycles_per_ms) {
    s_cycle_acc -= s_cycles_per_ms;
    uwTick += (uint32_t)uwTickFreq;
  }
}

uint32_t HAL_GetTick(void)
{
  Loader_PollTick();
  return uwTick;
}

void HAL_SuspendTick(void)
{
  /* Pause the counter; never touch TICKINT (must stay disabled). */
  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void HAL_ResumeTick(void)
{
  s_last_val = SysTick->VAL;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}
