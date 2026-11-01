# Applications

User-space applications for the Orange Pi Zero (H2+, Armbian 26.11). Each
subdirectory is one self-contained project with its own `Makefile` and
`README.md`.

| Project | Description |
|---|---|
| `hello/` | Minimal hello world - proves the cross toolchain + deploy loop |
| `blink_hello/` | Alternates the two on-board LEDs (green pwr / red status) + SoC die temp sampling |
| `coremark/` | EEMBC CoreMark 1.0, ARM and Thumb-2 variants (results in its README) |
| `ssd1317_md066_64x48_iic/` | SSD1317 64x48 I2C OLED on CON4 (i2c-dev + GPIO uAPI), ported from rk3506-dev |
| `ili9163c_md144_128x128_spi/` | ILI9163C 1.44" 128x128 SPI LCD on CON4 (hw SPI1 + 9-bit frames), ported from rk3506-dev |
| `st7789_md130_240x240/` | ST7789V 1.3" 240x240 SPI TFT on CON4 (hw SPI1 mode 3 + DC), ported from rk3506-dev - **broken on this board, see `docs/st7789_md130_issue.md`** |
| `st7789_md169_240x280_spi/` | ST7789V 1.69" 240x280 SPI TFT on CON4 (hw SPI1 mode 0 + DC), ported from rk3506-dev - **verified working**; its success isolated the MD130 failure to mode 3 |
| `rec_wav/` | Audio recorder: up to 30 s WAV, `q` stops early; MP3/OGG/FLAC via on-board encoders. H3 codec mic = CON5 10/11/12 |
| `netbench/` | TCP throughput benchmark, both interfaces both directions (`make bench`); ethernet 77/63 Mbit/s, wifi 1.5/6.5 Mbit/s (see README) |
| `uvc_to_st7789_md169_240x280_spi/` | UVC camera viewer on the MD169 panel: auto-detect, `uh-N` host number + fps band, `UVC NA` without camera; MJPEG via vendored libjpeg |
| `uvc_face_detect_st7789_md169_240x280_spi/` | UVC camera + **face detection** on the MD169 panel: vendored libfacedetection CNN (NEON, weights in-tree, no training), detector pthread at ~430 ms/pass while video streams at 15 fps, green boxes mapped through the panel rotation, face count in the band OSD; 18-bit RGB666 default |
| `uvc_face_pico_st7789_md169_240x280_spi/` | same, with the **pico** cascade (MIT, embedded facefinder): ~5-25 ms/pass - 30-60x faster than the CNN at some accuracy cost |
| `uvc_object_detect_st7789_md169_240x280_spi/` | **80-class object detection** (Yolo-Fastest v1.1 COCO-80 + NCNN NEON, vendored): ~280 ms/pass, 13-14 fps video, boxes + class labels on panel; model pre-flattened with ncnnoptimize (raw layout hangs armv7 load) |
| `uvc_qrscan_st7789_md169_240x280_spi/` | **QR/barcode reader** (zbar, board package): luma -> Y800 every frame, decoded text on panel + log; built natively on the board |
| `uvc_ped_detect_st7789_md169_240x280_spi/` | **pedestrian detection** (OpenCV HOG, pre-trained): detector thread + green boxes; built natively on the board (libopencv-dev) |
| `uvc_person_vis_st7789_md169_240x280_spi/` | **"is a person visible?"** (MLPerf-Tiny person_detect.tflite via TFLM): ~460 ms verdicts, PERSON % over live video; model in-tree |
| `uvc_wake_word_st7789_md169_240x280_spi/` | **"yes/no" wake word** (TFLM micro_speech + audio preprocessor model): H3 codec mic via ALSA, 1 s ring, ~2 verdicts/s, verdict + meter on panel; `-d` selects mic |
| _overview_ | models, runtimes, board features and licenses for all AI/ML apps: `docs/ai-ml-projects.md` |
| `e_server/` | Embedded web server (port 8080): LED control (green/red), CPU temp plot, MJPEG `/stream` from the UVC camera, board info. Ported from mp157-dk1-prj |

## Common workflow (per project, from the project directory)

Fast loop during development - cross-build on the host (WSL) and push only
the binary:

```
make        # arm-linux-gnueabihf-gcc
make push   # scp the binary to ~/<project>/ on the board
make run    # execute it there
```

Deploy sources + Makefile (then build on the board with its native gcc):

```
make deploy
make build-remote
```

Requirements: the `opz` ssh alias in `~/.ssh/config` (set up - see the
root README) and `arm-linux-gnueabihf-gcc` on PATH (WSL apt package
`gcc-arm-linux-gnueabihf`). `env.sh` at the repo root documents/exports
the variables; the Makefiles default to the same values and work without
sourcing it. Apps touching the LEDs run as the plain `dev` user - the
board has a udev rule delegating LED writes to group `leds`
(`tools/setup_led_group.sh`, one-time setup).
