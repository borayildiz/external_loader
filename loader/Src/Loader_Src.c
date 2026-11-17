/**
 * @file Loader_Src.c
 * @brief External loader entry points for IS25LP128F on STM32U5G9ZJ (milestone 4.2).
 *
 * CubeProgrammer loads this image into RAM and calls these symbols through
 * the debug probe. See doc/system_concept.md §5.
 *
 * Clock / GPIO / HSPI: Loader_BoardInit (loader_board.c).
 * Polled time base without SysTick IRQ: milestone 4.4.
 */
#include "Loader_Src.h"
#include "loader_board.h"
#include "is25lp128f.h"

#include <stddef.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/* Address helpers                                                            */
/* -------------------------------------------------------------------------- */

static uint32_t Loader_ToOffset(uint32_t address)
{
  if (address >= EXT_FLASH_BASE) {
    return address - EXT_FLASH_BASE;
  }
  return address;
}

static int Loader_OffsetInRange(uint32_t offset, uint32_t size)
{
  if (offset >= IS25_FLASH_SIZE) {
    return 0;
  }
  if (size > (IS25_FLASH_SIZE - offset)) {
    return 0;
  }
  return 1;
}

/* -------------------------------------------------------------------------- */
/* Minimal HAL tick stubs (replaced by polled time base in milestone 4.4)     */
/* -------------------------------------------------------------------------- */

KeepInCompilation HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  (void)TickPriority;
  /* Do not start SysTick IRQ — loader has no vector table in use. */
  return HAL_OK;
}

uint32_t HAL_GetTick(void)
{
  return uwTick;
}

void HAL_Delay(uint32_t Delay)
{
  /* Busy-wait placeholder until 4.4 provides a real time base. */
  volatile uint32_t n = Delay * 4000u;
  while (n-- != 0u) {
  }
}

/* -------------------------------------------------------------------------- */
/* Required entry points                                                      */
/* -------------------------------------------------------------------------- */

KeepInCompilation int Init(void)
{
  XSPI_HandleTypeDef *hxspi;

  /*
   * No C runtime: zero .bss when the RAM linker script exports the symbols
   * (milestone 4.5). Until then, rely on the programmer loading a clean image.
   */
  SystemInit();

  if (HAL_Init() != HAL_OK) {
    return LOADER_FAIL;
  }

  if (Loader_BoardInit() != 0) {
    return LOADER_FAIL;
  }

  hxspi = Loader_GetXspiHandle();
  if (hxspi == NULL) {
    return LOADER_FAIL;
  }

  if (IS25_Init(hxspi) != IS25_OK) {
    return LOADER_FAIL;
  }

  /* Leave memory-mapped so CubeProgrammer can read the flash window. */
  if (IS25_EnableMemoryMapped() != IS25_OK) {
    return LOADER_FAIL;
  }

  return LOADER_OK;
}

KeepInCompilation int Write(uint32_t Address, uint32_t Size, uint8_t *buffer)
{
  uint32_t offset;

  if ((buffer == NULL) || (Size == 0u)) {
    return LOADER_FAIL;
  }

  offset = Loader_ToOffset(Address);
  if (!Loader_OffsetInRange(offset, Size)) {
    return LOADER_FAIL;
  }

  /* IS25_Program leaves memory-mapped mode via EnsureReady. */
  if (IS25_Program(offset, buffer, Size) != IS25_OK) {
    return LOADER_FAIL;
  }

  return LOADER_OK;
}

KeepInCompilation int SectorErase(uint32_t EraseStartAddress, uint32_t EraseEndAddress)
{
  uint32_t start = Loader_ToOffset(EraseStartAddress);
  uint32_t end   = Loader_ToOffset(EraseEndAddress);

  if (start > end) {
    return LOADER_FAIL;
  }
  if (end >= IS25_FLASH_SIZE) {
    return LOADER_FAIL;
  }

  start &= ~(IS25_SECTOR_SIZE - 1u);

  while (start <= end) {
    if (IS25_EraseSector(start) != IS25_OK) {
      return LOADER_FAIL;
    }
    if (start > (IS25_FLASH_SIZE - IS25_SECTOR_SIZE)) {
      break;
    }
    start += IS25_SECTOR_SIZE;
  }

  return LOADER_OK;
}

KeepInCompilation int MassErase(void)
{
  if (IS25_EraseChip() != IS25_OK) {
    return LOADER_FAIL;
  }
  return LOADER_OK;
}

/* -------------------------------------------------------------------------- */
/* Optional entry points                                                      */
/* -------------------------------------------------------------------------- */

KeepInCompilation int Read(uint32_t Address, uint32_t Size, uint8_t *buffer)
{
  const uint8_t *src;
  uint32_t offset;

  if ((buffer == NULL) || (Size == 0u)) {
    return LOADER_FAIL;
  }

  offset = Loader_ToOffset(Address);
  if (!Loader_OffsetInRange(offset, Size)) {
    return LOADER_FAIL;
  }

  if (IS25_EnableMemoryMapped() != IS25_OK) {
    return LOADER_FAIL;
  }

  src = (const uint8_t *)(EXT_FLASH_BASE + offset);
  memcpy(buffer, src, Size);
  return LOADER_OK;
}

/**
 * Byte-wise checksum over a memory-mapped window (ST loader algorithm).
 * Size is in bytes here when called from Verify after scaling.
 */
KeepInCompilation uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal)
{
  uint8_t missalignementAddress = (uint8_t)(StartAddress % 4u);
  uint8_t missalignementSize    = (uint8_t)Size;
  uint32_t cnt;
  uint32_t Val;

  StartAddress -= StartAddress % 4u;
  Size += (Size % 4u == 0u) ? 0u : (4u - (Size % 4u));

  for (cnt = 0; cnt < Size; cnt += 4u) {
    Val = *(uint32_t *)StartAddress;

    if (missalignementAddress != 0u) {
      switch (missalignementAddress) {
        case 1:
          InitVal += (uint8_t)((Val >> 8) & 0xffu);
          InitVal += (uint8_t)((Val >> 16) & 0xffu);
          InitVal += (uint8_t)((Val >> 24) & 0xffu);
          missalignementAddress = 0;
          break;
        case 2:
          InitVal += (uint8_t)((Val >> 16) & 0xffu);
          InitVal += (uint8_t)((Val >> 24) & 0xffu);
          missalignementAddress = 0;
          break;
        case 3:
          InitVal += (uint8_t)((Val >> 24) & 0xffu);
          missalignementAddress = 0;
          break;
        default:
          break;
      }
    } else if (((Size - missalignementSize) % 4u) && ((Size - cnt) <= 4u)) {
      switch (Size - missalignementSize) {
        case 1:
          InitVal += (uint8_t)Val;
          InitVal += (uint8_t)((Val >> 8) & 0xffu);
          InitVal += (uint8_t)((Val >> 16) & 0xffu);
          missalignementSize = 0;
          break;
        case 2:
          InitVal += (uint8_t)Val;
          InitVal += (uint8_t)((Val >> 8) & 0xffu);
          missalignementSize = 0;
          break;
        case 3:
          InitVal += (uint8_t)Val;
          missalignementSize = 0;
          break;
        default:
          break;
      }
    } else {
      InitVal += (uint8_t)Val;
      InitVal += (uint8_t)((Val >> 8) & 0xffu);
      InitVal += (uint8_t)((Val >> 16) & 0xffu);
      InitVal += (uint8_t)((Val >> 24) & 0xffu);
    }

    StartAddress += 4u;
  }

  return InitVal;
}

KeepInCompilation uint64_t Verify(uint32_t MemoryAddr, uint32_t RAMBufferAddr,
                                  uint32_t Size, uint32_t missalignement)
{
  uint32_t VerifiedData = 0;
  uint32_t InitVal      = 0;
  uint64_t checksum;

  /* Size is in words from CubeProgrammer. */
  Size *= 4u;

  if (IS25_EnableMemoryMapped() != IS25_OK) {
    return LOADER_FAIL;
  }

  checksum = CheckSum(MemoryAddr + (missalignement & 0xfu),
                      Size - ((missalignement >> 16) & 0xFu),
                      InitVal);

  while (Size > VerifiedData) {
    if (*(uint8_t *)MemoryAddr++ !=
        *(((uint8_t *)RAMBufferAddr) + VerifiedData)) {
      return (checksum << 32) + MemoryAddr;
    }
    VerifiedData++;
  }

  return checksum << 32;
}
