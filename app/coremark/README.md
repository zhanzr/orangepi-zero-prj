# coremark

EEMBC CoreMark 1.0 benchmark, ported from `rk3506-dev/app/coremark`
(itself from `pbeagle_prj/app/coremark` / an STM32MP157 reference).
Upstream CoreMark sources + the `linux` port (`linux/core_portme.*`,
POSIX timers, heap memory) are used unchanged; the port work is the
build configuration: Cortex-A7 target (H2+ = 4x Cortex-A7, same core as
the rk3506-mini) and this repository's board/transfer workflow. See
`LICENSE.md` (Apache 2.0).

Two instruction-set variants are built:

- `coremark-arm`   : compiled with `-marm`  (A32 instruction set)
- `coremark-thumb` : compiled with `-mthumb` (T32/Thumb-2 instruction set)

Optimization: `-O3 -funroll-loops -flto` on top of
`-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard`. Runs use
`PERFORMANCE_RUN=1`, `SEED_METHOD=SEED_VOLATILE`, `MEM_METHOD=MEM_MALLOC`
(heap), compile-time pinned `ITERATIONS` (default 60000, > 10 s runtime
for a valid score; the `linux` port's default `SEED_ARG` would ignore
`-DITERATIONS` and fall back to auto-calibration, hence `SEED_VOLATILE`).

## Build and run

```
make                 # or: make ITERATIONS=60000
make push run        # scp both binaries to opz:~/coremark and run both
make run-perf        # same but with the performance cpufreq governor
                     # (sudo; restores ondemand afterwards)
make size            # text/data/bss sizes
```

Runs are pinned with `taskset -c 0`.

**Clock note (differs from the rk3506-mini):** this Armbian image has
working DVFS - `ondemand` governor, 480/648/816/960/1008/1104/1200/1296
MHz table. Measured: `ondemand` boosts to 1.296 GHz fast enough that
scores are identical under both governors (see results) - `make run` is
fine; `make run-perf` exists for a fixed-clock run anyway.

## Results - Orange Pi Zero (4x Cortex-A7 @ 1.296 GHz max, single core)

Validated runs (CRC checked correct), 60000 iterations each,
GCC 15.2.0 (Ubuntu arm-linux-gnueabihf), Armbian 26.11, kernel
6.18.57-current-sunxi:

| Binary | Governor | CoreMark/s | CoreMark/MHz | text | data | bss |
|---|---|---|---|---|---|---|
| coremark-arm | ondemand | 3709.428130 | 2.86 | 25 999 | 428 | 56 |
| coremark-thumb | ondemand | 3792.667509 | 2.93 | 20 759 | 428 | 56 |
| coremark-arm | performance | 3708.969525 | 2.86 | - | - | - |
| coremark-thumb | performance | 3793.386862 | 2.93 | - | - | - |

Observations:

- Validated runs (CRC `0xbd59` correct), 60000 iterations, ~16 s each.
- Governor makes no measurable difference: `ondemand` boosts to the top
  OPP (1.296 GHz) within the first fractions of a second of the 16 s run
  - identical scores under both governors.
- Thumb-2 came out ~2.2% faster here (3793 vs 3709), opposite of the
  rk3506-mini (ARM edged Thumb-2 by 1.4% there) - consistent with the
  "both directions seen on A7, likely LTO/code-layout luck" note from
  that port. Thumb-2 remains ~20% denser (20.8 KB vs 26.0 KB text).
- Absolute per-clock efficiency (~2.86-2.93 CM/MHz) is ~25% below the
  rk3506-mini's 3.85-3.90 although both SoCs use the same Cortex-A7
  core. Candidate factors: SoC memory subsystem config (H2+ 32-bit DDR3),
  GCC 15.2 vs 12.4 codegen, kernel 6.18 vs 6.1 timing/syscall path
  (core_portme measures with `clock_gettime`). Not investigated further;
  scores are self-consistent across runs.
- Die temp during the runs: ~65-70 C (board idles warm), no throttling
  observed.
