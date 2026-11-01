# AGENT.md — guidance for AI agents working in this repo

Orange Pi Zero (H2+/H3, Armbian) application repo. Displays: MD169
ST7789V 240x280 SPI panel (primary), MD130/MD144/MD066 (earlier
experiments). Network: Ethernet + WiFi + USB. Audio: H3 codec (CON5
mic) + USB webcams with mics.

## Golden rules

1. **Never commit/push without the user's explicit approval.** The user
   verifies results on real hardware first. This rule has priority over
   everything else.
2. **Verify empirically before claiming anything.** Assertions without
   an on-board test have repeatedly been wrong (camera host-number
   probes, washed-color theories, "unresponsive" guesses).
3. **Use rsync for transfers, never scp.** `rsync -az <src> opz:<dir>/`
   — all app Makefiles already do this. scp fails silently on this
   board's flaky link; retry loops (3-4 tries, sleep 3) are the norm.
4. **Only one panel/camera app runs at a time.** The panel (SPI) and
   UVC camera are single-open resources. Stop the previous app before
   starting another; redeploy `e_server` when done testing.

## Board access

- `ssh opz` from WSL (alias, currently 192.168.5.226 — IP drifts after
  reboots; `tools/find_board.sh` rescans). User `dev`, sudo password
  `pocket` (`echo pocket | sudo -S <cmd>`).
- The board **spontaneously reboots** occasionally, more under heavy
  sustained load (long compiles, long inference runs). If ssh dies
  mid-command, wait for it and check `ping`.
- Complex commands must go through **script files**
  (`\\wsl.localhost\Ubuntu\tmp\*.sh`, run via
  `wsl.exe -e bash -c "bash /tmp/x.sh"`) — inline quoting gets mangled
  by the Windows PowerShell layer. Never pass `$`, nested quotes or
  heredocs through `wsl.exe -e bash -c "..."` directly.

## Layout

- `app/<lcm_group>/<app>/` — one directory group per display
  (`st7789_md169_240x280_spi/` holds nine apps; future LCMs get their
  own group). Non-display apps (`rec_wav`) stay top-level.
- `app/vendor/` — shared vendor trees (TFLM headers + lib).
- Per-app `vendor/` — model/toolchain files only used by that app.
- `docs/` — `con4-header.md` (verified pinout + erratum),
  `ai-ml-projects.md` (models/runtimes/licenses table).
- `tools/` — cross toolchain files, `find_board.sh` (IP rescan).

## Build & deploy

- Cross-build (most apps): `make` then `make push` from WSL — pushes
  via rsync into `~/<BOARD_DIR>/` on the board. Toolchains: gcc-15
  default, **gcc-12 cross for NCNN and TFLM libs**
  (`tools/armv7-toolchain-gcc12.cmake`, `CROSS ?= arm-linux-gnueabihf-`,
  `CXX := $(CROSS)g++-12` — beware: `?=` loses to make's built-in
  `CXX := g++`).
- qr_scan and ped_detect build **natively on the board**
  (`make board` = push sources, board-side `make`; needs
  build-essential + libzbar-dev / libopencv-dev + pkg-config
  `PKG_CONFIG_PATH=/usr/lib/arm-linux-gnueabihf/pkgconfig`,
  `-I/usr/include/opencv4`).
- Overwriting a running binary fails with **ETXTBSY** — `pkill -9 -x
  <app>` before `make push`.

## Panel conventions (MD169)

- Pixel format: **18-bit RGB666** (COLMOD 0x06) by default — this
  matches browser colors; 16-bit 565 is washed (the chip's internal
  565->666 LUT is undocumented and non-replicating). Apps keep dual
  buffers: `panel_fb` (RGB565, OSD + telemetry) and `panel_666`
  (8-bit comps); blit packs `& 0xFC` per byte, 3 bytes/pixel
  (datasheet 8.8.43).
- OSD: transparent font drawn into both buffers every frame
  (`fb_string_t`), 8x16 ASCII font.
- Res = PA6 gpio 6 (CON4.7), DC = PA1 gpio 1 (CON4.11); SPI1 mode 0,
  requested 100 MHz (driver clamps ~37.5).

## Camera conventions

- Any standard UVC camera works; apps force **YUYV 640x480**.
- `find_uvc()` MUST enumerate `/sys/class/video4linux`, check
  `device_caps`, filter `driver == "uvcvideo"`, and canonicalize the
  `device` symlink with **realpath()** before matching the controller
  base address (1c19000/1c1b400/1c1c400/1c1d400 -> uh-0..3). Do not
  hand-roll shorter versions — they break (this exact bug happened).
- High-speed USB port required; full-speed ports collapse the format
  list.

## Audio conventions

- H3 codec capture route ships **disabled** — apps enable it at
  startup (`amixer -c 0 sset 'Mic1',0 100%,100% cap`), else reads fail
  EIO or deliver zeros.
- The USB webcam mic (`-d plughw:1,0`) is cleaner than the codec mic
  and has no mixer dependency.
- ALSA cross-linking: vendored copy of the board's `libasound.so.2`
  (see rec_wav/wake_word Makefiles).

## Toolchain gotchas

- NCNN on armv7: raw darknet-converted weights hang `load_model`
  (single-thread spin) — always ship ncnnoptimize-flattened models;
  Cortex-A7 lacks FP16-storage — neutralize `__ARM_FP & 2` guards in
  ncnn arm headers; NCNN static libs from gcc-15 are fine, but older
  ncnn trees need per-TU -O1 for some inline asm.
- pico returns face **center** + size, not top-left — convert before
  drawing boxes.
- Always build+run the upstream project's own test on the board first
  (e.g. `person_detection_test`) before wiring an app around a new
  runtime.

## Commit discipline

- Batch commits after the user verifies; descriptive messages with
  measured numbers; document root causes (see git log for examples).
- Keep binaries out of git (`.gitignore` has per-app entries); models
  and vendored libs are committed.
