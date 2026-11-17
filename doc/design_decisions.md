# Design decisions and trade-offs

Project-level choices for the IS25LP128F external loader on STM32U5G9ZJ. Driver-specific detail lives in [`flash_driver_design.md`](flash_driver_design.md); system roles and memory map in [`system_concept.md`](system_concept.md).

## 1. Scope

| Decision | Choice | Trade-off |
|---|---|---|
| Primary deliverable | Concept + `.stldr` loader skeleton sharing a flash driver with a future bootloader | A fully proven on-hardware loader and product bootloader/app are stretch / later |
| TrustZone | Assumed **disabled**; single non-secure world | Simpler bring-up; revisit only if option bytes or product security require TZEN |
| Application storage | External NOR at HSPI1 memory-mapped base `0xA0000000` | Large code space and XIP; image is physically dumpable without extra crypto (see open points) |

## 2. Architecture split

| Decision | Choice | Trade-off |
|---|---|---|
| Three roles | Bootloader (internal flash), external loader (RAM / `.stldr`), application (external flash) | Clear ownership; more components to integrate later (milestone 6) |
| Shared flash driver | `flash/` used by loader and (later) bootloader; board clocks/GPIO stay outside | Avoids duplicating JEDEC/QE/BP/erase/program; each consumer still owns its clock tree |
| Loader vs field update | CubeProgrammer + `.stldr` for lab/factory only | No in-field update path yet — must be designed separately (open point) |

## 3. External loader

| Decision | Choice | Trade-off |
|---|---|---|
| Link address | SRAM1 from `0x20000004` | Leaves word at `0x20000000` for CubeProgrammer’s call/BKPT stub; wrong base breaks entry calls |
| RAM budget | Lower ~256 KB for the loader image | Leaves room in the rest of 768 KB SRAM1 for the programmer’s data buffer |
| Entry model | No `Reset_Handler` / `main`; `ENTRY(Init)`, BSS cleared in `Init` | Matches how CubeProgrammer invokes symbols; no C runtime startup |
| Time base | Polled SysTick (`TICKINT` off), `loader_timebase.c` | Timeouts work without a vector table; no IRQ-driven `HAL_IncTick` |
| Loader clock | SYSCLK = 24 MHz HSE, no PLL; HSPI = SYSCLK | Simple and safe; programming time dominated by flash tPROG/tERASE, not CPU MHz |
| Mode switching | `Init` ends memory-mapped; erase/program drop mmap via the driver; `Verify`/`Read` re-enable | Lets the tool read `0xA0…` after init; write/erase use indirect commands |
| StorageInfo | Separate ELF `SgInfo` / `.Dev_info` segment | Host reads geometry from the file; not from the target |
| Keep entry symbols | `-Wl,--undefined=…` for Init/Write/… | Prevents `--gc-sections` from stripping CubeProgrammer entry points |

## 4. Flash driver

Summary of choices documented in [`flash_driver_design.md`](flash_driver_design.md):

| Decision | Choice | Trade-off |
|---|---|---|
| Addressing | 3-byte | Full 16 MB; no 4-byte mode complexity |
| Program opcode | `02h` (1-1-1) for the loader | Robust first bring-up; quad `32h` left for later speed-up |
| Erase for programmer | 4 KB sectors | Matches `StorageInfo` and typical ST loaders; slower mass erase than large blocks |
| Memory-mapped read | `EBh`, 6 dummy, mode `0x00` | Safe at 24 MHz (and up to 81 MHz); avoids continuous-read (AX) traps |
| Reset on init | Software `66h`/`99h` | No dedicated flash reset pin once QE is set |
| Block protect | Clear BP in init and before chip erase | Chip erase is ignored if any BP bit is set |
| HAL style | Polling only | Required for the loader; `flash_test` may still use SysTick IRQ |

## 5. Build and layout

| Decision | Choice | Trade-off |
|---|---|---|
| Build system | CMake + root Makefile dispatcher | Replaces a Make-only test build; two artifacts from one tree |
| Targets | `make flash_test` vs `make loader` (default `all`) | Clear separation; must run make from repo root |
| Toolchain | `arm-none-eabi` in Docker / Dev Container | Reproducible; host without the toolchain uses Compose wrappers |
| Output | `flash_test.elf` (internal flash) and `IS25LP128F_STM32U5G9ZJ.stldr` (renamed ELF) | `.stldr` is what CubeProgrammer loads; keep `.elf` for `nm`/`readelf` |

## 6. Verification

| Decision | Choice | Trade-off |
|---|---|---|
| On-target suite | `flash_test/` exercises the driver (including error paths) | Proves HSPI + IS25 behaviour; does **not** replace CubeProgrammer loader tests |
| Destructive region | Only sectors `0x0000` and `0x1000` | Limits wear; not a full-chip stress test |
| Chip erase in suite | Not in the default suite | Too slow (~tens of seconds); covered only by API/docs |
| Loader on hardware | Still open (milestone 5) | Skeleton builds; end-to-end with CubeProgrammer pending a board |

## 7. Explicit non-goals (for now)

- In-field update without a debugger (open point)
- Encrypting / signing the XIP image (OTFDEC, secure boot — open point)
- ICACHE / DCACHE enable, remap, MPU, and invalidate policy for loader coherency and XIP (open point)
- Native SEGGER loader format (would wrap the same driver)
- Full bootloader + application jump sequence (milestone 6, stretch, concept-level)

## Related docs

- [`system_concept.md`](system_concept.md) — roles, memory map, programming flow, polled time base
- [`flash_driver_design.md`](flash_driver_design.md) — geometry, commands, driver API
- [`flash_test_coverage.md`](flash_test_coverage.md) — on-target cases and fail codes
- [`../project_management/milestones.md`](../project_management/milestones.md) — status and open points
