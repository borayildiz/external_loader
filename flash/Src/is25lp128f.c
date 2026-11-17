/**
 * @file is25lp128f.c
 * @brief IS25LP128F driver using STM32U5 HAL XSPI (HSPI1 / OCTOSPI).
 */
#include "is25lp128f.h"

static XSPI_HandleTypeDef *s_hxspi;

/* -------------------------------------------------------------------------- */
/* Helpers                                                                    */
/* -------------------------------------------------------------------------- */

static void IS25_CmdDefaults(XSPI_RegularCmdTypeDef *cmd)
{
  cmd->OperationType          = HAL_XSPI_OPTYPE_COMMON_CFG;
  cmd->IOSelect               = HAL_XSPI_SELECT_IO_3_0;
  cmd->InstructionMode        = HAL_XSPI_INSTRUCTION_1_LINE;
  cmd->InstructionWidth       = HAL_XSPI_INSTRUCTION_8_BITS;
  cmd->InstructionDTRMode     = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  cmd->Address                = 0;
  cmd->AddressMode            = HAL_XSPI_ADDRESS_NONE;
  cmd->AddressWidth           = HAL_XSPI_ADDRESS_24_BITS;
  cmd->AddressDTRMode         = HAL_XSPI_ADDRESS_DTR_DISABLE;
  cmd->AlternateBytes         = 0;
  cmd->AlternateBytesMode     = HAL_XSPI_ALT_BYTES_NONE;
  cmd->AlternateBytesWidth    = HAL_XSPI_ALT_BYTES_8_BITS;
  cmd->AlternateBytesDTRMode  = HAL_XSPI_ALT_BYTES_DTR_DISABLE;
  cmd->DataMode               = HAL_XSPI_DATA_NONE;
  cmd->DataLength             = 0;
  cmd->DataDTRMode            = HAL_XSPI_DATA_DTR_DISABLE;
  cmd->DummyCycles            = 0;
  cmd->DQSMode                = HAL_XSPI_DQS_DISABLE;
  cmd->SIOOMode               = HAL_XSPI_SIOO_INST_EVERY_CMD;
}

static int32_t IS25_SendCommand(const XSPI_RegularCmdTypeDef *cmd, uint32_t timeout_ms)
{
  if (HAL_XSPI_Command(s_hxspi, cmd, timeout_ms) != HAL_OK) {
    return IS25_ERR_HAL;
  }
  return IS25_OK;
}

static int32_t IS25_ReadStatus(uint8_t *status)
{
  XSPI_RegularCmdTypeDef cmd;
  int32_t err;

  IS25_CmdDefaults(&cmd);
  cmd.Instruction = IS25_CMD_RDSR;
  cmd.DataMode    = HAL_XSPI_DATA_1_LINE;
  cmd.DataLength  = 1;

  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }
  if (HAL_XSPI_Receive(s_hxspi, status, IS25_TIMEOUT_DEFAULT_MS) != HAL_OK) {
    return IS25_ERR_HAL;
  }
  return IS25_OK;
}

static int32_t IS25_WriteStatus(uint8_t status)
{
  XSPI_RegularCmdTypeDef cmd;
  int32_t err;

  err = IS25_WriteEnable();
  if (err != IS25_OK) {
    return err;
  }

  IS25_CmdDefaults(&cmd);
  cmd.Instruction = IS25_CMD_WRSR;
  cmd.DataMode    = HAL_XSPI_DATA_1_LINE;
  cmd.DataLength  = 1;

  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }
  if (HAL_XSPI_Transmit(s_hxspi, &status, IS25_TIMEOUT_DEFAULT_MS) != HAL_OK) {
    return IS25_ERR_HAL;
  }

  return IS25_WaitWhileBusy(IS25_TIMEOUT_WRSR_MS);
}

static int32_t IS25_AutoPoll(uint32_t match, uint32_t mask, uint32_t timeout_ms)
{
  XSPI_RegularCmdTypeDef cmd;
  XSPI_AutoPollingTypeDef cfg;
  int32_t err;

  IS25_CmdDefaults(&cmd);
  cmd.Instruction = IS25_CMD_RDSR;
  cmd.DataMode    = HAL_XSPI_DATA_1_LINE;
  cmd.DataLength  = 1;

  err = IS25_SendCommand(&cmd, timeout_ms);
  if (err != IS25_OK) {
    return err;
  }

  cfg.MatchValue    = match;
  cfg.MatchMask     = mask;
  cfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  cfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;
  cfg.IntervalTime  = 0x10;

  if (HAL_XSPI_AutoPolling(s_hxspi, &cfg, timeout_ms) != HAL_OK) {
    return IS25_ERR_TIMEOUT;
  }
  return IS25_OK;
}

/** Busy-wait for software-reset recovery (~100 us). Avoids SysTick. */
static void IS25_DelayResetRecovery(void)
{
  volatile uint32_t n = 2400u; /* ~100 us at 24 MHz, roughly */
  while (n-- != 0u) {
  }
}

static int32_t IS25_EnsureReady(void)
{
  if (s_hxspi == 0) {
    return IS25_ERR_PARAM;
  }
  if (HAL_XSPI_IsMemoryMapped(s_hxspi) != 0u) {
    if (HAL_XSPI_Abort(s_hxspi) != HAL_OK) {
      return IS25_ERR_HAL;
    }
  }
  return IS25_OK;
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

int32_t IS25_Init(XSPI_HandleTypeDef *hxspi)
{
  int32_t err;

  if (hxspi == 0) {
    return IS25_ERR_PARAM;
  }

  s_hxspi = hxspi;

  err = IS25_DisableMemoryMapped();
  if (err != IS25_OK) {
    return err;
  }

  err = IS25_Reset();
  if (err != IS25_OK) {
    return err;
  }

  err = IS25_CheckJedecId(IS25_JEDEC_ID);
  if (err != IS25_OK) {
    return err;
  }

  err = IS25_EnableQuad();
  if (err != IS25_OK) {
    return err;
  }

  err = IS25_ClearBlockProtect();
  if (err != IS25_OK) {
    return err;
  }

  return IS25_OK;
}

int32_t IS25_Reset(void)
{
  XSPI_RegularCmdTypeDef cmd;
  int32_t err;

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  IS25_CmdDefaults(&cmd);
  cmd.Instruction = IS25_CMD_ENABLE_RESET;
  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }

  cmd.Instruction = IS25_CMD_RESET;
  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }

  IS25_DelayResetRecovery();
  return IS25_WaitWhileBusy(IS25_TIMEOUT_DEFAULT_MS);
}

int32_t IS25_ReadID(uint32_t *id)
{
  XSPI_RegularCmdTypeDef cmd;
  uint8_t buf[3];
  int32_t err;

  if (id == 0) {
    return IS25_ERR_PARAM;
  }

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  IS25_CmdDefaults(&cmd);
  cmd.Instruction = IS25_CMD_READ_JEDEC_ID;
  cmd.DataMode    = HAL_XSPI_DATA_1_LINE;
  cmd.DataLength  = 3;

  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }
  if (HAL_XSPI_Receive(s_hxspi, buf, IS25_TIMEOUT_DEFAULT_MS) != HAL_OK) {
    return IS25_ERR_HAL;
  }

  *id = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | (uint32_t)buf[2];
  return IS25_OK;
}

int32_t IS25_CheckJedecId(uint32_t expected)
{
  uint32_t id;
  int32_t err;

  err = IS25_ReadID(&id);
  if (err != IS25_OK) {
    return err;
  }
  if (id != expected) {
    return IS25_ERR_ID;
  }
  return IS25_OK;
}

int32_t IS25_EnableQuad(void)
{
  uint8_t status;
  int32_t err;

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  err = IS25_ReadStatus(&status);
  if (err != IS25_OK) {
    return err;
  }

  if ((status & IS25_SR_QE) != 0u) {
    return IS25_OK;
  }

  /* Set QE, clear BP bits while we are writing the non-volatile SR. */
  status = (uint8_t)((status | IS25_SR_QE) & (uint8_t)~IS25_SR_BP_MASK);
  return IS25_WriteStatus(status);
}

int32_t IS25_ClearBlockProtect(void)
{
  uint8_t status;
  int32_t err;

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  err = IS25_ReadStatus(&status);
  if (err != IS25_OK) {
    return err;
  }

  if ((status & IS25_SR_BP_MASK) == 0u) {
    return IS25_OK;
  }

  status = (uint8_t)(status & (uint8_t)~IS25_SR_BP_MASK);
  return IS25_WriteStatus(status);
}

int32_t IS25_SetBlockProtect(uint8_t bp_bits)
{
  uint8_t status;
  int32_t err;

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  err = IS25_ReadStatus(&status);
  if (err != IS25_OK) {
    return err;
  }

  bp_bits = (uint8_t)(bp_bits & IS25_SR_BP_MASK);
  status  = (uint8_t)((status & (uint8_t)~IS25_SR_BP_MASK) | bp_bits);
  return IS25_WriteStatus(status);
}

int32_t IS25_WriteEnable(void)
{
  XSPI_RegularCmdTypeDef cmd;
  int32_t err;

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  IS25_CmdDefaults(&cmd);
  cmd.Instruction = IS25_CMD_WREN;
  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }

  /* Wait until WEL is set. */
  return IS25_AutoPoll(IS25_SR_WEL, IS25_SR_WEL, IS25_TIMEOUT_DEFAULT_MS);
}

int32_t IS25_WaitWhileBusy(uint32_t timeout_ms)
{
  /* Match when WIP (bit 0) is 0. */
  return IS25_WaitStatus(0x00u, IS25_SR_WIP, timeout_ms);
}

int32_t IS25_WaitStatus(uint32_t match, uint32_t mask, uint32_t timeout_ms)
{
  int32_t err;

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  return IS25_AutoPoll(match, mask, timeout_ms);
}

int32_t IS25_EraseSector(uint32_t address)
{
  XSPI_RegularCmdTypeDef cmd;
  int32_t err;

  if (address >= IS25_FLASH_SIZE) {
    return IS25_ERR_PARAM;
  }

  address &= ~(IS25_SECTOR_SIZE - 1u);

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  err = IS25_WriteEnable();
  if (err != IS25_OK) {
    return err;
  }

  IS25_CmdDefaults(&cmd);
  cmd.Instruction  = IS25_CMD_SECTOR_ERASE;
  cmd.AddressMode  = HAL_XSPI_ADDRESS_1_LINE;
  cmd.AddressWidth = HAL_XSPI_ADDRESS_24_BITS;
  cmd.Address      = address;

  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }

  return IS25_WaitWhileBusy(IS25_TIMEOUT_SE_MS);
}

int32_t IS25_EraseChip(void)
{
  XSPI_RegularCmdTypeDef cmd;
  int32_t err;

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  /* Chip erase is ignored unless BP3–BP0 are cleared. */
  err = IS25_ClearBlockProtect();
  if (err != IS25_OK) {
    return err;
  }

  err = IS25_WriteEnable();
  if (err != IS25_OK) {
    return err;
  }

  IS25_CmdDefaults(&cmd);
  cmd.Instruction = IS25_CMD_CHIP_ERASE;

  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }

  return IS25_WaitWhileBusy(IS25_TIMEOUT_CE_MS);
}

int32_t IS25_Program(uint32_t address, const uint8_t *data, uint32_t size)
{
  XSPI_RegularCmdTypeDef cmd;
  int32_t err;

  if ((data == 0) || (size == 0u) || (address >= IS25_FLASH_SIZE) ||
      (size > (IS25_FLASH_SIZE - address))) {
    return IS25_ERR_PARAM;
  }

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  while (size > 0u) {
    uint32_t page_off = address & (IS25_PAGE_SIZE - 1u);
    uint32_t chunk    = IS25_PAGE_SIZE - page_off;

    if (chunk > size) {
      chunk = size;
    }

    err = IS25_WriteEnable();
    if (err != IS25_OK) {
      return err;
    }

    IS25_CmdDefaults(&cmd);
    cmd.Instruction  = IS25_CMD_PAGE_PROGRAM;
    cmd.AddressMode  = HAL_XSPI_ADDRESS_1_LINE;
    cmd.AddressWidth = HAL_XSPI_ADDRESS_24_BITS;
    cmd.Address      = address;
    cmd.DataMode     = HAL_XSPI_DATA_1_LINE;
    cmd.DataLength   = chunk;

    err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
    if (err != IS25_OK) {
      return err;
    }
    if (HAL_XSPI_Transmit(s_hxspi, data, IS25_TIMEOUT_PP_MS) != HAL_OK) {
      return IS25_ERR_HAL;
    }

    err = IS25_WaitWhileBusy(IS25_TIMEOUT_PP_MS);
    if (err != IS25_OK) {
      return err;
    }

    address += chunk;
    data    += chunk;
    size    -= chunk;
  }

  return IS25_OK;
}

int32_t IS25_Read(uint32_t address, uint8_t *data, uint32_t size)
{
  XSPI_RegularCmdTypeDef cmd;
  int32_t err;

  if ((data == 0) || (size == 0u) || (address >= IS25_FLASH_SIZE) ||
      (size > (IS25_FLASH_SIZE - address))) {
    return IS25_ERR_PARAM;
  }

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  IS25_CmdDefaults(&cmd);
  cmd.Instruction  = IS25_CMD_READ;
  cmd.AddressMode  = HAL_XSPI_ADDRESS_1_LINE;
  cmd.AddressWidth = HAL_XSPI_ADDRESS_24_BITS;
  cmd.Address      = address;
  cmd.DataMode     = HAL_XSPI_DATA_1_LINE;
  cmd.DataLength   = size;

  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }
  if (HAL_XSPI_Receive(s_hxspi, data, IS25_TIMEOUT_DEFAULT_MS) != HAL_OK) {
    return IS25_ERR_HAL;
  }

  return IS25_OK;
}

int32_t IS25_EnableMemoryMapped(void)
{
  XSPI_RegularCmdTypeDef cmd;
  XSPI_MemoryMappedTypeDef mmap;
  int32_t err;

  err = IS25_EnsureReady();
  if (err != IS25_OK) {
    return err;
  }

  /*
   * Fast Read Quad I/O (EBh):
   *   instr 1-line, addr 4-line 24-bit, mode byte 4-line (2 clocks),
   *   then 4 more dummy clocks → 6 total (datasheet default).
   * Mode 0x00 avoids continuous (AX) read.
   */
  IS25_CmdDefaults(&cmd);
  cmd.Instruction           = IS25_CMD_FAST_READ_QIO;
  cmd.AddressMode           = HAL_XSPI_ADDRESS_4_LINES;
  cmd.AddressWidth          = HAL_XSPI_ADDRESS_24_BITS;
  cmd.AlternateBytesMode    = HAL_XSPI_ALT_BYTES_4_LINES;
  cmd.AlternateBytesWidth   = HAL_XSPI_ALT_BYTES_8_BITS;
  cmd.AlternateBytes        = IS25_FRQIO_MODE_BYTE;
  cmd.DummyCycles           = IS25_FRQIO_DUMMY_CYCLES - 2u; /* mode uses 2 of 6 */
  cmd.DataMode              = HAL_XSPI_DATA_4_LINES;
  cmd.DataLength            = 0;

  err = IS25_SendCommand(&cmd, IS25_TIMEOUT_DEFAULT_MS);
  if (err != IS25_OK) {
    return err;
  }

  mmap.TimeOutActivation  = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  mmap.TimeoutPeriodClock = 0;

  if (HAL_XSPI_MemoryMapped(s_hxspi, &mmap) != HAL_OK) {
    return IS25_ERR_HAL;
  }

  return IS25_OK;
}

int32_t IS25_DisableMemoryMapped(void)
{
  if (s_hxspi == 0) {
    return IS25_ERR_PARAM;
  }

  if (HAL_XSPI_IsMemoryMapped(s_hxspi) != 0u) {
    if (HAL_XSPI_Abort(s_hxspi) != HAL_OK) {
      return IS25_ERR_HAL;
    }
  }

  return IS25_OK;
}
