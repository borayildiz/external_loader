/**
 * @file Dev_Inf.c
 * @brief IS25LP128F geometry for STM32CubeProgrammer (milestone 4.1).
 *
 * Values mirror flash/Inc/is25lp128f.h and doc/flash_driver_design.md:
 * 16 MB at 0xA0000000, 256-byte pages, 4 KB sectors, erased 0xFF.
 */
#include "Dev_Inf.h"

#if defined(__ICCARM__)
__root struct StorageInfo const StorageInfo = {
#else
struct StorageInfo const StorageInfo __attribute__((section(".Dev_info"))) = {
#endif
  "IS25LP128F_STM32U5G9ZJ", /* device name shown in CubeProgrammer */
  NOR_FLASH,               /* HSPI/OCTOSPI NOR */
  0xA0000000u,             /* memory-mapped base (HSPI1) */
  0x01000000u,             /* 16 MB */
  0x100u,                  /* page program size */
  0xFFu,                   /* erased value */
  {
    {0x01000000u / 0x1000u, 0x1000u}, /* 4096 × 4 KB sectors */
    {0u, 0u},
  }
};
