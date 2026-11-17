cmake_minimum_required(VERSION 3.16)
project(flash_test C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(HAL "${ROOT}/third_party/STM32U5xx_HAL_Driver")
set(CMSIS_D "${ROOT}/third_party/CMSIS_Device_U5")
set(CMSIS_C "${ROOT}/third_party/CMSIS_Core/CMSIS/Core")

set(MCU_FLAGS -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
set(LDSCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/STM32U5G9ZJTx_FLASH.ld")

add_executable(flash_test
  Src/main.c
  Src/hspi.c
  Src/flash_tests.c
  "${ROOT}/flash/Src/is25lp128f.c"
  "${CMSIS_D}/Source/Templates/system_stm32u5xx.c"
  "${HAL}/Src/stm32u5xx_hal.c"
  "${HAL}/Src/stm32u5xx_hal_cortex.c"
  "${HAL}/Src/stm32u5xx_hal_rcc.c"
  "${HAL}/Src/stm32u5xx_hal_rcc_ex.c"
  "${HAL}/Src/stm32u5xx_hal_gpio.c"
  "${HAL}/Src/stm32u5xx_hal_pwr.c"
  "${HAL}/Src/stm32u5xx_hal_pwr_ex.c"
  "${HAL}/Src/stm32u5xx_hal_dma.c"
  "${HAL}/Src/stm32u5xx_hal_dma_ex.c"
  "${HAL}/Src/stm32u5xx_hal_flash.c"
  "${HAL}/Src/stm32u5xx_hal_flash_ex.c"
  "${HAL}/Src/stm32u5xx_hal_xspi.c"
  "${CMSIS_D}/Source/Templates/gcc/startup_stm32u5g9xx.s"
)

set_target_properties(flash_test PROPERTIES
  OUTPUT_NAME flash_test
  SUFFIX .elf
)

target_include_directories(flash_test PRIVATE
  Inc
  "${ROOT}/flash/Inc"
  "${HAL}/Inc"
  "${HAL}/Inc/Legacy"
  "${CMSIS_D}/Include"
  "${CMSIS_C}/Include"
)

target_compile_definitions(flash_test PRIVATE
  STM32U5G9xx
  USE_HAL_DRIVER
)

target_compile_options(flash_test PRIVATE
  ${MCU_FLAGS}
  -Og
  -Wall
  -fdata-sections
  -ffunction-sections
)

target_link_options(flash_test PRIVATE
  ${MCU_FLAGS}
  "-T${LDSCRIPT}"
  -Wl,--gc-sections
  -specs=nano.specs
  -Wl,-Map=$<TARGET_FILE_DIR:flash_test>/flash_test.map
  -lc
  -lm
  -lnosys
)

add_custom_command(TARGET flash_test POST_BUILD
  COMMAND "${CMAKE_OBJCOPY}" -O binary
          $<TARGET_FILE:flash_test>
          $<TARGET_FILE_DIR:flash_test>/flash_test.bin
  COMMAND "${CMAKE_SIZE}" $<TARGET_FILE:flash_test>
  COMMENT "Generating flash_test.bin and printing size"
)
