# netbench

TCP throughput benchmark across the board's two network interfaces:
Ethernet (`end0`, 100M) and WiFi (`wld1`, XR819 SDIO 802.11n 1x1).

The **server runs on the board**, the **client on the host** (WSL) -
connections are always initiated by the client, so both directions work
through WSL2's NAT; the measured direction is chosen per connection.

## Usage

```
make bench          # full 4-way run (10 s per direction, both interfaces)
```

or manually:

```
make                # cross-builds netbench (board) + netbench-host (client)
make push
ssh opz 'cd netbench && ./netbench -s'          # server on the board
./netbench-host -H <board-eth-ip> -t 10         # host -> board
./netbench-host -H <board-eth-ip> -r -t 10      # board -> host
./netbench-host -H <board-wifi-ip> -t 10
./netbench-host -H <board-wifi-ip> -r -t 10
```

`run_bench.sh` picks the current `end0`/`wld1` addresses automatically
(they move around - see the root README, `tools/find_board.sh`).

## Results - Orange Pi Zero vs this host (WSL2), 10 s TCP streams

Verified 2026-11-01 (board: end0 = 192.168.5.226, wld1 = 192.168.5.218):

| Direction | Ethernet (end0, 100M) | WiFi (wld1, XR819) |
|---|---|---|
| host -> board | **77.3 Mbit/s** | **1.5 Mbit/s** |
| board -> host | **62.9 Mbit/s** | **6.5 Mbit/s** |

Notes:

- Ethernet lands at 60-77 Mbit/s TCP depending on the run - below the
  ~94 Mbit/s wire ceiling mostly due to the WSL2 NAT/vswitch path on the
  host side; the board-side server view agrees within a few percent.
- WiFi is very slow even for an XR819 (1x1 802.11n typically manages
  10-25 Mbit/s TCP) - TX direction especially. Worth a retry closer to
  the AP / with a different AP before blaming the driver; the numbers
  above are as-measured.
- First-run ethernet numbers are lower (45 Mbit/s) - cold socket/ARP;
  `SEC=20 ./run_bench.sh` for more stable results.
