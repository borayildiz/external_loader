/**
 * @file loader_board.h
 * @brief Board bring-up for the external loader (clock, GPIO, HSPI1).
 *
 * Implemented in milestone 4.3. Loader_Src calls these from Init().
 */
#ifndef LOADER_BOARD_H
#define LOADER_BOARD_H

#include "stm32u5xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Configure SYSCLK (24 MHz HSE), GPIO, and HSPI1. 0 = ok, non-zero = fail. */
int Loader_BoardInit(void);

/** HSPI1 handle bound by Loader_BoardInit. */
XSPI_HandleTypeDef *Loader_GetXspiHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_BOARD_H */
