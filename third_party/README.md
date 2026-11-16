# third_party

STM32U5 HAL and CMSIS, used by the flash driver and the test app.

| Path | Upstream | Role |
|---|---|---|
| `STM32U5xx_HAL_Driver/` | [stm32u5xx-hal-driver](https://github.com/STMicroelectronics/stm32u5xx-hal-driver) | HAL, including XSPI for HSPI1 |
| `CMSIS_Device_U5/` | [cmsis_device_u5](https://github.com/STMicroelectronics/cmsis_device_u5) | Device headers (`stm32u5g9xx.h`) and startup |
| `CMSIS_Core/` | [cmsis_core](https://github.com/STMicroelectronics/cmsis_core) | Cortex-M CMSIS core |

These are git submodules. After clone:

```bash
git submodule update --init --recursive
```
