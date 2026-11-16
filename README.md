# external_loader

External loader for programming an external NOR flash on an STM32U5 board with STM32CubeProgrammer.

Status: work in progress.

## Hardware

| Item | Value |
|---|---|
| MCU | STM32U5G9ZJ |
| External flash | ISSI IS25LP128F, 128 Mbit (16 MB) NOR, quad SPI |
| Interface | HSPI1, 4 data lines |
| Pins | PH9 NCS, PH10–PH13 IO0–IO3, PI3 CLK |
| Memory-mapped address | `0xA0000000` |
| Crystal | 24 MHz HSE |

## System concept

The application does not run from the MCU's internal flash. It is stored in the external NOR flash and executed from there. Three pieces of software make this work:

1. **Bootloader** – lives in the internal flash. After reset it initialises the external flash, switches HSPI1 to memory-mapped mode so the flash appears at `0xA0000000`, and jumps to the application.
2. **External loader** (this repository) – a small program that STM32CubeProgrammer loads into the MCU's RAM through the debug probe. It provides the routines the programmer calls to initialise, erase, write and verify the external flash, and is built as a `.stldr` file.
3. **Application** – linked to run at the external flash address and programmed there using the external loader.

```
            debug probe
PC  ───────────────────────►  MCU RAM: external loader
(STM32CubeProgrammer)                    │ HSPI1
                                         ▼
MCU internal flash: bootloader ──►  external NOR flash: application
                     (maps flash, jumps)
```

## References

- [IS25LP128F datasheet](https://www.issi.com/WW/pdf/25LP-WP128F.pdf) (ISSI, Rev. A10, 03/09/2026)