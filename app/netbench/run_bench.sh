#!/bin/bash
# run_bench.sh - full 4-way TCP benchmark: {ethernet, wifi} x {host->board,
# board->host}. Starts the server on the board, runs the host client
# against each interface address.
set -u
BOARD=opz
SEC=${SEC:-10}

# current interface addresses from the board
read -r ETH WIFI <<< "$(ssh $BOARD "ip -4 -o addr show | awk '/end0|wld1/ {print \$4}' | cut -d/ -f1 | tr '\n' ' '")"
echo "ethernet = $ETH   wifi = $WIFI   (test duration: ${SEC}s each)"

# NOTE: this ssh is wrapped in a local timeout on purpose - sshd keeps
# the session open while the nohup'd server lives, so it "hangs"; rc=124
# is expected once the server banner has appeared.
timeout 8 ssh $BOARD "pkill -x netbench 2>/dev/null; cd netbench && nohup ./netbench -s > /tmp/netbench.log 2>&1 < /dev/null & sleep 0.5; head -1 /tmp/netbench.log; ss -tln | grep -q 9001 && echo LISTENING" || true

run() {
  echo "=== $1: netbench -H $2 -t $SEC $3"
  ./netbench-host -H "$2" -t "$SEC" $3
}

run "ethernet host->board" "$ETH"  ""
run "ethernet board->host" "$ETH"  "-r"
run "wifi     host->board" "$WIFI" ""
run "wifi     board->host" "$WIFI"  "-r"

ssh $BOARD "pkill -x netbench 2>/dev/null; echo '--- server view ---'; cat /tmp/netbench.log"
