/*
  whiteband_test - isolates the top-strip write path (rows 0..27) on the
  ST7365P 3.5" 320x480 (MD350).

  Cycles three phases over the strip only, 3 s each:
    0: black strip + white text "RGB565 100M uh-1"
    1: solid WHITE strip
    2: solid BLACK strip

  If all three phases show on the panel, the strip write path works and
  the problem is elsewhere (camera app flow). If the strip never
  changes, the row addressing / window setup is at fault.

  The rest of the panel (rows 28..479) is filled dark gray once at
  init and stays untouched.
*/
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "lcd.h"
#include "spi_hw.h"
#include "ref/lcd_fonts.h"
#include "ref/lcd_font_1608.h"

static void delay_ms(uint32_t ms) { usleep(ms * 1000); }

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    spi_res_open(6);   /* PA6 = CON4.7 */
    spi_dc_open(1);    /* PA1 = CON4.11 */
    if (spi_open(100 * 1000 * 1000) != 0)
        return 1;
    LCD_Init();
    LCD_SetAsciiFont(&ASCII_Font16);

    /* rest of the panel: dark gray, untouched afterwards */
    LCD_SetColor(0x2104);
    LCD_FillRect(0, 28, LCD_Width, LCD_Height - 28);

    for (int loop = 0;; loop++)
    {
        switch (loop % 3)
        {
        case 0:
            LCD_SetColor(LCD_BLACK);
            LCD_SetBackColor(LCD_BLACK);
            LCD_FillRect(0, 0, LCD_Width, 28);
            LCD_SetColor(LCD_WHITE);
            LCD_DisplayString(4, 4, "RGB565 100M uh-1");
            printf("phase 0: black strip + text\n");
            break;
        case 1:
            LCD_SetColor(LCD_WHITE);
            LCD_FillRect(0, 0, LCD_Width, 28);
            printf("phase 1: WHITE strip\n");
            break;
        case 2:
            LCD_SetColor(LCD_BLACK);
            LCD_FillRect(0, 0, LCD_Width, 28);
            printf("phase 2: BLACK strip\n");
            break;
        }
        delay_ms(3000);
    }
}
