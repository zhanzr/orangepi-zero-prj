# st7365_md350_320x480 — ST7365P 3.5" 320x480 SPI panel (MD350)

Group for apps driving the **ST7365P** 3.5" 320x480 SPI panel (MD350).
ST7365P is ST7796-register-compatible; the init sequence is ported from
the proven f411 reference (`f4-demo/nano-f411/app/st7365_md350_320x480`).

Panel notes (verified on this module):
- MADCTL 0x48 (portrait, BGR), COLMOD 0x05 (16bpp RGB565) — the proven
  configuration; windows at COL_Pre = 0 / ROW_Pre = 0 (full GRAM)
- 18bpp (COLMOD 0x06) also works over SPI with 3-byte pixels
  (24 bits/pixel, 2 pad bits per byte ignored)
- reset timing: high 20 ms -> low 20 ms -> high 200 ms
- the glass has an inactive border outside the 320x480 active area —
  it renders as a light strip and cannot display content
- SPI at 100 MHz works and is ~2x faster than 40 MHz on this header

## Apps

- `lcm_test/` — bring-up demo: color/gray/band screens, boot logo,
  font scales, info page (pixel format, SPI speed, UVC host probe)
- `uvc_to_lcd/` — UVC camera viewer: crop-fill center (no letterbox),
  OSD line (mode/SPI/host) over the video, fps counter on the frame;
  camera auto-detect + lifecycle (UVC NA + rescan). RGB565 is the
  default; pass `-6` for RGB666.
- `whiteband_test/` — write-path isolator for the top strip
  (rows 0..27): cycles black+text / white / black

`lcm_test` and `whiteband_test` use RGB565 only; `-6` is available on
`uvc_to_lcd`, which has both pixel-format paths.
