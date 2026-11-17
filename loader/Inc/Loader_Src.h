/**
 * @file Loader_Src.h
 * @brief STM32CubeProgrammer external-loader entry points.
 *
 * Return convention: 1 = success, 0 = failure (except Verify/CheckSum).
 * Addresses from the programmer are absolute (base 0xA0000000).
 */
#ifndef LOADER_SRC_H
#define LOADER_SRC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOADER_OK   0x1
#define LOADER_FAIL 0x0

#define EXT_FLASH_BASE  0xA0000000u

#if defined(__ICCARM__)
#define KeepInCompilation __root
#else
#define KeepInCompilation __attribute__((used))
#endif

KeepInCompilation int Init(void);
KeepInCompilation int Write(uint32_t Address, uint32_t Size, uint8_t *buffer);
KeepInCompilation int SectorErase(uint32_t EraseStartAddress, uint32_t EraseEndAddress);
KeepInCompilation int MassErase(void);
KeepInCompilation int Read(uint32_t Address, uint32_t Size, uint8_t *buffer);

KeepInCompilation uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal);
KeepInCompilation uint64_t Verify(uint32_t MemoryAddr, uint32_t RAMBufferAddr,
                                  uint32_t Size, uint32_t missalignement);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_SRC_H */
