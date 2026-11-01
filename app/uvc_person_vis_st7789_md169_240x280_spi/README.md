# uvc_person_vis_st7789_md169_240x280_spi

**"Is a person visible?"** — MLPerf-Tiny person detection on the
ST7789V 1.69" 240x280 (MD169) SPI panel, Orange Pi Zero.

## Model + runtime

- model: **person_detect.tflite** (MobileNetV1 0.25, 96x96x1 int8,
  2 classes person/notperson; MLPerf-Tiny V0.7 vision benchmark model,
  Apache-2.0, ships inside the tflite-micro repo at
  `tensorflow/lite/micro/models/person_detect.tflite`)
- runtime: **TensorFlow Lite Micro** (`vendor/tflm/lib/libtensorflow-microlite.a`,
  cross-built from the tflite-micro tree with gcc-12; the app uses a
  `MicroMutableOpResolver<5>`: Conv2D / DepthwiseConv2D / AveragePool2D /
  Reshape / Softmax, int8 variants; tensor arena 136 KB)
- preprocessing: 320x240 luma sample -> 96x96 box-downsample -> int8
  quantize using the tensor's own scale/zero-point (read at runtime)

## Board features used

- UVC camera (YUYV 640x480) via V4L2 MMAP, auto lifecycle
- MD169 panel over SPI1 (18-bit RGB666 default)
- NEON via `-march=armv7-a+neon-vfpv4` (TFLM generic kernels)

## Run characteristics (measured on the H3)

- inference: **~455-490 ms per pass** on a detector pthread; camera
  video keeps streaming at **25 fps** with the C920 (15 fps with the
  32e6 module)
- verdict `PERSON <score>%` (score >= 60%) or `no person <score>%`
  drawn over the live view; band OSD shows the score + det time
- x86 cross-check: sample WAV-free image pipeline validated against
  tflite-micro's own PersonDetectionTest (passes on x86 and on the board)

## Build & deploy (WSL cross)

```bash
make
make push     # binary + person_detect.tflite
```

TFLM headers for the app compile live in `tflm_inc/` (generated from
the tflite-micro tree; see `docs/ai-ml-projects.md` for the recipe).
Stop other panel/camera apps first (single-open resources).
