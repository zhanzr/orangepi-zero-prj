# Applications

User-space applications for the Orange Pi Zero (H2+, Armbian 26.11). Each
subdirectory is one self-contained project with its own `Makefile` and
`README.md`.

| Project | Description |
|---|---|
| `hello/` | Minimal hello world - proves the cross toolchain + deploy loop |

## Common workflow (per project, from the project directory)

Fast loop during development - cross-build on the host (WSL) and push only
the binary:

```
make        # arm-linux-gnueabihf-gcc
make push   # scp the binary to ~/<project>/ on the board
make run    # execute it there
```

Deploy sources + Makefile (then build on the board with its native gcc):

```
make deploy
make build-remote
```

Requirements: the `opz` ssh alias in `~/.ssh/config` (set up - see the
root README) and `arm-linux-gnueabihf-gcc` on PATH (WSL apt package
`gcc-arm-linux-gnueabihf`). `env.sh` at the repo root documents/exports
the variables; the Makefiles default to the same values and work without
sourcing it.
