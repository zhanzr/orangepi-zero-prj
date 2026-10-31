#!/bin/bash
# One-time setup on the Orange Pi Zero (run from WSL):
# make /dev/gpiochip* group-writable for group "gpio" (and keep /dev/i2c-*
# on group i2c) so user-space apps can drive GPIO/I2C without root.
# User "dev" is already a member of gpio/i2c/spi groups (Armbian default).
set -e
BOARD=opz

echo "== install udev rule =="
scp -q "$(dirname "$0")/gpio_i2c.rules" $BOARD:/tmp/gpio_i2c.rules
ssh $BOARD "echo pocket | sudo -S sh -c 'cat /tmp/gpio_i2c.rules > /etc/udev/rules.d/90-gpio-i2c-perm.rules && rm /tmp/gpio_i2c.rules'
echo pocket | sudo -S udevadm control --reload-rules 2>/dev/null"

echo "== apply to already-created nodes =="
ssh $BOARD "echo pocket | sudo -S sh -c 'chgrp -f gpio /dev/gpiochip*; chmod g+rw /dev/gpiochip*; chgrp -f i2c /dev/i2c-* 2>/dev/null; chmod g+rw /dev/i2c-* 2>/dev/null; chgrp -f spi /dev/spidev* 2>/dev/null; chmod g+rw /dev/spidev* 2>/dev/null' 2>/dev/null
ls -l /dev/gpiochip* /dev/i2c-* /dev/spidev* 2>/dev/null"

echo "== line states of interest (PA6 = RES cand., PL6 = VCC-IO-EN) =="
ssh $BOARD "gpioinfo gpiochip0 2>/dev/null | sed -n '1,3p;8,9p'; gpioinfo gpiochip1 2>/dev/null | sed -n '8p'"

echo "== done =="
