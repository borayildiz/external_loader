# Known gaps and next steps

Status snapshot for the IS25LP128F external loader on STM32U5G9ZJ.
Open product questions (field update, IP protection, caches, SEGGER format) are listed under [milestones open points](../project_management/milestones.md#open-points); this page turns them into a practical backlog.

## Where things stand

| Area | Status | Notes |
|---|---|---|
| System concept | Done | [`system_concept.md`](system_concept.md) |
| Flash driver design + `flash/` | Done | Bring-up path: SPI program `02h`, mmap `EBh` |
| On-target driver suite | Done in code | [`flash_test/`](../flash_test/), [`flash_test_coverage.md`](flash_test_coverage.md); **not yet run on board** |
| Loader skeleton (`.stldr`) | Builds | Entry points, StorageInfo, polled time base; **CubeProgrammer E2E open** |
| Bootloader + XIP app | Concept only | Milestone 6 stretch |
| Documentation | Done | README, design decisions, this gaps doc |

Assumptions still in force: 24 MHz HSE; TrustZone off; boot from internal flash.

## Known gaps

### Verification (milestone 5)

- No confirmed run of `flash_test.elf` on hardware.
- No end-to-end STM32CubeProgrammer session with `IS25LP128F_STM32U5G9ZJ.stldr` (Init → erase → Write → Verify).
- Chip erase is API/docs only; not in the default on-target suite (slow).

### Loader robustness

- **ICACHE / DCACHE:** caches are not enabled in the loader, but residual state from prior firmware can poison memory-mapped `Read` / `Verify` / `CheckSum`. No disable/invalidate in `Init` yet.
- Loader clock stays at 24 MHz HSE / no PLL by design; fine for bring-up, not optimised for programming throughput.
- Program path uses SPI `02h` only; quad page program `32h` is deferred.

### Bootloader / application (milestone 6)

- No application linked at `0xA0000000`, no VTOR hand-over, no jump sequence beyond the concept in [`system_concept.md`](system_concept.md).
- Bootloader HSPI / SYSCLK target frequency not chosen.
- XIP performance path (ICACHE remap, MPU for HSPI window, DCACHE1 policy) undefined.
- Application must not touch HSPI while executing from external flash; no sample “run-from-RAM” helper yet.

### Product / tooling (open points)

- No in-field update path without CubeProgrammer / debug probe.
- No encryption / signing of the external image (OTFDEC, secure boot).
- Native SEGGER loader format not supported (`.stldr` is CubeProgrammer only).

## Recommended next steps

Ordered for closing the current deliverable first, then stretch work.

1. **On-hardware `flash_test`** — flash `flash_test.elf`, confirm `g_flash_test_result == 0`, fix pin/clock/ID issues if any.
2. **CubeProgrammer loader bring-up** — install the `.stldr`, program a known pattern into external NOR, verify via the tool and optionally via mmap read in a debugger. Close the two open checklist items under milestone 5.
3. **Loader cache hygiene** — in `Init` (and before mmap verify after write/erase), disable and/or invalidate ICACHE and DCACHE1 so verify is correct regardless of prior firmware. See open point in milestones.
4. **Optional loader speed-ups** — only after E2E works: consider quad program `32h`, and/or a higher HSPI clock if timing allows.
5. **Milestone 6 (stretch)** — minimal bootloader in internal flash: init flash → mmap → validate SP/Reset in `0xA0…` → set VTOR → jump; plus a tiny app linked to `0xA0000000`. Decide clock, ICACHE remap, MPU, and DCACHE1 as part of that design.
6. **Product open points (later)** — field-update transport and image layout; OTFDEC / signing if IP protection is required; SEGGER wrapper only if those tools are needed.

## Related docs

- [`../project_management/milestones.md`](../project_management/milestones.md) — checklist status and open points
- [`design_decisions.md`](design_decisions.md) — choices and explicit non-goals
- [`system_concept.md`](system_concept.md) — roles, memory map, boot and programming flows
- [`flash_driver_design.md`](flash_driver_design.md) — driver API and command choices
- [`flash_test_coverage.md`](flash_test_coverage.md) — on-target fail codes
