# Flash driver design

Small driver layer for the IS25LP128F on top of the STM32U5 XSPI HAL. Shared by the external loader and the bootloader; board bring-up (clocks, GPIO, HSPI1) stays outside this layer.

## 1. Geometry and identification

| Item | Value |
|---|---|
| Density | 128 Mbit = 16 MB |
| Page size | 256 bytes |
| Sector size | 4 KB |
| Block sizes | 32 KB and 64 KB (driver uses 4 KB sectors for programmer erase; optional 64 KB for bootloader) |
| Address width | 3 bytes (`EXTADD = 0`) |
| Erased value | `0xFF` |
| JEDEC ID (`9Fh`) | `9Dh 60h 18h` (manufacturer, memory type, capacity) |

Confirmed from datasheet Table 8.7: manufacturer `9Dh`, memory type + capacity `6018h`. The 1-byte device ID returned by `ABh`/`90h` is `17h` and is not used.

## 2. Command set used by the driver

| Operation | Opcode | Notes |
|---|---|---|
| Enable reset / reset | `66h` / `99h` | Software reset; wait `tSRST` afterwards |
| Read JEDEC ID | `9Fh` | Expect `9Dh 60h 18h` |
| Write enable | `06h` | Required before every program, erase and status write |
| Read status register | `05h` | WIP bit 0, WEL bit 1, BP0–BP3 bits 2–5, QE bit 6 |
| Write status register | `01h` | Clear BP bits; set QE |
| Page program (1-1-1) | `02h` | Loader: simple, robust |
| Quad page program (1-1-4) | `32h` | Optional faster path once QE is set |
| Sector erase 4 KB | `20h` | Used by `SectorErase` |
| Block erase 64 KB | `D8h` | Optional |
| Chip erase | `C7h` | Requires BP3–BP0 = 0 |
| Fast read quad I/O | `EBh` | Memory-mapped mode |

## 3. Status register

```
bit 7: SRWD
bit 6: QE      (non-volatile; required for IO2/IO3 and for EBh)
bit 5: BP3
bit 4: BP2
bit 3: BP1
bit 2: BP0
bit 1: WEL
bit 0: WIP
```

- **QE** must be `1` before any quad command or memory-mapped quad read. Factory default is `0`; write once if needed, then only check.
- **BP3–BP0** must all be `0` before chip erase. A chip erase with any BP bit set is ignored. The driver clears them in `Init` (or immediately before mass erase) via write enable + `WRSR`, then waits until WIP clears.
- Poll WIP after every program, erase and status-register write.

## 4. Dummy cycles and mode bits for `EBh`

From datasheet Table 6.11 and section 8.9:

| Setting | Dummy cycles for `EBh` | Max clock for `EBh` |
|---|---|---|
| Default (`P[6:3] = 0`) | **6** | 81 MHz |
| Explicit `P[6:3] = 6` | 6 | 81 MHz |

- The loader runs HSPI1 at 24 MHz, so the default **6 dummy cycles** is correct and leaves margin.
- Of those 6 cycles, the first **2** carry the 8 mode bits (`M7–M0`) on four lines; the remaining 4 are pure dummy.
- Mode bits `M[7:4] = Ah` enter continuous (AX) read and skip the next command byte. For normal memory-mapped use the alternate bytes must **not** be `AXh` (use `0x00`) so each access sends a fresh `EBh`.
- XSPI HAL mapping: instruction 8 bits on 1 line, address 24 bits on 4 lines, alternate bytes 8 bits on 4 lines, 4 further dummy cycles (or configure 6 total dummy with alternate included, matching the HAL’s usual FRQIO setup), data on 4 lines.

No need to program the Read Parameters register for the loader. The bootloader may later raise the HSPI clock; if it stays at or below 81 MHz it can keep 6 dummy cycles.

## 5. Timeouts (AC characteristics, max)

Use the datasheet maxima with a small safety factor. Values are for polling WIP, not for busy-wait without status reads.

| Operation | Datasheet max | Driver timeout |
|---|---|---|
| Page program (`tPP`) | 0.8 ms | 5 ms |
| Sector erase 4 KB | 300 ms | 500 ms |
| Block erase 64 KB | 1.0 s | 2 s |
| Chip erase | 90 s | 120 s |
| Write status register (`tW`) | 15 ms | 40 ms |
| Software reset recovery (`tSRST`) | 35 µs | 100 µs delay after `99h` |
| Power-up to first CE# (`tVCE`) | 300 µs min | Board must respect this; driver does not wait on every `Init` beyond reset recovery |

## 6. Driver API

Addresses are flash-relative (`0` … `0x00FFFFFF`), not the memory-mapped base. The loader subtracts `0xA0000000` before calling into the driver. Return `0` on success, negative on error (wrong ID, timeout, argument error).

```c
/* Geometry (also mirrored in Dev_Inf StorageInfo) */
#define IS25_FLASH_SIZE       0x01000000u
#define IS25_PAGE_SIZE        0x100u
#define IS25_SECTOR_SIZE      0x1000u
#define IS25_JEDEC_ID         0x9D6018u

int32_t IS25_Init(XSPI_HandleTypeDef *hxspi);
int32_t IS25_Reset(void);
int32_t IS25_ReadID(uint32_t *id);
int32_t IS25_EnableQuad(void);
int32_t IS25_ClearBlockProtect(void);   /* BP3–BP0 = 0, keep QE */
int32_t IS25_WriteEnable(void);
int32_t IS25_WaitWhileBusy(uint32_t timeout_ms);

int32_t IS25_EraseSector(uint32_t address);          /* 4 KB, address aligned */
int32_t IS25_EraseChip(void);                        /* clears BP first */
int32_t IS25_Program(uint32_t address, const uint8_t *data, uint32_t size);
int32_t IS25_Read(uint32_t address, uint8_t *data, uint32_t size); /* indirect, optional */

int32_t IS25_EnableMemoryMapped(void);  /* EBh, 6 dummy, mode 0x00 */
int32_t IS25_DisableMemoryMapped(void); /* back to indirect for erase/program */
```

### Expected call order

**Loader / bootloader init**

1. Board code: clocks, GPIO, HSPI1 in indirect mode.
2. `IS25_Init` → reset, read ID, enable quad if needed, clear BP bits (stays in indirect mode).
3. Loader/bootloader calls `IS25_EnableMemoryMapped` when it needs XIP or programmer read-back.

**Sector erase / program (loader)**

1. `IS25_DisableMemoryMapped` (also done automatically inside erase/program helpers)
2. `IS25_EraseSector` / `IS25_Program` (each does write enable + WIP poll; program splits page boundaries)
3. Caller may re-enter memory-mapped mode for verify.

**Mass erase**

1. `IS25_EraseChip` (clears BP, write enable, `C7h`, wait up to chip-erase timeout).

## 7. Design decisions

| Decision | Choice | Why |
|---|---|---|
| Addressing | 3-byte | Full 16 MB fits; no 4-byte mode |
| Program command | `02h` for the loader | Works without QE on the data phase; simpler first bring-up. Quad `32h` can be added later |
| Erase granularity for CubeProgrammer | 4 KB sectors | Matches `StorageInfo` and typical ST loaders |
| Memory-mapped read | `EBh` + 6 dummy + mode `0x00` | Default flash setting; safe at 24 MHz and up to 81 MHz |
| Reset on every init | Software `66h`/`99h` | No dedicated reset line once QE is set; MCU reset does not reset the flash |
| Protection | Clear BP in init and before chip erase | Chip erase is a no-op if any BP bit is set |
| HAL usage | Polling only | Loader has no interrupt vector table; see [polled time base](system_concept.md#polled-time-base) (`loader/Src/loader_timebase.c`) |

## 8. Ownership boundary

| In the flash driver (`flash/`) | Outside |
|---|---|
| Commands, status, QE, BP, timeouts | `loader/`: CubeProgrammer entry points, board clock/GPIO/HSPI, polled HAL tick |
| Indirect and memory-mapped XSPI command config for this part | `flash_test/`: HSE / SYSCLK / HSPI clock tree, GPIO AF, on-target harness |
| Page splitting and busy polling | `third_party/`: STM32U5 HAL + CMSIS |
| JEDEC ID check | |

## References

- [IS25LP128F datasheet](https://www.issi.com/WW/pdf/25LP-WP128F.pdf) (ISSI, Rev. A10): Tables 6.11, 8.3, 8.7; sections 8.9, 9.6, 9.8, 9.9
- [doc/system_concept.md](system_concept.md) — roles of loader vs bootloader and memory map
