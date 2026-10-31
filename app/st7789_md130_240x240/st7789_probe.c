/*
  st7789_probe.c - isolate which operations the MD130 panel executes
  after init on the OPi Zero. Run it, watch the screen, compare with the
  console steps.

  Expected (if all works):
    step1: cyan screen  (3 s)
    step2: red screen   (3 s)
    step3: green screen (3 s)
    step4: display OFF  (dark, 2 s) -> display ON (2 s)
    step5: re-init + blue screen (3 s)
*/
#include <stdio.h>
#include <unistd.h>

#include "lcd.h"
#include "spi_hw.h"

static void wr_cmd(unsigned char c)
{
    spi_dc_set(0);
    spi_write_byte(c);
    spi_dc_set(1);
}

int main(void)
{
    spi_res_open(6);   /* PA6 = CON4.7 */
    spi_dc_open(1);    /* PA1 = CON4.11 */
    if (spi_open(8 * 1000 * 1000) != 0)
        return 1;

    printf("step0: LCD_Init (watch: screen should light up)\n");
    LCD_Init();
    sleep(1);

    printf("step1: DispColor CYAN (3 s)\n");
    DispColor(LCD_CYAN);
    sleep(3);

    printf("step2: DispColor RED (3 s)\n");
    DispColor(LCD_RED);
    sleep(3);

    printf("step3: DispColor GREEN (3 s)\n");
    DispColor(LCD_GREEN);
    sleep(3);

    printf("step4: display OFF (0x28) 2 s -> ON (0x29) 2 s\n");
    wr_cmd(0x28);
    sleep(2);
    wr_cmd(0x29);
    sleep(2);

    printf("step5: re-init + DispColor BLUE (3 s)\n");
    LCD_Init();
    DispColor(LCD_BLUE);
    sleep(3);

    printf("done\n");
    return 0;
}
