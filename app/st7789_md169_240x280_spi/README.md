# st7789_md169_240x280_spi

ST7789V 1.69" 240x280 (MD169) SPI TFT for the Orange Pi Zero, ported from
`rk3506-dev/app/st7789_md169_240x280_spi` (hw spidev version <- f411/f746
reference). Hardware SPI1 transport (`/dev/spidev1.0`, **SPI MODE 0**).

**Isolation result (2026-11-01): this demo runs fully on the OPi Zero** -
the board-side transport (sunxi SPI1, DC/RES GPIO, spidev, non-root) is
exonerated. Conclusion for the MD130: **SPI mode 3 on sunxi SPI1** is what
that panel rejects (see `docs/st7789_md130_issue.md`).

## Driver facts (carried over from the references)

- ST7789 240x280 glass on a 240x320 GRAM: **ROW_Pre = 20** (glass covers
  GRAM rows 20..299), MADCTL 0x00, IPS inversion (INVON 0x21)
- **SPI MODE 0** for this module (the MD130 wanted mode 3 - do not share
  transport assumptions blindly)
- CS is wired to the board's hw CS0 and toggled by spidev per write()

## Wiring (CON4, 26-pin header)

| Module | CON4 | Signal (SoC) | Notes |
|---|---|---|---|
| GND | 20 | GND | |
| **V50** | **2** | DCIN-5V | **5 V** - the module has an onboard LDO; V33 (CON4.17) also works, but 5 V is the proper supply |
| SCLK | 23 | SPI1_CLK (PA14) | hw SPI1, mode 0, `/dev/spidev1.0` |
| MOSI | 19 | SPI1_MOSI (PA15) | |
| RESET | 7 | PA6 | active low, gpio 6; shared with the other module demos |
| DC | 11 | PA1 | gpio 1 |
| CS | 24 | SPI1_CE0 (PA13) | hw CS0 |
| BLK | 17 | 3.3 V | full brightness (a plain 3.3 V BLK was already verified lit on the rk3506-mini for this module family) |

## Board preparation (already done on this board)

- `spi-spidev` overlay + `param_spidev_spi_bus=1` -> `/dev/spidev1.0`
- Non-root access via `tools/gpio_i2c.rules` (groups `gpio`/`spi`/`i2c`)

## Stock assets

`logo64.h` (Armbian boot logo from `/boot/boot.bmp`) and the 8x16 console
font (`ref/lcd_font_1608.c`, Lat15-Fixed16) are regenerated from the board
by `tools/gen_stock_assets.py`.

## Build and run

```
make push run           # cross-build, scp, run (default 40 MHz)
./st7789_demo -s 8000   # slower, for long wires
```

Demo loops: banner -> info -> inverted info -> TEST_STAND (8 screens,
write time + 4 s dwell each) -> gradient -> board boot logo -> scaled
fonts. Console logs phases, write times and fps. The panel keeps the
last frame after exit.
