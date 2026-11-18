cmake_minimum_required(VERSION 3.16)
project(external_loader C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(HAL "${ROOT}/third_party/STM32U5xx_HAL_Driver")
set(CMSIS_D "${ROOT}/third_party/CMSIS_Device_U5")
set(CMSIS_C "${ROOT}/third_party/CMSIS_Core/CMSIS/Core")

set(MCU_FLAGS -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
set(LDSCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/STM32U5G9ZJTx_RAM.ld")
set(STLDR_NAME "IS25LP128F_STM32U5G9ZJ")

# No startup / crt0: CubeProgrammer calls Init() directly (ENTRY in linker script).
add_executable(loader
  Src/Dev_Inf.c
  Src/Loader_Src.c
  Src/loader_board.c
  Src/loader_timebase.c
  "${ROOT}/flash/Src/is25lp128f.c"
  "${CMSIS_D}/Source/Templates/system_stm32u5xx.c"
  "${HAL}/Src/stm32u5xx_hal.c"
  "${HAL}/Src/stm32u5xx_hal_cortex.c"
  "${HAL}/Src/stm32u5xx_hal_dcache.c"
  "${HAL}/Src/stm32u5xx_hal_icache.c"
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
)

set_target_properties(loader PROPERTIES
  OUTPUT_NAME "${STLDR_NAME}"
  SUFFIX .elf
)

target_include_directories(loader PRIVATE
  Inc
  "${ROOT}/flash/Inc"
  "${HAL}/Inc"
  "${HAL}/Inc/Legacy"
  "${CMSIS_D}/Include"
  "${CMSIS_C}/Include"
)

target_compile_definitions(loader PRIVATE
  STM32U5G9xx
  USE_HAL_DRIVER
)

target_compile_options(loader PRIVATE
  ${MCU_FLAGS}
  -Og
  -Wall
  -fdata-sections
  -ffunction-sections
)

target_link_options(loader PRIVATE
  ${MCU_FLAGS}
  -nostartfiles
  "-T${LDSCRIPT}"
  -Wl,--gc-sections
  # CubeProgrammer resolves these by symbol name; keep them despite --gc-sections.
  -Wl,--undefined=Init
  -Wl,--undefined=Write
  -Wl,--undefined=SectorErase
  -Wl,--undefined=MassErase
  -Wl,--undefined=Read
  -Wl,--undefined=Verify
  -Wl,--undefined=CheckSum
  -specs=nano.specs
  -Wl,-Map=$<TARGET_FILE_DIR:loader>/${STLDR_NAME}.map
  -lc
  -lm
  -lnosys
)

# CubeProgrammer consumes a renamed ELF.
add_custom_command(TARGET loader POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy
          $<TARGET_FILE:loader>
          $<TARGET_FILE_DIR:loader>/${STLDR_NAME}.stldr
  COMMAND "${CMAKE_SIZE}" $<TARGET_FILE:loader>
  COMMENT "Generating ${STLDR_NAME}.stldr"
)
