# rec_wav

Record up to 30 s of audio to a WAV file (16-bit PCM) on the Orange Pi
Zero. `q` stops early (single keypress, no Enter); Ctrl-C also stops
gracefully and still writes a valid WAV.

## Audio input on this board

**PulseAudio gotcha (fixed on this board, 2026-11-01):** the stock image
runs PulseAudio, which grabs the capture device and starves direct ALSA
clients (`arecord`/this app hang with 0 frames while the subdevice shows
`RUNNING` owned by `pulseaudio`). Fixed by killing it and disabling
autostart: `pkill -x pulseaudio; echo "autospawn = no" >
~/.config/pulse/client.conf; systemctl --user mask pulseaudio.service
pulseaudio.socket`. If capture ever hangs again with a healthy
`RUNNING` state, check for PA first (`fuser -v /dev/snd/pcmC0D0c`).

The H2+ on-chip codec is enabled by the `analog-codec` DT overlay
(already added to `/boot/armbianEnv.txt`) - capture device `hw:0,0`
(`H3 Audio Codec`):

- **Microphone input goes to CON5** (13-pin header): pin 10
  `MIC-MBIAS`, pin 11 `MIC1P`, pin 12 `MIC1N` - an external electret
  mic breakout is required (there is no audio jack on this board).
- Line-out is on CON5 pins 7/8 (`LINEOUTR`/`LINEOUTL`) for playback
  (`aplay`).
- Alternatively any **USB sound card** in the host ports appears as
  card 1; select it with `-d hw:1,0`.

## Formats

WAV (PCM) is written natively. **MP3/OGG/FLAC** are produced by
transcoding the WAV with an encoder installed on the board (the codec
chip itself only does PCM):

```
sudo apt install lame           # mp3 (or: vorbis-tools for ogg, flac, ffmpeg)
./rec_wav -f mp3                # records WAV, then encodes; both kept
```

`-f` falls back to `ffmpeg` if the specific encoder is missing, and
tells you what to install if neither exists.

## Usage

```
rec_wav [-d device] [-r rate] [-c channels] [-t max_sec]
        [-f wav|mp3|ogg|flac] [-o out_base]
```

Defaults: `default` device, 16 kHz, mono, 30 s, WAV,
`rec_wav_<timestamp>` output name.

## Build

Links against the board's own `libasound.so.2` (vendored under
`vendor/`, gitignored; `make` re-fetches it via scp if missing). At
runtime the SONAME resolves from the board's own alsa-lib.

```
make push
./rec_wav              # 30 s, WAV
./rec_wav -t 5         # 5 s smoke test
./rec_wav -f mp3       # WAV + MP3
```

Playback check on the board (needs a speaker on CON5 line-out):
`aplay rec_wav_*.wav`.
