# Milestones

Goal: concept and first implementation of an external loader (`.stldr`) for the IS25LP128F NOR flash on an STM32U5G9ZJ board. The focus is a traceable approach and sound technical concepts; a fully working loader is the stretch target.

## Overview

| # | Milestone | Status |
|---|---|---|
| 1 | Analyse the inputs | Done |
| 2 | Technical concept | Done |
| 3 | Flash driver design | Done |
| 4 | Loader skeleton | Done |
| 5 | Test and verification strategy | Open |
| 6 | Bootloader and application integration (stretch) | Open |

## 1. Analyse the inputs

Pin down how the flash is connected and which peripheral drives it.

- [x] Read the flash pins from the schematic: PH9 NCS, PH10–PH13 IO0–IO3, PI3 CLK
- [x] Identify the peripheral: HSPI1, 4 data lines,
- [x] Fix the memory-mapped base address: `0xA0000000` (`HSPI1_BASE`)
- [x] Fix the clock source: 24 MHz HSE crystal (assumption)

## 2. Technical concept

Describe how the pieces fit together. Result: [`doc/technical_concept.md`](../doc/technical_concept.md).

- [x] Roles of bootloader, external loader and application
- [x] Memory map: internal flash, RAM used by the loader, external flash at `0xA0000000`
- [x] Boot flow: reset, flash initialisation, memory-mapped mode, jump to the application
- [x] Programming flow: how STM32CubeProgrammer uses the loader
- [x] Architecture diagram
- [x] Clock concept, tooling, assumptions and open points

## 3. Flash driver design

A small driver layer on top of the XSPI HAL, kept separate so the bootloader can reuse it. Result: [`doc/flash_driver_design.md`](../doc/flash_driver_design.md).

- [x] Collect geometry from the datasheet: 16 MB, 256-byte pages, 4 KB sectors, erased value `0xFF`, 3-byte addressing
- [x] Collect the command set from the datasheet: reset `66h`/`99h`, JEDEC ID `9Fh`, write enable `06h`, read status `05h`, page program `02h` / quad `32h`, sector erase `20h`, block erase `D8h`, chip erase `C7h`, quad read `EBh`
- [x] Status register layout: QE bit 6, WEL bit 1, WIP bit 0
- [x] Confirm the JEDEC ID bytes in the datasheet: `9Dh 60h 18h` (Table 8.7)
- [x] Confirm dummy cycles versus frequency (Table 6.11) and the mode bits for `EBh`: default 6 cycles (up to 81 MHz); first 2 cycles carry mode bits; use mode `0x00` (not `AXh`)
- [x] Take program, erase and power-up times from the AC characteristics for the timeouts
- [x] Define the driver API: init, reset, read ID, enable quad, clear BP, write enable, wait while busy, erase, program, read, memory-mapped mode
- [x] Clear block protection before chip erase (BP3–BP0 must be 0 or `C7h` is ignored)

## 4. Loader skeleton

The core deliverable.

- [x] `Dev_Inf.c` with the `StorageInfo` struct: base `0xA0000000`, size `0x01000000`, page size `0x100`, sector size `0x1000`
- [x] `Loader_Src.c` with `Init`, `Write`, `SectorErase`, `MassErase`, and optionally `Read`, `Verify` and `CheckSum`
- [x] Clock and GPIO setup inside `Init`: SYSCLK from the 24 MHz HSE without PLL, HSPI clocked from SYSCLK
- [x] Polling instead of interrupts, no dependency on SysTick interrupts
- [x] Linker script for execution from RAM
- [x] Build that outputs the ELF as `.stldr`

## 5. Test and verification strategy

On-target driver suite: [`flash_test/`](../flash_test/), coverage map [`doc/flash_test_coverage.md`](../doc/flash_test_coverage.md).

- [x] Erase, write, read-back and verify through the on-target test app
- [x] Edge cases: unaligned addresses, writes crossing page boundaries, sizes that are not a multiple of the page size
- [x] Error handling: timeouts, wrong JEDEC ID, protected sectors
- [ ] Erase, write, read-back and verify through STM32CubeProgrammer (loader)
- [ ] Run on hardware if a board is available

## 6. Bootloader and application integration (stretch)

Concept level only.

- [ ] Link the application to the external flash address
- [ ] Relocate the vector table
- [ ] Jump sequence from the bootloader to the application



