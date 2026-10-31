#!/bin/bash
# Find the Orange Pi Zero on the local /24 and update the
# opz ssh alias in ~/.ssh/config (WSL cannot resolve the mDNS
# name orangepizero.local, so the numeric IP lives in the config).
set -u

SUBNET=${1:-}
if [ -z "$SUBNET" ]; then
	# derive from the default route interface address
	SUBNET=$(ip -4 route show default | awk '{print $3}' | cut -d. -f1-3)
fi
if [ -z "$SUBNET" ]; then
	echo "cannot determine subnet, pass one: $0 192.168.5" >&2
	exit 1
fi

echo "scanning $SUBNET.1-254 port 22 ..."
FOUND=""
for i in $(seq 1 254); do
	(timeout 0.4 bash -c "echo > /dev/tcp/$SUBNET.$i/22" 2>/dev/null && echo "$SUBNET.$i") &
done
wait > /tmp/.opzscan.$$

for ip in $(cat /tmp/.opzscan.$$); do
	host=$(ssh -o BatchMode=yes -o ConnectTimeout=3 \
		-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
		dev@$ip hostname 2>/dev/null)
	if [ "$host" = "orangepizero" ]; then
		echo "FOUND board at $ip"
		FOUND=$ip
		break
	fi
done
rm -f /tmp/.opzscan.$$

if [ -z "$FOUND" ]; then
	echo "board not found on $SUBNET.0/24" >&2
	exit 1
fi

# rewrite the Host block in ~/.ssh/config
awk -v ip="$FOUND" '
	/^Host opz$/ {
		skip = 1
		print "Host opz"
		print "    HostName " ip
		print "    User dev"
		print "    IdentityFile ~/.ssh/id_rsa_pocket"
		print "    StrictHostKeyChecking no"
		print "    UserKnownHostsFile /dev/null"
		print "    LogLevel ERROR"
		next
	}
	/^[ \t]/ { if (skip) next }
	{ skip = 0; print }
' ~/.ssh/config > ~/.ssh/config.new && mv ~/.ssh/config.new ~/.ssh/config
chmod 600 ~/.ssh/config

echo "ssh alias opz -> $FOUND"
ssh -o ConnectTimeout=5 opz hostname
