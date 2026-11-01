/*
  rec_wav - record up to 30 s of audio on the Orange Pi Zero to a WAV
  file (16-bit PCM). Press 'q' to stop early; Ctrl-C also stops
  gracefully (still writes a valid WAV).

  Capture device: the H3 on-chip audio codec (enabled by the
  `analog-codec` DT overlay; mic connects to CON5 MIC-MBIAS/MIC1P/MIC1N)
  or any USB sound card. Select with -d (default "default").

  Optional transcoding after capture (if the encoder is installed on the
  board): -f mp3 (lame/ffmpeg), -f ogg (oggenc/ffmpeg), -f flac
  (flac/ffmpeg). The intermediate WAV is kept.

  Usage: rec_wav [-d device] [-r rate] [-c channels] [-t max_sec]
                 [-f wav|mp3|ogg|flac] [-o out_base]
*/
#include <alsa/asoundlib.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_DEV      "default"
#define DEFAULT_RATE     16000
#define DEFAULT_CH       1
#define DEFAULT_SEC      30
#define FRAMES_PER_READ  320    /* 20 ms at 16 kHz: keeps q-checks tight */

static volatile sig_atomic_t stop_req;

static void on_signal(int sig)
{
    (void)sig;
    stop_req = 1;
}

/* no SA_RESTART: an interrupted snd_pcm_readi must return so the loop
   can observe stop_req - with signal()'s default SA_RESTART a blocked
   readi would never return on SIGTERM/Ctrl-C */
static void install_handlers(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sa.sa_flags = 0;                /* explicitly no SA_RESTART */
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

/* single-key 'q' without Enter: disable ICANON/ECHO, non-blocking read */
static struct termios saved_tio;
static int tio_saved;

static void tio_restore(void)
{
    if (tio_saved)
        tcsetattr(0, TCSANOW, &saved_tio);
}

static void tio_single_key(void)
{
    struct termios t;

    if (tcgetattr(0, &saved_tio) != 0)
        return;
    tio_saved = 1;
    t = saved_tio;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &t);
}

static int stdin_has_key(void)
{
    unsigned char c;
    ssize_t n = read(0, &c, 1);

    if (n == 1 && (c == 'q' || c == 'Q'))
        return 1;
    return 0;
}

/* enumerate every capture-capable PCM device across all cards */
static void list_capture_devices(void)
{
    int card = -1;

    printf("capture devices (-d <dev> to choose):\n");
    while (snd_card_next(&card) >= 0 && card >= 0)
    {
        char name[32];
        snd_ctl_t *ctl = NULL;

        snprintf(name, sizeof(name), "hw:%d", card);
        if (snd_ctl_open(&ctl, name, 0) < 0)
            continue;
        snd_ctl_card_info_t *info;
        snd_ctl_card_info_alloca(&info);
        snd_ctl_card_info(ctl, info);
        const char *card_name = snd_ctl_card_info_get_name(info);

        int dev = -1;
        while (snd_ctl_pcm_next_device(ctl, &dev) >= 0 && dev >= 0)
        {
            snd_pcm_info_t *pinfo;
            snd_pcm_info_alloca(&pinfo);
            snd_pcm_info_set_device(pinfo, (unsigned)dev);
            snd_pcm_info_set_stream(pinfo, SND_PCM_STREAM_CAPTURE);
            if (snd_ctl_pcm_info(ctl, pinfo) < 0)
                continue;
            printf("  plughw:%d,%d  [%s]  (%d subdevice(s))\n",
                   card, dev, card_name,
                   snd_pcm_info_get_subdevices_count(pinfo));
        }
        snd_ctl_close(ctl);
    }
    printf("  default      [ALSA routing; on this board usually card 0]\n");
}

static void wav_write_header(FILE *f, uint32_t rate, uint16_t ch,
                             uint32_t data_bytes)
{
    uint32_t byte_rate = rate * ch * 2;
    uint16_t block_align = (uint16_t)(ch * 2);
    uint32_t riff_size = 36 + data_bytes;

    fseek(f, 0, SEEK_SET);
    fwrite("RIFF", 1, 4, f);
    fwrite(&riff_size, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f);
    uint32_t fmt_len = 16;
    uint16_t fmt_pcm = 1;
    fwrite(&fmt_len, 4, 1, f);
    fwrite(&fmt_pcm, 2, 1, f);
    fwrite(&ch, 2, 1, f);
    fwrite(&rate, 4, 1, f);
    fwrite(&byte_rate, 4, 1, f);
    fwrite(&block_align, 2, 1, f);
    uint16_t bits = 16;
    fwrite(&bits, 2, 1, f);
    fwrite("data", 1, 4, f);
    fwrite(&data_bytes, 4, 1, f);
    fflush(f);
}

static int have_tool(const char *t)
{
    char cmd[96];

    snprintf(cmd, sizeof(cmd), "command -v %s >/dev/null 2>&1", t);
    return system(cmd) == 0;
}

static int encode(const char *fmt, const char *wav, const char *out)
{
    char cmd[1024];
    int rc;

    if (strcmp(fmt, "mp3") == 0)
    {
        if (have_tool("lame"))
            snprintf(cmd, sizeof(cmd), "lame --quiet %s %s", wav, out);
        else if (have_tool("ffmpeg"))
            snprintf(cmd, sizeof(cmd),
                     "ffmpeg -y -loglevel error -i %s %s", wav, out);
        else
        {
            fprintf(stderr, "rec_wav: no lame/ffmpeg on the board - "
                    "sudo apt install lame\n");
            return -1;
        }
    }
    else if (strcmp(fmt, "ogg") == 0)
    {
        if (have_tool("oggenc"))
            snprintf(cmd, sizeof(cmd), "oggenc -Q -o %s %s", out, wav);
        else if (have_tool("ffmpeg"))
            snprintf(cmd, sizeof(cmd),
                     "ffmpeg -y -loglevel error -i %s %s", wav, out);
        else
        {
            fprintf(stderr, "rec_wav: no oggenc/ffmpeg on the board - "
                    "sudo apt install vorbis-tools\n");
            return -1;
        }
    }
    else if (strcmp(fmt, "flac") == 0)
    {
        if (have_tool("flac"))
            snprintf(cmd, sizeof(cmd), "flac -s -f -o %s %s", out, wav);
        else if (have_tool("ffmpeg"))
            snprintf(cmd, sizeof(cmd),
                     "ffmpeg -y -loglevel error -i %s %s", wav, out);
        else
        {
            fprintf(stderr, "rec_wav: no flac/ffmpeg on the board - "
                    "sudo apt install flac\n");
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "rec_wav: unsupported format %s\n", fmt);
        return -1;
    }

    rc = system(cmd);
    if (rc != 0)
        fprintf(stderr, "rec_wav: encode failed (%s)\n", cmd);
    return rc;
}

static void usage(void)
{
    printf("usage: rec_wav [-l] [-d device] [-r rate] [-c channels] "
           "[-t max_sec] [-f wav|mp3|ogg|flac] [-o out_base]\n"
           "  -l   list all capture devices\n");
}

int main(int argc, char **argv)
{
    const char *dev = DEFAULT_DEV;
    const char *fmt = "wav";
    char base[256] = "";   /* -o override or timestamp default below */
    char wav_path[288], out_path[288];
    unsigned rate = DEFAULT_RATE, channels = DEFAULT_CH;
    unsigned max_sec = DEFAULT_SEC;
    int arg_err = 0;

    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "-l") == 0)
        {
            list_capture_devices();
            return 0;
        }

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-' && argv[i][1] && !argv[i][2] && i + 1 < argc)
        {
            switch (argv[i][1])
            {
            case 'd': dev = argv[++i]; break;
            case 'r': rate = (unsigned)atoi(argv[++i]); break;
            case 'c': channels = (unsigned)atoi(argv[++i]); break;
            case 't': max_sec = (unsigned)atoi(argv[++i]); break;
            case 'f': fmt = argv[++i]; break;
            case 'o':
                snprintf(base, sizeof(base), "%s", argv[++i]);
                break;
            default:
                arg_err = 1;
            }
        }
        else
            arg_err = 1;

        if (arg_err)
        {
            usage();
            return 1;
        }
    }

    if (base[0] == '\0')
    {
        time_t now = time(NULL);
        struct tm tm;
        localtime_r(&now, &tm);
        strftime(base, sizeof(base), "audio_%Y%m%d_%H%M%S", &tm);
    }
    snprintf(wav_path, sizeof(wav_path), "%s.wav", base);
    snprintf(out_path, sizeof(out_path), "%s.%s", base, fmt);

    signal(SIGPIPE, SIG_IGN);
    install_handlers();
    tio_single_key();
    atexit(tio_restore);

    /* the H3 codec ships with the capture route disabled -> snd_pcm_readi
       fails with EIO forever; enable Mic1/Mic2 capture + gain first */
    if (system("amixer -c 0 sset 'Mic1',0 100%,100% cap > /dev/null 2>&1; "
               "amixer -c 0 sset 'Mic2',0 100%,100% cap > /dev/null 2>&1") == 0)
        printf("rec_wav: Mic1/Mic2 capture routes enabled\n");

    snd_pcm_t *pcm;
    int err = snd_pcm_open(&pcm, dev, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0)
    {
        fprintf(stderr, "rec_wav: cannot open capture device '%s': %s\n"
                        "  (cards on this board: see 'cat /proc/asound/cards')\n",
                dev, snd_strerror(err));
        return 1;
    }

    snd_pcm_hw_params_t *hw;
    snd_pcm_hw_params_malloc(&hw);
    snd_pcm_hw_params_any(pcm, hw);
    err = snd_pcm_hw_params_set_access(pcm, hw,
                                       SND_PCM_ACCESS_RW_INTERLEAVED);
    err |= snd_pcm_hw_params_set_format(pcm, hw, SND_PCM_FORMAT_S16_LE);
    err |= snd_pcm_hw_params_set_channels(pcm, hw, channels);
    unsigned real_rate = rate;
    err |= snd_pcm_hw_params_set_rate_near(pcm, hw, &real_rate, 0);
    {
        snd_pcm_uframes_t period = FRAMES_PER_READ;

        snd_pcm_hw_params_set_period_size_near(pcm, hw, &period, 0);
    }
    err |= snd_pcm_hw_params(pcm, hw);
    snd_pcm_hw_params_free(hw);
    if (err < 0)
    {
        fprintf(stderr, "rec_wav: cannot set hw params (%u Hz, %u ch): %s\n",
                rate, channels, snd_strerror(err));
        return 1;
    }
    err = snd_pcm_prepare(pcm);
    if (err < 0)
    {
        fprintf(stderr, "rec_wav: prepare: %s\n", snd_strerror(err));
        return 1;
    }

    FILE *f = fopen(wav_path, "wb");
    if (!f)
    {
        perror("rec_wav: open output");
        return 1;
    }
    wav_write_header(f, real_rate, (uint16_t)channels, 0);

    unsigned max_frames = max_sec * real_rate;
    unsigned total = 0;
    int16_t buf[FRAMES_PER_READ * 2];
    int io_errs = 0;
    time_t t0 = time(NULL);

    printf("recording %s (%u Hz, %u ch, max %u s) - press 'q' to stop\n",
           wav_path, real_rate, channels, max_sec);

    while (!stop_req)
    {
        if (stdin_has_key())
        {
            printf("'q' pressed - stopping\n");
            break;
        }

        long n = snd_pcm_readi(pcm, buf, FRAMES_PER_READ);
        if (stop_req)
            break;
        if (n == -EPIPE)
        {
            fprintf(stderr, "rec_wav: overrun, recovering\n");
            snd_pcm_prepare(pcm);
            continue;
        }
        if (n == -EINTR)
            break;
        if (n < 0)
        {
            /* EIO here almost always means the capture route was off;
               try to recover a few times instead of dropping the file */
            if (snd_pcm_recover(pcm, (int)n, 1) == 0 && ++io_errs < 50)
            {
                fprintf(stderr, "rec_wav: read %s - retrying\n",
                        snd_strerror((int)n));
                continue;
            }
            fprintf(stderr, "rec_wav: read: %s\n", snd_strerror((int)n));
            break;
        }
        io_errs = 0;

        fwrite(buf, 2, (size_t)n * channels, f);
        total += (unsigned)n;

        time_t now = time(NULL);
        if (now != t0)
        {
            t0 = now;
            printf("\r%u s / %u s", total / real_rate, max_sec);
            fflush(stdout);
        }

        if (total >= max_frames)
            break;
    }

    tio_restore();
    uint32_t data_bytes = total * channels * 2;
    wav_write_header(f, real_rate, (uint16_t)channels, data_bytes);
    fclose(f);

    printf("\nrecorded %u frames (%u s, %u bytes) -> %s\n",
           total, total / real_rate, data_bytes, wav_path);

    if (strcmp(fmt, "wav") != 0)
    {
        printf("encoding to %s...\n", fmt);
        if (encode(fmt, wav_path, out_path) == 0)
            printf("written %s\n", out_path);
        else
        {
            printf("keeping %s\n", wav_path);
            return 1;
        }
    }

    snd_pcm_close(pcm);
    return 0;
}
