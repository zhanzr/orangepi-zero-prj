# ili9163c_md144_128x128_spi

ILI9163C 1.44" 128x128 LCD (MD144 connector) for the Orange Pi Zero,
ported from `rk3506-dev/app/ili9163c_md144_128x128_spi` (Vanxoak
rk3506-mini <- f411 reference). Hardware SPI transport only (spidev,
9-bit frames packed in software); the driver logic and the f411 flow are
unchanged.

**This module is 3-wire serial**: the connector has no D/C pin and no
backlight pin - every byte is a 9-bit frame (D/C bit clocked first, then
the 8 data bits, MSB first), and the backlight is hardwired on-module.

Driver facts carried over from the reference:
- gamma 0x26, frame rate 0xB1, power 0xC0/0xC1, VCOM 0xC5/0xC7,
  COLMOD 0x3A = 0x05 (16bpp), MADCTL 0x08 (180 deg vs the 0xC8 default),
  source direction 0xB7, gamma enable 0xF2, gamma tables 0xE0/0xE1
- geometry 128x128, COL_Pre = 0, ROW_Pre = 0 (init addresses columns
  0..127 and pages 0..159)

## Wiring (CON4, 26-pin header)

| LCD pin | CON4 pin | Signal (SoC) | Notes |
|---|---|---|---|
| GND | 20 | GND | |
| VCC | 17 | VCC3V3 | 3.3 V |
| SCL | 23 | SPI1_CLK (PA14) | hw SPI1 via `/dev/spidev1.0` |
| SDA | 19 | SPI1_MOSI (PA15) | plain MOSI - verified free (the red LED is on PA17, *not* on this header) |
| RES | 7 | PA6 | active low, gpio 6; shared with the other module demos - run one at a time |
| CS | 24 | SPI1_CE0 (PA13) | hw CS0 |

(MISO on CON4.21/PA16 exists on the bus but the panel is write-only -
leave it unconnected.)

## Board preparation (already done on this board)

- `spi-spidev` DT overlay with `param_spidev_spi_bus=1`
  (`/boot/armbianEnv.txt`, reboot) -> `/dev/spidev1.0` (SPI1 on PA13-16)
- Non-root access: same `tools/gpio_i2c.rules` udev rule covers
  `/dev/spidev*` (group `spi`) - installed by `tools/setup_gpio_i2c.sh`

## Build and run

```
make push run           # cross-build, scp, run (default 40 MHz, driver clamps)
./ili9163c_demo -s 8000 # slower, for long wires
```

Demo: init (reset + ILI9163C init + black clear), then loops: solid red
-> 8-color band -> gray ramp -> banner "now will do / HW SPI1 test" ->
scaled font "OPi-Zero", FPS band + counter. Ctrl-C clean.

SPI writes succeed even without the module attached (no ACK on SPI), so
a blank screen with no errors means: check wiring, especially the
swappable SCL/SDA-free module pinout (GND-VCC-CS-RES-SDA-SCL order
varies between MD144 boards).
