# uvc_to_st7789_md169_240x280_spi

Stream a UVC camera to the ST7789V 1.69" 240x280 (MD169) SPI panel on
the Orange Pi Zero (hw SPI1 mode 0, same transport as
`app/st7789_md169_240x280_spi`; SPI clock requested at 100 MHz - the
controller clamps to its ~37.5 MHz ceiling).

- **Auto-detects** the first UVC capture device under `/dev/video*`
  (driver `uvcvideo`, capture capability)
- Shows the **USB host controller number** as `uh-N` in the bottom band
  (derived from the controller base in the device's sysfs path:
  `1c19000`=uh-0/OTG, `1c1b400`=uh-1, `1c1c400`=uh-2, `1c1d400`=uh-3),
  together with the live fps
- **No camera: shows `UVC NA`** and rescans every 2 s — plug one in and
  it starts streaming automatically; a camera that disappears mid-stream
  is released and the app returns to the scanning state
- **Exclusive use:** the camera and the panel are single-open resources —
  e_server and this app cannot run at the same time (the second one to
  start exits with a busy message)
- V4L2 MMAP streaming; **format/size auto-selected per camera** by
  enumerating its descriptors: YUYV preferred (raw path, no decode),
  else MJPEG decoded via the board's libjpeg (`libjpeg.so.8`, vendored;
  minimal `include/jconfig.h` for the cross build); largest size that
  fits 640x480 wins. Both paths convert with BT.601 limited-range math
  (UVC cameras encode limited-range - using full-range washes the image)
- rotated 90 deg and scaled to the portrait panel, 4096-byte chunked
  blits over spidev
- corrupt/truncated MJPEG frames are skipped (non-fatal libjpeg error
  handler); the panel is the fps bottleneck (~45 ms per full blit)

## Color pipeline (fixed 2026-11-01)

YUYV/MJPEG chroma -> **BT.601 limited-range** -> RGB666 (18-bit), chroma
properly centered (an early version kept the +128 offset in the formula -
permanent magenta-white veil on every camera). Tunables: `-G` luma
gamma exponent %, `-S` chroma saturation %, `-R lim|full` source range,
`-5` legacy 16-bit RGB565 (defaults 100/100/lim, 18-bit on). The fps line
also prints the framebuffer average R/G/B as a color-balance telemetry.

**Panel color depth - why 18-bit is the default:** the ST7789VW expands
incoming RGB565 to its 18-bit frame memory through an internal LUT that
the datasheet documents only as a black box (sec 8.8.42, "Look-up table
for 65k color data mapping", no contents given). Empirically that LUT
washes colors: the same scene matched the browser view only when the
panel is driven in 18-bit mode (sec 8.8.43: 3 bytes/pixel, each 6-bit
component in D7..D2) with the conversion pipeline feeding full 8-bit
values quantized to 6 bits ourselves. 18-bit costs no fps (15-16 fps
verified); `-5` remains for reference/bring-up. The earlier "residual
color abnormality" question is closed - it was the 16-bit expansion,
not the gamma registers (verified against the ST7789VW datasheet,
vendor-tuned E0/E1).
- known cameras: a no-name "Web Camera" (32e6:9228, usbhost 1) is
  **MJPEG-only 160x120@30** - its entire capability list, no RGB/YUV
  modes at any size (confirmed on Linux ENUM_FMT and the USB
  descriptors); expect washed-out color from that unit itself. A
  Logitech C920 (YUYV/MJPG up to 1080p) streams with correct color

## Wiring / prerequisites

Same panel wiring as `app/st7789_md169_240x280_spi` (its README has the
table). Camera: any UVC webcam in a USB host port - verified with a
"Web Camera" (32e6:9228) on usbhost 1 (`usb6` bus, controller
`1c1b400.usb`).

## Usage

```
make push run
```

Ctrl-C stops (shows `bye`). The fps print also goes to the console.
