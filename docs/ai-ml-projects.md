# AI/ML projects — models, runtimes, board usage

All seven camera/audio AI apps in `app/` share the same skeleton:
UVC camera auto-detection (standard UVC probe, any compliant camera),
MD169 ST7789V panel output over SPI1 (**18-bit RGB666** default, `-5`
legacy 565 — see the panel notes in the per-app READMEs), and a
**detector-thread pattern**: the capture loop streams video at full
rate while AI inference runs on a pthread; results (boxes / verdicts)
are shared via a mutex+condvar handshake and drawn into both panel
buffers (RGB565 + RGB666) every frame, so the OSD never flickers.

Every model below is **pre-trained and vendored** — nothing is trained
at deploy time, nothing is downloaded at runtime.

| App | Task | Model (vendored) | Runtime | Inference (measured, H3) | Video fps |
|---|---|---|---|---|---|
| `uvc_face_detect_...` | face boxes | libfacedetection CNN (BSD, weights in `facedetectcnn-model.cpp`) | vendored C++ sources, NEON | ~430-450 ms | 15 |
| `uvc_face_pico_...` | face boxes, lighter | pico `facefinder` cascade (MIT, embedded via `facefinder_embed.h`) | `vendor/pico/picornt.c` (pure C) | **6-25 ms** | 24-25 |
| `uvc_object_detect_...` | 80-class object boxes | Yolo-Fastest v1.1 COCO-80 (`yf-opt.{param,bin}`, pre-flattened with ncnnoptimize) | **NCNN** (`vendor/ncnn/`, gcc-12, NEON) | ~280 ms | 13-14 |
| `uvc_qrscan_...` | QR / barcode read | zbar symbology detectors | **zbar** (`libzbar-dev`, board package; built natively on the board) | <1 ms | 9-10 |
| `uvc_ped_detect_...` | pedestrian boxes | OpenCV HOG + Dalal-Triggs SVM (`getDefaultPeopleDetector`, ships with OpenCV) | **OpenCV** (`libopencv-dev`, board package; built natively) | ~600 ms | 15-16 |
| `uvc_person_vis_...` | "is a person visible?" | MLPerf-Tiny `person_detect.tflite` (MobileNetV1 0.25, 96x96x1 int8) | **TensorFlow Lite Micro** (`vendor/tflm/lib/`, gcc-12 cross) | ~455-490 ms | 15-25 |
| `uvc_wake_word_...` | "yes/no" wake word | `audio_preprocessor_int8.tflite` + `micro_speech_quantized.tflite` (DS-CNN) | TFLM, two chained interpreters (signal custom ops for the FFT frontend) | ~2 verdicts/s | n/a (audio) |

## Board features used

- **SPI1 + GPIO** (DC=PA1/CON4.11, RES=PA6/CON4.7): MD169 panel,
  18-bit RGB666 pixel format (COLMOD 0x06) — see
  `app/uvc_object_detect_.../README.md` history and the panel notes;
  apps keep an RGB888 shadow buffer so OSD/boxes keep full precision.
- **UVC camera** (V4L2 MMAP, YUYV 640x480 forced; MJPEG path via
  vendored libjpeg.so.8 where the camera lacks YUYV). Any standard
  UVC camera works — verified with a 32e6:9228 module and a Logitech
  C920. High-speed USB port required for the full format list.
- **H3 audio codec mic** (CON5) via ALSA 16 kHz mono: rec_wav and
  wake_word. The codec capture route ships DISABLED — apps enable it
  at startup (`amixer -c 0 sset 'Mic1',0 100%,100% cap`), otherwise
  snd_pcm_readi fails with EIO / delivers zeros.
- **NEON**: -march=armv7-a+neon-vfpv4 everywhere; libfacedetection,
  NCNN and TFLM kernels make direct use of it.

## Provenance / licenses

| Vendor dir | Upstream | License |
|---|---|---|
| `uvc_face_detect_.../` (sources compiled in) | github.com/ShiqiYu/libfacedetection | 3-clause BSD |
| `uvc_face_pico_.../vendor/pico` + embedded cascade | github.com/nenadmarkus/pico | MIT |
| `uvc_object_detect_.../vendor/ncnn*` | github.com/Tencent/ncnn (+ dog-qiuqiu/Yolo-Fastest model) | BSD / Yolo-Fastest LICENSE |
| `uvc_qrscan_...` | zbar (board package libzbar-dev) | LGPL-2.1 |
| `uvc_ped_detect_...` | OpenCV HOG (board package libopencv-dev) | Apache-2.0 (OpenCV) |
| `uvc_person_vis_.../person_detect.tflite`, `uvc_wake_word_.../*.tflite` | github.com/tensorflow/tflite-micro (MLPerf-Tiny models) | Apache-2.0 |
| `uvc_wake_word_.../vendor/libasound.so.2` | board's own ALSA lib (copied at build time) | LGPL-2.1 |

## Runtime notes

- TFLM (person_vis / wake_word): apps load the `.tflite` from disk at
  startup and run a `MicroInterpreter` with an explicit op resolver.
  person_vis: Conv2D/DepthwiseConv2D/AveragePool2D/Reshape/Softmax
  (int8), 136 KB arena. wake_word: the preprocessor graph needs TFLM's
  **signal custom ops** (Window/FftAutoScale/Rfft/Energy/FilterBank*/
  PCAN) — 24 resolver slots cover both graphs.
- NCNN (object_detect): the raw darknet2ncnn weights layout HANGS
  `load_model` on armv7 (single-thread spin; x86 loads fine). The
  committed `yf-opt.*` model is pre-flattened with `ncnnoptimize` on
  x86 and loads everywhere. Also: Cortex-A7 has no FP16-storage —
  ncnn's `cast_fp16.h` guard must be neutralized when building.
- OpenCV and zbar apps build **natively on the board**
  (`make board` pushes sources and compiles there); the others are
  cross-built in WSL (`make`, `make push`).

## Tuning quick reference

- object_detect: `-T <score%>` threshold (default 60), `-C <classes>`
  allow-list (comma-separated COCO names, `_` = space). The tiny model
  throws confident false positives (e.g. "bicycle") on desk scenes —
  use the allow-list.
- face_pico: quality threshold and search range in `det_thread`
  (scalefactor 1.1, size 32-240, quality >= 3). Frontal faces only.
- wake_word: `-d plughw:1,0` selects the webcam mic (cleaner than the
  board codec mic — measured lower clipping, no rail whine).
