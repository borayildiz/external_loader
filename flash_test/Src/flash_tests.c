/**
 * @file flash_tests.c
 * @brief On-target tests for the IS25LP128F driver.
 */
#include <string.h>
#include "is25lp128f.h"
#include "main.h"

#define TEST_ADDR       0x00000000u
#define TEST_BUF_SIZE   512u

static uint8_t s_tx[TEST_BUF_SIZE];
static uint8_t s_rx[TEST_BUF_SIZE];

static int expect_ok(int32_t rc)
{
  return (rc == IS25_OK) ? 0 : -1;
}

int FlashTests_RunAll(void)
{
  uint32_t id = 0;
  uint32_t i;

  if (expect_ok(IS25_ReadID(&id)) != 0) {
    return -1;
  }
  if (id != IS25_JEDEC_ID) {
    return -2;
  }

  if (expect_ok(IS25_EraseSector(TEST_ADDR)) != 0) {
    return -3;
  }

  if (expect_ok(IS25_Read(TEST_ADDR, s_rx, IS25_PAGE_SIZE)) != 0) {
    return -4;
  }
  for (i = 0; i < IS25_PAGE_SIZE; i++) {
    if (s_rx[i] != 0xFFu) {
      return -5;
    }
  }

  for (i = 0; i < TEST_BUF_SIZE; i++) {
    s_tx[i] = (uint8_t)(i & 0xFFu);
  }

  /* Crosses a page boundary on purpose (512 bytes from page-aligned start). */
  if (expect_ok(IS25_Program(TEST_ADDR, s_tx, TEST_BUF_SIZE)) != 0) {
    return -6;
  }

  memset(s_rx, 0, TEST_BUF_SIZE);
  if (expect_ok(IS25_Read(TEST_ADDR, s_rx, TEST_BUF_SIZE)) != 0) {
    return -7;
  }
  if (memcmp(s_tx, s_rx, TEST_BUF_SIZE) != 0) {
    return -8;
  }

  /* Unaligned program within a freshly erased second sector. */
  if (expect_ok(IS25_EraseSector(IS25_SECTOR_SIZE)) != 0) {
    return -9;
  }
  if (expect_ok(IS25_Program(IS25_SECTOR_SIZE + 3u, s_tx, 100u)) != 0) {
    return -10;
  }
  memset(s_rx, 0, 100u);
  if (expect_ok(IS25_Read(IS25_SECTOR_SIZE + 3u, s_rx, 100u)) != 0) {
    return -11;
  }
  if (memcmp(s_tx, s_rx, 100u) != 0) {
    return -12;
  }

  if (expect_ok(IS25_EnableMemoryMapped()) != 0) {
    return -13;
  }
  {
    const volatile uint8_t *mapped =
        (const volatile uint8_t *)(EXT_FLASH_BASE + TEST_ADDR);
    for (i = 0; i < TEST_BUF_SIZE; i++) {
      if (mapped[i] != s_tx[i]) {
        (void)IS25_DisableMemoryMapped();
        return -14;
      }
    }
  }
  if (expect_ok(IS25_DisableMemoryMapped()) != 0) {
    return -15;
  }

  return 0;
}
