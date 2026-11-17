/**
 * @file is25lp128f.h
 * @brief IS25LP128F NOR flash driver on STM32U5 XSPI/HSPI.
 *
 * Addresses are flash-relative (0 .. IS25_FLASH_SIZE-1). Callers that use the
 * memory-mapped base (0xA0000000) must subtract it before calling this driver.
 * See doc/flash_driver_design.md.
 */
#ifndef IS25LP128F_H
#define IS25LP128F_H

#include <stdint.h>
#include "stm32u5xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Geometry */
#define IS25_FLASH_SIZE   0x01000000u
#define IS25_PAGE_SIZE    0x100u
#define IS25_SECTOR_SIZE  0x1000u
#define IS25_JEDEC_ID     0x9D6018u

/* Status register bits */
#define IS25_SR_WIP       (1u << 0)
#define IS25_SR_WEL       (1u << 1)
#define IS25_SR_BP0       (1u << 2)
#define IS25_SR_BP1       (1u << 3)
#define IS25_SR_BP2       (1u << 4)
#define IS25_SR_BP3       (1u << 5)
#define IS25_SR_QE        (1u << 6)
#define IS25_SR_SRWD      (1u << 7)
#define IS25_SR_BP_MASK   (IS25_SR_BP0 | IS25_SR_BP1 | IS25_SR_BP2 | IS25_SR_BP3)

/* Opcodes */
#define IS25_CMD_WREN          0x06u
#define IS25_CMD_RDSR          0x05u
#define IS25_CMD_WRSR          0x01u
#define IS25_CMD_READ          0x03u
#define IS25_CMD_PAGE_PROGRAM  0x02u
#define IS25_CMD_SECTOR_ERASE  0x20u
#define IS25_CMD_BLOCK_ERASE   0xD8u
#define IS25_CMD_CHIP_ERASE    0xC7u
#define IS25_CMD_READ_JEDEC_ID 0x9Fu
#define IS25_CMD_ENABLE_RESET  0x66u
#define IS25_CMD_RESET         0x99u
#define IS25_CMD_FAST_READ_QIO 0xEBu

/* Return codes: 0 = success, negative = error */
#define IS25_OK            0
#define IS25_ERR_PARAM    (-1)
#define IS25_ERR_HAL      (-2)
#define IS25_ERR_TIMEOUT  (-3)
#define IS25_ERR_ID       (-4)

/* Timeouts (ms), from datasheet max + margin — doc/flash_driver_design.md */
#define IS25_TIMEOUT_PP_MS      5u
#define IS25_TIMEOUT_SE_MS      500u
#define IS25_TIMEOUT_BE64_MS    2000u
#define IS25_TIMEOUT_CE_MS      120000u
#define IS25_TIMEOUT_WRSR_MS    40u
#define IS25_TIMEOUT_DEFAULT_MS 1000u

/** Fast Read Quad I/O: 6 dummy clocks total; first 2 carry mode bits. */
#define IS25_FRQIO_DUMMY_CYCLES  6u
#define IS25_FRQIO_MODE_BYTE     0x00u  /* not AXh — exit continuous read */

/**
 * @brief Bind the XSPI/HSPI handle, reset the flash, check JEDEC ID,
 *        enable QE and clear block protection. Leaves the device in
 *        indirect mode (call IS25_EnableMemoryMapped when needed).
 */
int32_t IS25_Init(XSPI_HandleTypeDef *hxspi);

int32_t IS25_Reset(void);
int32_t IS25_ReadID(uint32_t *id);
int32_t IS25_EnableQuad(void);
int32_t IS25_ClearBlockProtect(void);
int32_t IS25_WriteEnable(void);
int32_t IS25_WaitWhileBusy(uint32_t timeout_ms);

int32_t IS25_EraseSector(uint32_t address);
int32_t IS25_EraseChip(void);
int32_t IS25_Program(uint32_t address, const uint8_t *data, uint32_t size);
int32_t IS25_Read(uint32_t address, uint8_t *data, uint32_t size);

/** Memory-mapped Fast Read Quad I/O (EBh), 6 dummy, mode 0x00. */
int32_t IS25_EnableMemoryMapped(void);
/** Leave memory-mapped mode so erase/program can run. */
int32_t IS25_DisableMemoryMapped(void);

#ifdef __cplusplus
}
#endif

#endif /* IS25LP128F_H */
