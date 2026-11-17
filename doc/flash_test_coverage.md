# Flash driver test coverage

On-target coverage of the IS25LP128F driver (`flash/`) by the app in `flash_test/`.
Tests run from internal flash after `HSPI1_Init` and `IS25_Init`.

Result: `g_flash_test_result` — `0` pass, `1` not finished, negative = fail code below.

## Setup covered by `main` (before `FlashTests_RunAll`)

| Step | API / path | What it proves |
|---|---|---|
| Clock | HSE 24 MHz, no PLL | Same clock concept as the loader |
| HSPI1 | pins PH9–PH13, PI3; kernel = SYSCLK | Board wiring and XSPI init |
| `IS25_Init` | reset, JEDEC check, QE, clear BP | Device present and quad-ready |

If `IS25_Init` fails, `Error_Handler` is entered and `g_flash_test_result` stays `1`.

## Cases in `FlashTests_RunAll`

| Fail code | Action | Covers |
|---|---|---|
| `-1` | `IS25_ReadID` HAL/API failure | Indirect JEDEC read path |
| `-2` | ID ≠ `9Dh 60h 18h` | ID value check (also done in `IS25_Init`) |
| `-3` | `IS25_EraseSector(0)` | 4 KB sector erase + WIP poll |
| `-4` | Read after erase fails | Indirect `03h` read |
| `-5` | Byte ≠ `0xFF` after erase | Erased value |
| `-6` | `IS25_Program(0, 512)` | Page program; **crosses page boundary** (2×256) |
| `-7` | Read-back of 512 bytes fails | Multi-page indirect read |
| `-8` | Buffer mismatch | Program data integrity |
| `-9` | Erase sector at `0x1000` | Second-sector erase |
| `-10` | `IS25_Program(0x1003, 100)` | **Unaligned address** + size not a page multiple |
| `-11` | Read of 100 bytes fails | Short unaligned read |
| `-12` | Buffer mismatch | Unaligned program integrity |
| `-13` | `IS25_EnableMemoryMapped` | `EBh` mmap config (6 dummy, mode `0x00`) |
| `-14` | CPU read at `0xA0000000` ≠ programmed pattern | Memory-mapped read vs earlier program at `0` |
| `-15` | `IS25_DisableMemoryMapped` | Return to indirect mode |

Destructive region used: sectors `0x0000` and `0x1000` only (8 KB).

## API coverage

| API | Exercised? | How |
|---|---|---|
| `IS25_Init` | Yes | `main` |
| `IS25_Reset` | Indirect | Inside `IS25_Init` |
| `IS25_ReadID` | Yes | Explicit + inside init |
| `IS25_EnableQuad` | Indirect | Inside `IS25_Init` |
| `IS25_ClearBlockProtect` | Indirect | Inside `IS25_Init` |
| `IS25_WriteEnable` | Indirect | Erase / program paths |
| `IS25_WaitWhileBusy` | Indirect | After erase / program / status write |
| `IS25_EraseSector` | Yes | Two sectors |
| `IS25_Program` | Yes | Aligned multi-page + unaligned short |
| `IS25_Read` | Yes | After erase and after program |
| `IS25_EnableMemoryMapped` | Yes | Explicit |
| `IS25_DisableMemoryMapped` | Yes | Explicit |
| `IS25_EraseChip` | **No** | Slow (~tens of seconds); not in the default suite |

## Edge cases (milestone 5)

| Case | Status |
|---|---|
| Writes crossing page boundaries | Covered (`512` bytes from `0`) |
| Unaligned program address | Covered (`sector + 3`) |
| Size not a multiple of page size | Covered (`100` bytes) |
| Erase → `0xFF` verify | Covered |
| Memory-mapped read-back | Covered (sector 0 pattern) |
| Wrong JEDEC ID handling | Only via init failure path, not a dedicated negative test |
| Timeouts | Not forced |
| Protected sectors / BP reject | Not covered (init clears BP) |
| Chip erase | Not covered |
| Out-of-range / null args (`IS25_ERR_PARAM`) | Not covered |
| CubeProgrammer `.stldr` path | Out of scope for this app |

## How to run

```bash
git submodule update --init --recursive
make   # from repo root → flash_test/build/flash_test.elf
# flash and run; read g_flash_test_result
```
