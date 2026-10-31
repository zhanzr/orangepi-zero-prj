#!/bin/bash
# One-time setup on the Orange Pi Zero (run from WSL):
# delegate LED control to a "leds" group so blink_hello can run
# without root. Installs tools/leds.rules on the board, applies the
# perms to the current LED nodes and adds user "dev" to the group.
set -e
BOARD=opz
DEV_USER=dev

echo "== create leds group + add $DEV_USER =="
ssh $BOARD "echo pocket | sudo -S groupadd -f leds 2>/dev/null; \
            echo pocket | sudo -S usermod -aG leds $DEV_USER 2>/dev/null"

echo "== install udev rule =="
scp -q "$(dirname "$0")/leds.rules" $BOARD:/tmp/leds.rules
ssh $BOARD "echo pocket | sudo -S sh -c 'cat /tmp/leds.rules > /etc/udev/rules.d/90-leds-perm.rules && rm /tmp/leds.rules'
echo pocket | sudo -S udevadm control --reload-rules 2>/dev/null"

echo "== apply perms to already-created LED nodes =="
ssh $BOARD "echo pocket | sudo -S sh -c 'chgrp -f leds /sys/class/leds/*/brightness /sys/class/leds/*/trigger; chmod g+w /sys/class/leds/*/brightness /sys/class/leds/*/trigger' 2>/dev/null
ls -l /sys/class/leds/orangepi:red:status/brightness /sys/class/leds/orangepi:red:status/trigger"

echo "== done - re-login (new ssh session) picks up group membership =="
