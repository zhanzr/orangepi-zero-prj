# blink_hello

Alternately blinks the two on-board LEDs via the kernel LED class API and
samples the SoC die temperature (Allwinner THS) on every toggle:

| LED | sysfs path |
|---|---|
| green (pwr) | `/sys/class/leds/orangepi:green:pwr` |
| red (status) | `/sys/class/leds/orangepi:red:status` |

500 ms per phase, Ctrl-C to stop - the previous LED triggers are restored
on exit.

Runs as the normal `dev` user: the board has a udev rule
(`/etc/udev/rules.d/90-leds-perm.rules`, source `tools/leds.rules`,
installed once by `tools/setup_led_group.sh` from WSL) that makes LED
`brightness`/`trigger` writable by group `leds` (the rule survives
reboots; re-run the setup script after a system reflash). Without the
rule the LED writes need root - prefix with `sudo` as a fallback.

```
make push run
```

Expected output:

```
green: ON  red  : OFF  die temp: 65.7 C
red  : ON  green: OFF  die temp: 65.6 C
...
```

Die temp source: `/sys/class/thermal/thermal_zone0` (`cpu_thermal`, hwmon
`cpu_thermal` = Allwinner THS die sensor), millidegrees C.
