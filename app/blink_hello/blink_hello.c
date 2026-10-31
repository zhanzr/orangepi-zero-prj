/*
 * blink_hello - Orange Pi Zero (H2+, Armbian)
 *
 * Alternately blinks the two on-board LEDs (kernel LED class API):
 *   /sys/class/leds/orangepi:green:pwr
 *   /sys/class/leds/orangepi:red:status
 * and samples the SoC die temperature (Allwinner THS, cpu_thermal
 * thermal zone) on every toggle.
 *
 * The previous LED triggers are saved on start and restored on exit
 * (including on SIGINT/SIGTERM). Needs root to write the LED sysfs
 * nodes: run via "make run" (ssh -t + sudo).
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLINK_MS 500

#define GREEN_LED "/sys/class/leds/orangepi:green:pwr"
#define RED_LED   "/sys/class/leds/orangepi:red:status"
#define TEMP_MDC  "/sys/class/thermal/thermal_zone0/temp"

static const char *led_paths[2] = { GREEN_LED, RED_LED };
static char saved_trigger[2][160];
static volatile sig_atomic_t quit;

static void on_signal(int sig)
{
	(void)sig;
	quit = 1;
}

static int write_sysfs(const char *path, const char *val)
{
	FILE *f = fopen(path, "w");

	if (!f) {
		fprintf(stderr, "ERR: cannot write %s (root?)\n", path);
		return -1;
	}
	fputs(val, f);
	fclose(f);
	return 0;
}

/* trigger file content: "list of triggers [current] ..." */
static void read_trigger(const char *led, char *buf, size_t len)
{
	char path[256];
	char line[512];
	char *open, *close;

	buf[0] = '\0';
	snprintf(path, sizeof(path), "%s/trigger", led);
	FILE *f = fopen(path, "r");
	if (!f)
		return;
	if (fgets(line, sizeof(line), f)) {
		open = strchr(line, '[');
		close = open ? strchr(open, ']') : NULL;
		if (open && close && close > open) {
			size_t n = (size_t)(close - open - 1);
			if (n >= len)
				n = len - 1;
			memcpy(buf, open + 1, n);
			buf[n] = '\0';
		}
	}
	fclose(f);
}

static int set_brightness(const char *led, int on)
{
	char path[256];

	snprintf(path, sizeof(path), "%s/brightness", led);
	return write_sysfs(path, on ? "1" : "0");
}

static int set_trigger_none(const char *led)
{
	char path[256];

	snprintf(path, sizeof(path), "%s/trigger", led);
	return write_sysfs(path, "none");
}

static int read_temp_mdeg(long *mdeg)
{
	FILE *f = fopen(TEMP_MDC, "r");

	if (!f) {
		fprintf(stderr, "ERR: cannot read " TEMP_MDC "\n");
		return -1;
	}
	if (fscanf(f, "%ld", mdeg) != 1) {
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}

static void msleep(int ms)
{
	struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000L };

	nanosleep(&ts, NULL);
}

int main(void)
{
	int i;

	for (i = 0; i < 2; i++) {
		read_trigger(led_paths[i], saved_trigger[i],
			     sizeof(saved_trigger[i]));
		if (set_trigger_none(led_paths[i]) < 0)
			return 1;
	}

	signal(SIGINT, on_signal);
	signal(SIGTERM, on_signal);

	printf("blinking %s <-> %s, %d ms per phase, Ctrl-C to stop\n",
	       GREEN_LED, RED_LED, BLINK_MS);

	int phase = 0;
	long mdeg = 0;

	while (!quit) {
		if (set_brightness(led_paths[phase], 1) < 0 ||
		    set_brightness(led_paths[!phase], 0) < 0)
			break;
		if (read_temp_mdeg(&mdeg) == 0)
			printf("%s: ON  %s: OFF  die temp: %ld.%ld C\n",
			       phase == 0 ? "green" : "red  ",
			       phase == 0 ? "red  " : "green",
			       mdeg / 1000, (mdeg % 1000) / 100);
		else
			printf("%s: ON  %s: OFF  die temp: read error\n",
			       phase == 0 ? "green" : "red  ",
			       phase == 0 ? "red  " : "green");
		fflush(stdout);

		for (i = 0; i < BLINK_MS / 50 && !quit; i++)
			msleep(50);
		phase = !phase;
	}

	set_brightness(led_paths[0], 0);
	set_brightness(led_paths[1], 0);
	for (i = 0; i < 2; i++) {
		if (saved_trigger[i][0] &&
		    strcmp(saved_trigger[i], "none") != 0) {
			char path[256];

			snprintf(path, sizeof(path), "%s/trigger",
				 led_paths[i]);
			write_sysfs(path, saved_trigger[i]);
		}
	}
	printf("\nstopped, LED triggers restored\n");
	return 0;
}
