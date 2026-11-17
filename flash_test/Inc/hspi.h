#ifndef HSPI_H
#define HSPI_H

#include "stm32u5xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

int HSPI1_Init(void);
XSPI_HandleTypeDef *HSPI1_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* HSPI_H */
