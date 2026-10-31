/*
 * SSD1317 "MD066" 0.66" 64x48 mono OLED demo for the Orange Pi Zero
 * (Allwinner H2+, Armbian 26.11), ported from
 * rk3506-dev/app/ssd1317_md066_64x48_iic (Vanxoak rk3506-mini), which
 * itself came from pbeagle_prj (PocketBeagle) <- MP157 DK1 <- AT32F425.
 *
 * Same Linux user-space architecture - i2c-dev for the module, GPIO
 * char-device uAPI for RES - no library dependencies:
 *
 *   SCL  -> CON4.5  (TWI0-SCK / PA11)    /dev/i2c-0 (i2c0 overlay)
 *   SDA  -> CON4.3  (TWI0-SDA / PA12)
 *   RES  -> CON4.7  (PWM1 / PA6),        active low
 *          = gpiochip0 line 6 (linux gpio number 6)
 *   V33  -> CON4.1  (VCC3V3-EXT), GND -> CON4.6/9/14/20/25
 *
 * The i2c0 bus is dedicated on this image (no other devices probed),
 * i2c0 enabled via the sun8i-h3-i2c0 DT overlay (armbianEnv.txt).
 *
 * Module 7-bit I2C address 0x3C. Gotcha (carried through all ports):
 * the module does not ACK while RES is floating/low, so the line is
 * requested with an initial inactive (high) value before the probe.
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <linux/i2c-dev.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "oled.h"

#define OLED_ADDR       0x3C
#define I2C_DEV         "/dev/i2c-0"
#define RES_GPIO_CHIP   0       /* PIO (PA bank) */
#define RES_GPIO_OFFSET 6       /* PA6 -> linux gpio 6 */
#define MAX_SECONDS     30

static volatile sig_atomic_t running = 1;
volatile sig_atomic_t oled_running = 1;

static void stop_handler(int sig)
{
    (void)sig;
    running = 0;
    oled_running = 0;
}

/* request one line as output; initial value high (RES inactive) */
static int line_request_output(int chip_fd, unsigned int offset)
{
    struct gpio_v2_line_request req;

    memset(&req, 0, sizeof(req));
    req.offsets[0] = offset;
    req.num_lines = 1;
    req.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
    snprintf(req.consumer, sizeof(req.consumer), "ssd1317_demo");

    if (ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &req) < 0)
        return -1;
    {
        struct gpio_v2_line_values vals;

        memset(&vals, 0, sizeof(vals));
        vals.mask = 1;
        vals.bits = 1; /* RES inactive (high) */
        ioctl(req.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &vals);
    }
    return req.fd;
}

int main(void)
{
    int i2c_fd, res_chip, res_fd;
    char res_chip_path[32];

    signal(SIGINT, stop_handler);
    signal(SIGTERM, stop_handler);

    i2c_fd = open(I2C_DEV, O_RDWR);
    if (i2c_fd < 0)
    {
        fprintf(stderr, "open %s: %s\n", I2C_DEV, strerror(errno));
        return 1;
    }
    if (ioctl(i2c_fd, I2C_SLAVE, OLED_ADDR) < 0)
    {
        fprintf(stderr, "I2C_SLAVE: %s\n", strerror(errno));
        return 1;
    }

    snprintf(res_chip_path, sizeof(res_chip_path), "/dev/gpiochip%d",
             RES_GPIO_CHIP);
    res_chip = open(res_chip_path, O_RDWR | O_CLOEXEC);
    if (res_chip < 0)
    {
        fprintf(stderr, "open %s: %s\n", res_chip_path, strerror(errno));
        return 1;
    }
    res_fd = line_request_output(res_chip, RES_GPIO_OFFSET);
    if (res_fd < 0)
    {
        fprintf(stderr, "RES gpio%d.%d busy: %s\n",
                RES_GPIO_CHIP, RES_GPIO_OFFSET, strerror(errno));
        return 1;
    }
    printf("RES = CON4.7 / PA6 (gpiochip%d line %d, gpio %d) ok\n",
           RES_GPIO_CHIP, RES_GPIO_OFFSET,
           RES_GPIO_CHIP * 32 + RES_GPIO_OFFSET);

    oled_set_i2c_fd(i2c_fd);

    {
        uint8_t probe[2] = { 0x00, 0xAE };

        if (write(i2c_fd, probe, 2) != 2)
        {
            fprintf(stderr, "OLED not detected at 0x%02x on %s (%s) - "
                    "check wiring (SCL=CON4.5, SDA=CON4.3, RES=CON4.7, "
                    "V33=CON4.1, GND=CON4.6)\n",
                    OLED_ADDR, I2C_DEV, strerror(errno));
            return 1;
        }
    }

    printf("SSD1317 MD066 64x48 I2C demo start (%s @ 0x%02x)\n",
           I2C_DEV, OLED_ADDR);
    OLED_Init(res_fd);

    {
        struct timespec ts;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        long start_ms = ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;

        while (running)
        {
            OLED_Demo();
            if (!running)
                break;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            long now_ms = ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
            if (now_ms - start_ms > MAX_SECONDS * 1000L)
            {
                printf("%d s cap reached\n", MAX_SECONDS);
                break;
            }
            oled_delay_ms(200);
        }
    }

    OLED_Clear();
    OLED_ShowString(0, 2, (const unsigned char *)"bye", 16);
    printf("bye\n");

    close(res_fd);
    close(res_chip);
    close(i2c_fd);
    return 0;
}
