# lcm_test (ST7365P 3.5" 320x480, MD350)

First bring-up app for the **ST7365P** 3.5" 320x480 SPI panel (MD350).
ST7365P is ST7796-register-compatible; the init sequence is ported
verbatim from the proven f411 reference
(`f4-demo/nano-f411/app/st7365_md350_320x480`).

- geometry 320x480, windows at COL_Pre = 0 / ROW_Pre = 0 (full GRAM)
- MADCTL 0x48 (portrait, BGR), COLMOD 0x05 (16bpp RGB565)
- reset timing (f411-verified): high 20 ms -> low 20 ms -> high 200 ms
- same transport as the MD169 apps: spi_hw (SPI1, DC/RES GPIO), panel
  rasters stream with DC held high

## Run

```bash
./lcm_test [-s speed_kHz]
```

Cycles the color/gray/band/logo/font screens with 4 s dwell. Stop other
panel apps first (single-open resources).

## Wiring note

Backlight (BL) must be at 3.3 V for the panel to light up.
