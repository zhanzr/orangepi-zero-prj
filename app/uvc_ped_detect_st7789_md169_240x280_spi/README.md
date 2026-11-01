# uvc_ped_detect_st7789_md169_240x280_spi

UVC camera **pedestrian detection** on the ST7789V 1.69" 240x280 (MD169)
SPI panel - Orange Pi Zero, using OpenCV's **HOG people detector**
(Dalal-Triggs SVM, `getDefaultPeopleDetector` - pre-trained, ships with
OpenCV, no model download).

- same capture/panel stack as the other uvc_* apps (18-bit RGB666
  default, auto camera lifecycle, `uh-N` + fps band)
- 320x240 luma sample -> detector pthread (video keeps streaming at
  15 fps); green 2 px boxes mapped through the panel rotation
- band OSD: `uh-N <fps> fps <n>p <det>ms`
- HOG wants upright full-body silhouettes: best at 2-6 m from the
  camera; expect some false positives (classic HOG behavior)

## Build & run (native on the board)

```bash
sudo apt-get install -y build-essential libopencv-dev pkg-config
# sources pushed by `make board` from WSL, then:
cd uvc_ped_detect_st7789_md169_240x280_spi && make
./ped_detect
```

Stop other panel/camera apps first (single-open resources). Expect
~300-700 ms per detection pass on the H3.
