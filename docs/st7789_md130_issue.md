# ST7789 MD130 (240x240) - stuck after first frame on the OPi Zero

Status: **unresolved** - isolated to "panel stops executing everything
after the first fill post-init; only a fresh RES+init revives it".
The identical module + identical code run correctly on the rk3506-mini.

## Symptom

- `st7789_demo` (ST7789V 1.3" 240x240, hw SPI1, mode 3, 4-wire):
  the panel shows the first banner fill and then **never updates again**.
  Every subsequent screen write produces a **single visible "shake"** and
  the image stays static (last successful frame). No console-side errors:
  all phases log, SPI statistics show tens of thousands of messages,
  0 errors, 0 timeouts.
- Probe experiment (`st7789_probe.c` in the app dir, 2026-11-01):

| step | action | result |
|---|---|---|
| 0-1 | LCD_Init + fill cyan | **works** (screen cyan) |
| 2-3 | DispColor red / green | **no effect** |
| 4 | raw 0x28/0x29 display off/on | **no effect** |
| 5 | LCD_Init again + fill blue | **works** (screen blue) |

  -> the panel executes exactly one fill after each hardware-reset+
  init, then ignores *everything* (even raw commands) until re-init.

## Ruled out

| Hypothesis | Evidence against |
|---|---|
| Port code divergence | full diff vs `rk3506-dev/app/st7789_md130_240x240`: only device-path strings (`spidev1.0`), gpio numbers, CON4 texts, regenerated font/logo assets; `lcd.c`/`spi_hw.c` logic byte-identical |
| SPI signal integrity | identical behavior at 40 / 16 / 8 / **1 MHz**; the working first fill uses the same wires at the same speed |
| Power rail (CON4.1 vs .17) | user confirmed with schematic + meter that pins 1 and 17 are the same net; moving VCC made no difference |
| CS floating | module is the *same physical unit* that ran correctly on the rk3506-mini; its CS is hardwired to GND on-module |
| cpufreq / `vdd-cpux` regulator pulsing the rail | pinned governor to `performance` (zero transitions) - no change. (Note: the DT's `vdd-cpux-regulator` drives **PL6**, which per the schematic is `VCC-IO-EN` - the LCD rail's enable - and *was* caught toggling (`out lo` sightings) during probe load. Suspicious, but not the trigger here.) |
| PWM / BLK | BLK is static 3.3 V (full-on); the panel logic failure is independent of it |

## Remaining suspects

1. **sunxi SPI1 controller specifics in mode 3** (vs Rockchip SPI0 on the
   rk3506-mini): something about how transfers after the first one are
   clocked/framed that this ST7789 panel rejects. Mode 0 is not an option
   for this module (renders nothing - f411 reference finding).
2. **DC (PA1) / RES (PA6) GPIO timing or level specifics** on the H3
   (e.g. brief re-muxes, drive strength) - the probe's failure boundary
   (first fill OK, then deaf) is odd for pure wiring though.
3. Panel + controller interaction at the GRAM-window level (window set
   once after init, later CASET/RASET rejected) - would need a logic
   analyzer on SCLK/MOSI/DC/CS to resolve.

## Isolation result (2026-11-01)

`st7789_md169_240x280_spi` (ST7789 1.69" 240x280, **mode 0**, same
4-wire DC-gpio transport family, same CON4 pins) **runs fully on the
OPi Zero** - banner, all TEST_STAND screens, gradient, logo, fonts at
40 MHz. The board-side transport (sunxi SPI1, spidev, DC/RES chardev
GPIO, permissions) is therefore **exonerated**.

**Conclusion: SPI mode 3 on the sunxi SPI1 controller is what the MD130
panel rejects** (the Rockchip SPI0 on the rk3506-mini drives mode 3
cleanly; this module renders nothing on mode 0, so mode 3 is mandatory
for it). Any further progress needs either a logic analyzer on
SCLK/MOSI/DC in mode 3 to characterize what the sunxi controller emits
between/inside transfers, or a sunxi-specific mode-3 workaround.

Also observed during debugging (kept for the record): the DT's
`vdd-cpux-regulator` drives PL6 - which per the schematic is
`VCC-IO-EN`, the LCD rail's enable - and was caught toggling under
load. Pinning the governor to `performance` stopped the toggling but
did NOT change the MD130 behavior, so it is not the root cause of this
issue (it remains a board-health concern worth a proper DT fix).

## Isolation history / next steps (archive)

- `st7789_md169_240x280_spi` (ST7789 1.69" 240x280, **mode 0**, same
  4-wire DC-gpio transport family) ported and **verified working**
  (see isolation result above).
- Ultimate A/B: re-wire this exact module to the rk3506-mini and re-run
  its demo there today.
- Hardware: logic analyzer on SCLK/MOSI/DC during a failing write.

## Reproduce

```
cd ~/st7789_md130_240x240
./st7789_demo          # screen: banner once, then static
./st7789_probe         # cyan -> (red/green/off-on dead) -> blue after re-init
```
