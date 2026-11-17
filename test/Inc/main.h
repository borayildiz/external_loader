#ifndef MAIN_H
#define MAIN_H

#include "stm32u5xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXT_FLASH_BASE  0xA0000000u

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */
