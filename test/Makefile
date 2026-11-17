# Minimal GNU Make build for the flash driver test app.
# Requires arm-none-eabi-gcc on PATH.

TARGET   = flash_test
BUILD    = build

MCU      = -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard
DEFS     = -DSTM32U5G9xx -DUSE_HAL_DRIVER
LDSCRIPT = STM32U5G9ZJTx_FLASH.ld

ROOT     = ..
HAL      = $(ROOT)/third_party/STM32U5xx_HAL_Driver
CMSIS_D  = $(ROOT)/third_party/CMSIS_Device_U5
CMSIS_C  = $(ROOT)/third_party/CMSIS_Core/CMSIS/Core

INCLUDES = \
  -IInc \
  -I$(ROOT)/flash/Inc \
  -I$(HAL)/Inc \
  -I$(HAL)/Inc/Legacy \
  -I$(CMSIS_D)/Include \
  -I$(CMSIS_C)/Include

C_SOURCES = \
  Src/main.c \
  Src/hspi.c \
  Src/flash_tests.c \
  $(ROOT)/flash/Src/is25lp128f.c \
  $(CMSIS_D)/Source/Templates/system_stm32u5xx.c \
  $(HAL)/Src/stm32u5xx_hal.c \
  $(HAL)/Src/stm32u5xx_hal_cortex.c \
  $(HAL)/Src/stm32u5xx_hal_rcc.c \
  $(HAL)/Src/stm32u5xx_hal_rcc_ex.c \
  $(HAL)/Src/stm32u5xx_hal_gpio.c \
  $(HAL)/Src/stm32u5xx_hal_pwr.c \
  $(HAL)/Src/stm32u5xx_hal_pwr_ex.c \
  $(HAL)/Src/stm32u5xx_hal_dma.c \
  $(HAL)/Src/stm32u5xx_hal_dma_ex.c \
  $(HAL)/Src/stm32u5xx_hal_flash.c \
  $(HAL)/Src/stm32u5xx_hal_flash_ex.c \
  $(HAL)/Src/stm32u5xx_hal_xspi.c

ASM_SOURCES = $(CMSIS_D)/Source/Templates/gcc/startup_stm32u5g9xx.s

CFLAGS  = $(MCU) $(DEFS) $(INCLUDES) -Og -Wall -fdata-sections -ffunction-sections
ASFLAGS = $(MCU) $(DEFS) -Wall -fdata-sections -ffunction-sections
LDFLAGS = $(MCU) -T$(LDSCRIPT) -Wl,--gc-sections -specs=nano.specs -lc -lm -lnosys

CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc -x assembler-with-cpp
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

OBJECTS = $(addprefix $(BUILD)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD)/,$(notdir $(ASM_SOURCES:.s=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

.PHONY: all clean

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin
	$(SIZE) $(BUILD)/$(TARGET).elf

$(BUILD)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -Wl,-Map=$(BUILD)/$(TARGET).map -o $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/%.o: %.c Makefile | $(BUILD)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD)/%.o: %.s Makefile | $(BUILD)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
