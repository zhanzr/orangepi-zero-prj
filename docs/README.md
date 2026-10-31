# Docs

Setup and design notes for the Orange Pi Zero project.

| File | Content |
|---|---|
| `con4-header.md` | CON4 26-pin header: full pinout with SoC pin names (schematic + vendor table), I2C/SPI/GPIO usage notes, free pins, permission setup |
| `st7789_md130_issue.md` | MD130 (240x240) stuck-after-first-frame issue: symptoms, probe results, everything ruled out, remaining suspects, isolation plan |

Vendor manuals (PDFs, schematics) are kept in
`D:\board_database\orangepi-zero` and are not part of this repository.
The connector tables from `D:\board_database\orangepi-zero\README.md`
are mirrored (expanded) in `con4-header.md`.

| Vendor file | Content |
|---|---|
| `Allwinner_H2+_Datasheet_V1.2.pdf` | SoC datasheet (register-level detail) |
| `OrangePi_zero_schematic_v1.5.pdf` | Board schematic (26-pin header nets, LED wiring, PHY/TF/WiFi) |
| `OrangePi_Zero_H2+_用户手册_v3.2.pdf` | Vendor user manual (CN): header pinouts, image flashing, WiringOP examples |
| `官方工具/` | Vendor flashing tools (PhoenixCard etc.) |
| `board_images/` (mirrored here) | Board photos / connector close-ups |

## Armbian notes

- Image: Armbian community 26.11.0-trunk.108, `BOARD=orangepizero`,
  `BOARDFAMILY=sun8i` (see `/etc/armbian-release` on the board).
- `orangepi-build` (github.com/orangepi-xunlong/orangepi-build) is only
  needed to rebuild the OS image / kernel / u-boot - application
  development uses the plain armhf cross compiler.
- DT overlays for the 26-pin header (i2c/spi/uart/...): `sudo armbian-config`
  -> System -> Hardware, or edit `/boot/armbianEnv.txt` (`overlays=` line),
  then reboot. Overlay sources: `/usr/share/doc/linux-u-boot-*/` and the
  Armbian `user-overlays` mechanism.
