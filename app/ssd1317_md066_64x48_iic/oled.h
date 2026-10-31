/*
 * SSD1317 "MD066" 0.66" 64x48 mono OLED, I2C - driver logic ported from the
 * AT32F425 reference (f425-start bare/ssd1317_md066_64x48_iic), transport
 * layer (i2c-dev + GPIO uAPI) successively ported through MP157 DK1,
 * PocketBeagle, rk3506-mini, now the Orange Pi Zero (this header shows the
 * current board's wiring):
 *
 *   SCL  -> CON4.5 (TWI0-SCK / PA11, /dev/i2c-0)
 *   SDA  -> CON4.3 (TWI0-SDA / PA12)
 *   RES  -> CON4.7 (PWM1 / PA6, gpiochip0 line 6, active low)
 *   VCC  -> CON4.1 (3.3 V), GND -> CON4.6
 *
 * module I2C address: 7-bit 0x3C (0x78 in 8-bit write notation), ~100 kHz
 */
#ifndef __OLED_H
#define __OLED_H

#include <signal.h>
#include <stdint.h>

/* shared stop flag: cleared by the signal handler in main(), polled by the
 * driver so Ctrl-C takes effect immediately (mid-demo, mid-sleep) */
extern volatile sig_atomic_t oled_running;

#define SIZE 8
#define XLevelL   0x00
#define XLevelH   0x10
#define Max_Column 64
#define Max_Row   48
#define Brightness 0xFF
#define X_WIDTH   64
#define Y_WIDTH   48

#define OLED_CMD  0  /* command byte */
#define OLED_DATA 1  /* data byte */

extern const unsigned char F6x8[][6];
extern const unsigned char F8X16[];

void OLED_WR_Byte(unsigned dat, unsigned cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Init(int res_fd);
void OLED_Clear(void);
void OLED_Fill(unsigned char data);
void OLED_Invert(unsigned char on);
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char chr,
                   unsigned char Char_Size);
void OLED_ShowNum(unsigned char x, unsigned char y, uint32_t num,
                  unsigned char len, unsigned char size2);
void OLED_ShowString(unsigned char x, unsigned char y,
                     const unsigned char *p, unsigned char Char_Size);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_Demo(void);
void fill_picture(unsigned char fill_Data);
void Write_IIC_Command(unsigned char IIC_Command);
void Write_IIC_Data(unsigned char IIC_Data);
void Write_IIC_Byte_Buf(uint8_t slave_addr, uint8_t *buf, uint16_t size);
void oled_set_i2c_fd(int fd);
void oled_res_set(int res_fd, int on);
void oled_delay_ms(long ms);

#endif
