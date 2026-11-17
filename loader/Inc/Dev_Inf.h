/**
 * @file Dev_Inf.h
 * @brief StorageInfo layout for STM32CubeProgrammer external loaders (.stldr).
 *
 * Shape matches ST's stm32-external-loader examples. CubeProgrammer reads
 * StorageInfo from the .stldr file (not from the target).
 */
#ifndef DEV_INF_H
#define DEV_INF_H

#define MCU_FLASH   1
#define NAND_FLASH  2
#define NOR_FLASH   3
#define SRAM        4
#define PSRAM       5
#define PC_CARD     6
#define SPI_FLASH   7
#define I2C_FLASH   8
#define SDRAM       9
#define I2C_EEPROM  10

#define SECTOR_NUM  10

struct DeviceSectors {
  unsigned long SectorNum;  /* number of sectors of this size */
  unsigned long SectorSize; /* bytes */
};

struct StorageInfo {
  char DeviceName[100];
  unsigned short DeviceType;
  unsigned long DeviceStartAddress;
  unsigned long DeviceSize;
  unsigned long PageSize;
  unsigned char EraseValue;
  struct DeviceSectors sectors[SECTOR_NUM];
};

#endif /* DEV_INF_H */
