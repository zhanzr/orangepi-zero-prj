# ssd1317_md066_64x48_iic

SSD1317 "MD066" 0.66" 64x48 mono OLED demo on I2C, ported from
`rk3506-dev/app/ssd1317_md066_64x48_iic` (Vanxoak rk3506-mini <- PocketBeagle
<- MP157 DK1 <- AT32F425 references). Driver logic (oled.c) unchanged; only
the wiring defines and the transport comments were adapted (i2c-dev +
GPIO char-device uAPI, no library dependencies).

## Wiring (CON4, 26-pin header)

| Module pin | CON4 pin | Signal (SoC) | Notes |
|---|---|---|---|
| SCL | 5 | TWI0-SCK (PA11) | I2C0, ~100 kHz |
| SDA | 3 | TWI0-SDA (PA12) | |
| RES | 7 | PA6 | active low; gpiochip0 line 6 |
| VCC | 1 | VCC3V3-EXT | 3.3 V |
| GND | 6 (or 9/14/20/25) | GND | |

Module I2C address: 7-bit `0x3C`. Gotcha (carried through all ports): the
module does not ACK while RES is floating/low, so the demo requests the
line as output-high before the first probe.

## Board preparation (already done on this board)

- `i2c0` DT overlay enabled (`overlays=usbhost2 usbhost3 i2c0` in
  `/boot/armbianEnv.txt`, reboot) -> `/dev/i2c-0`
- Non-root access: `dev` is member of the `i2c` and `gpio` groups; a udev
  rule (`tools/gpio_i2c.rules`, installed once by
  `tools/setup_gpio_i2c.sh`) keeps `/dev/gpiochip*` and `/dev/i2c-*`
  writable by those groups (re-run the script after a system reflash)

## Build and run

```
make push        # cross-build both binaries, scp to opz:~/ssd1317_md066_64x48_iic
make run         # demo: banner -> info -> inverted -> HSV-ish -> fps loops (30 s cap, Ctrl-C clean)
make run-logo    # lcm_logo: reference boot logo + scaled font showcase
```

Both binaries run as the plain `dev` user. `make run-demo` prints
`OLED not detected ... check wiring` until the module is physically
connected - that is the expected pre-wiring state.

## Troubleshooting (no ACK at 0x3C)

`make scan` holds RES high (the module NACKs everything while RES floats -
that gotcha again) and probes the whole bus. Verified good state: bus
idles high and scans clean with nothing attached, so an empty scan with
the module wired means the module is not on the bus electrically:

1. **Swap SDA and SCL** - the #1 cause; a swapped pair still produces a
   perfectly clean empty scan.
2. **Power**: 3.3 V on module VCC, GND common with the board (CON4.6/9/...).
   Measure 3.3 V across the module's VCC/GND with the module connected.
3. **Pin count**: CON4 is 2x13 - recount from the marked pin 1 (see the
   connector diagram in the vendor manual, `D:\board_database\orangepi-zero`).
4. **Contacts**: seat the dupont wires fully; try different jumpers.
