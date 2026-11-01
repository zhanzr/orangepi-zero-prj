# e_server - embedded web demo (Orange Pi Zero port)

Single-page web app for the **Orange Pi Zero** (H2+), served by a native
Linux web server (`server.c`) running directly on the board. Ported from
the mp157-dk1-prj `e_server` (STM32MP157A-DK1 port of the apollo-f429 /
fire-f429 original frontend + host reference backend).

Adapted for the OPi Zero:

- **LED control** drives the two on-board LEDs via the kernel LED class
  (sysfs): **Green (power)** and **Red (status)**
  (`/sys/class/leds/orangepi:*`, writable by group `leds` - no root).
- **Sensors** plots the CPU die temperature (`cpu_thermal` thermal zone,
  °C). The H2+ has no exposed ADC, so `vrefint_mv`/`vddcore_mv` are
  reported as `null` by API contract and their plots were removed.
- **Streaming** tab: live **MJPEG camera stream** from a UVC webcam
  (`/stream`, 640×480 MJPEG, multipart/x-mixed-replace - opens in any
  browser `<img>`). One viewer at a time (further clients get 503); the
  camera must not be held by another app (uvc_to_lcd etc.). Each frame is
  also mirrored to the MD169 panel with a stable transparent `uh-N fps`
  OSD (drawn into the framebuffer - no flicker), and the camera
  detection refreshes on every stream start (hot-plug aware). Verified
  with the 32e6:9228 webcam on usbhost 1 (~15 fps over WiFi,
  USB-bottlenecked only by the camera).

  Panel color depth is **18-bit RGB666 by default** (`-5` = legacy
  16-bit RGB565). Reason: the ST7789VW expands incoming RGB565 to its
  18-bit frame memory through an internal LUT that the datasheet only
  shows as a black box (8.8.42, "Look-up table for 65k color data
  mapping", contents undocumented). Empirically that LUT washes colors;
  driving 18-bit mode directly (8.8.43: 3 bytes/pixel, each 6-bit
  component in D7..D2) with our own 6-bit values matches the browser
  view. Costs no fps (verified 15-16 fps in both modes).
- **Board info** reports `arch` + `lan_ip` (real), and best-effort
  `public_ip` / geo / weather over plain HTTP (ipify / ip-api / wttr.in);
  anything unavailable shows as "N/A".

## API (unchanged contract, adapted fields)

| Route           | Description |
| --------------- | ----------- |
| `GET /`         | gzipped page (CSS/JS inlined) |
| `GET /api/leds` | `{"leds":[0,0]}` - {Green, Red} |
| `POST /api/leds`| body `{"leds":[1,0]}` → applied, `{"leds":[1,0]}` |
| `GET /api/adc`  | `{"vrefint_mv":null,"vddcore_mv":null,"temp_c":41.62,"ts":…}` (no ADC on this board) |
| `GET /stream`   | MJPEG multipart stream (640×480), single viewer |
| `GET /api/info` | `{"arch":"arm","lan_ip":…,"public_ip":…,"geo":…,"weather":…,"ts":…}` |
| `GET /public/*` | raw image bytes from `embedded_files[]` |

## Access

Server listens on all interfaces, port from argv (default 8080):

```
http://192.168.5.226:8080/      # from the LAN (ethernet)
http://192.168.5.218:8080/      # or the wifi address
```

## Build & deploy (WSL cross)

```bash
python3 build_web.py     # regenerate web_assets.h after editing web/ or public/
make CC=arm-linux-gnueabihf-gcc
make push                # scp to opz:~/e_server
make run                 # start it (foreground; or run manually in background)
```

Runs as the plain `dev` user (LED sysfs + camera are group-delegated on
this board). Stop with Ctrl-C.
