# CON4 - 26-pin expansion header (verified)

Source: `OrangePi_zero_schematic_v1.5.pdf` (sheet 11 "Ext Port") + the
vendor connector table in `D:\board_database\orangepi-zero\README.md`.
Signal-to-SoC-pin mapping from schematic sheet 6 (CPU) net labels.

| CON4 pin | Signal | SoC pin | Function / notes |
|---|---|---|---|
| 1 | VCC3V3-EXT | - | 3.3 V supply to header (see PL6 note below) |
| 2 | DCIN-5V | - | 5 V input rail |
| 3 | TWI0-SDA | PA12 | I2C0 SDA (overlay `i2c0` -> `/dev/i2c-0`) |
| 4 | DCIN-5V | - | |
| 5 | TWI0-SCK | PA11 | I2C0 SCL |
| 6 | GND | - | |
| 7 | PWM1 | PA6 | GPIO - shared RESET (PA6) for the SSD1317/ILI9163C/ST7789 demos; PWM1-capable in silicon but **unusable on this Armbian kernel** (pinctrl table lacks pwm1@PA6 - DT overlay rejected; pwm driver supports channel 0/PA5 only - verified 2026-11-01) |
| 8 | UART1_TX | PG6 | |
| 9 | GND | - | |
| 10 | UART1_RX | PG7 | |
| 11 | UART2_RX / IO-0 | PA1 | free GPIO (DC for the ST7789 demo) |
| 12 | PA7 / IO-1 | PA7 | free GPIO |
| 13 | UART2_TX / IO-2 | PA0 | free GPIO |
| 14 | GND | - | |
| 15 | UART2_CTS / IO-3 | PA3 | free GPIO |
| 16 | IO-4 / TWI1-SDA | PA19 | I2C1 SDA (overlay `i2c1`); 2K pull-up on board |
| 17 | VCC3V3 | - | |
| 18 | IO-5 / TWI1-SCK | PA18 | I2C1 SCK; 2K pull-up on board |
| 19 | SPI1_MOSI | PA15 | plain SPI1 MOSI - verified free (red LED is NOT here, see below) |
| 20 | GND | - | |
| 21 | SPI1_MISO | PA16 | |
| 22 | IO-6 / UART2_RTS | PA2 | free GPIO |
| 23 | SPI1_CLK | PA14 | |
| 24 | SPI-CE0 | PA13 | |
| 25 | GND | - | |
| 26 | PA10 / SPI-CE1 | PA10 | free GPIO |

## Notes (verified on the running system)

- All four I2C controllers are disabled in the stock Armbian DT; enable
  with overlays in `/boot/armbianEnv.txt`: `overlays=... i2c0` (PA11/PA12,
  `/dev/i2c-0`) or `i2c1` (PA18/PA19). `i2c2` muxes to PE12/PE13 - **not**
  on this header. The CPUS rtc-TWI (PL0/PL1) is used internally.
- PL6 (`VCC-IO-EN` per schematic, gates the header 3.3 V section) is
  claimed by the kernel as the `vdd-cpux` regulator enable and is driven -
  the 3.3 V header pins are on by default; do not use PL6 as GPIO.
- **On-board LEDs (verified via `/sys/kernel/debug/gpio`)**: red
  `orangepi:red:status` = **PA17** (gpio-17), green `orangepi:green:pwr` =
  **PL10** (gpio-10, r_pio). Both are *not* on the CON4 header - the
  vendor manual's table (green PL10 / red PA17) is correct. PA15 (pin 19)
  is plain SPI1_MOSI: pinmux shows `function spi1, GPIO UNCLAIMED`.
- **Schematic erratum**: sheet 4 "GPIO ASSIGNMENT" marks PA15 =
  STATUS-LED / PA17 = SPDIF-OUT - contradicted by the running system
  (red LED = PA17, PA15 = free MOSI). See the erratum note in the root
  README. Trust sheet 6/11 net labels + the DT, not sheet 4.
- On-board SPI flash option (U54) is on SPI0 (PC0-PC3), **not** on this
  header; the header carries SPI1 (PA13-PA16).
- Free GPIOs for expansion: PA6 (pin 7), PA1 (11), PA7 (12), PA0 (13),
  PA3 (15), PA2 (22), PA10 (26) - all bank A = `/dev/gpiochip0`, line =
  PA number. PL bank (PL0-PL11) = `/dev/gpiochip1`, mostly used by the
  board itself (usb power, keys, LEDs, wifi en).
- GPIO access as non-root: `dev` is in the `gpio`/`i2c` groups and
  `tools/gpio_i2c.rules` (installed by `tools/setup_gpio_i2c.sh`) keeps
  the char devices group-writable.
