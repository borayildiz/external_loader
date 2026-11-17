# test

On-target test application for the IS25LP128F flash driver. Runs from
internal flash, brings up HSPI1, then exercises erase / program / read /
memory-mapped mode.

| Path | Role |
|---|---|
| `Src/main.c` | Clock + HAL init, runs tests |
| `Src/hspi.c` | HSPI1 + pin mux (PH9–PH13, PI3) |
| `Src/flash_tests.c` | Driver test cases |
| `Inc/stm32u5xx_hal_conf.h` | HAL modules used by this app |
| `STM32U5G9ZJTx_FLASH.ld` | Internal-flash linker script |

Startup / system files come from `third_party/CMSIS_Device_U5`. HAL from
`third_party/STM32U5xx_HAL_Driver`.

After a run, inspect `g_flash_test_result` in the debugger (`0` = pass).

Coverage (cases, fail codes, API matrix, gaps):
[`doc/flash_test_coverage.md`](../doc/flash_test_coverage.md).

Build with Docker from the repo root (`make docker-build`) or locally
(`make` with `arm-none-eabi-gcc` on `PATH`).
