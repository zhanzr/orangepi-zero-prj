# st7789_md130_240x240

ST7789V 1.3" 240x240 (MD130) SPI TFT for the Orange Pi Zero, ported from
`rk3506-dev/app/st7789_md130_240x240` (Vanxoak rk3506-mini <- f411
reference). Hardware SPI1 transport (`/dev/spidev1.0`, **SPI MODE 3**).

## Driver facts (carried over from the references)

- ST7789 240x240 glass at **GRAM rows 0..239**: `COL_Pre = ROW_Pre = 0`
  (a nonzero offset shifts every window and leaves a blank band)
- Init: sleep out, MADCTL 0x00, COLMOD 0x55 (16bpp), **INVON 0x21**
  (IPS panel), PORCTRL/VCOMS/VDVVRHEN/VRHS/VDVS/VCOM-offset/FRCTRL2/
  PWCTRL1 + pos/neg gamma, full-window CASET/RASET 0..239, display on
- **SPI MODE 3** (SCL idle high, rising-edge sample): this module renders
  nothing on mode 0 - the exact failure recorded in the reference repo's
  `c542_st7789_md130_possible_issue.md`. NOTE: the MD144 (ILI9163C) module
  works on mode 0; do not share transport code blindly.
- **CS is hardwired to GND** on the module - the board's CS0 pin
  (CON4.24) is left unconnected; spidev still toggles it harmlessly.
  The GRAM window auto-increments across spidev's 4096-byte chunk
  boundaries, so bulk rasters just work with CS permanently low.

## Wiring (CON4, 26-pin header)

| Module | CON4 | Signal (SoC) | Notes |
|---|---|---|---|
| GND | 20 | GND | |
| V33 | 17 | VCC3V3 | 3.3 V |
| SCLK | 23 | SPI1_CLK (PA14) | hw SPI1, mode 3, `/dev/spidev1.0` |
| MOSI | 19 | SPI1_MOSI (PA15) | |
| RESET | 7 | PA6 | active low, gpio 6; shared with the other module demos - run one at a time |
| DC | 11 | PA1 | gpio 1; command/data select |
| CS | - | - | **module-internal GND**, leave unconnected |
| BLK | 17 | 3.3 V | **full brightness, hardwired** - see below |

## Backlight difference vs the rk3506 port

The rk3506-mini drove BLK with 200 kHz PWM (11%/55% alternating loops).
On the OPi Zero this is **not possible** (verified 2026-11-01): the only
PWM-capable CON4 pin is PA6 (PWM1), but this Armbian kernel's pinctrl
table lacks `pwm1` on PA6 (a DT overlay enabling it is rejected:
"unsupported function pwm1 on pin PA6") and the pwm driver itself only
supports channel 0 (PA5 - not on the header). So BLK is tied to 3.3 V
(always full brightness) and the PWM code is a stub. If dimming is ever
needed, wire BLK to a spare GPIO (CON4.13/PA0, CON4.15/PA3,
CON4.22/PA2, CON4.26/PA10) for a coarse on/off, or use an external
PWM driver module.

## Board preparation (already done on this board)

- `spi-spidev` overlay + `param_spidev_spi_bus=1` -> `/dev/spidev1.0`
  (same as the ILI9163C app)
- Non-root access via `tools/gpio_i2c.rules` (groups `gpio`/`spi`/`i2c`)

## Stock assets

`logo64.h` (Armbian boot logo from `/boot/boot.bmp`) and the 8x16 console
font (`ref/lcd_font_1608.c`, Lat15-Fixed16) are regenerated from the board
by `tools/gen_stock_assets.py` - same as the ILI9163C app.

## Build and run

```
make push run           # cross-build, scp, run (default 40 MHz)
./st7789_demo -s 8000   # slower, for long wires
```

Demo loops (~1 min per loop): banner -> info -> inverted info ->
TEST_STAND (8 screens, write time + 4 s dwell each) -> HSV gradient ->
board boot logo (2x) -> scaled font (1x/2x/3x). Console logs phases,
write times and fps.
