TARGET     := uvc_to_lcd
CROSS      ?= arm-linux-gnueabihf-
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard -Iinclude -Ivendor
LDLIBS     := -lm
BOARD      ?= opz
BOARD_DIR  ?= uvc_to_st7789_md169_240x280_spi
SOURCES    := uvc_to_lcd.c lcd.c spi_hw.c lcd.h spi_hw.h ref/lcd_fonts.c ref/lcd_fonts.h ref/lcd_font_1608.c ref/lcd_font_1608.h Makefile README.md

all: $(TARGET)

$(TARGET): uvc_to_lcd.c lcd.c spi_hw.c lcd.h spi_hw.h ref/lcd_fonts.c ref/lcd_fonts.h ref/lcd_font_1608.c ref/lcd_font_1608.h
	$(CROSS)gcc $(CROSS_FLAGS) -o $@ uvc_to_lcd.c lcd.c spi_hw.c ref/lcd_fonts.c ref/lcd_font_1608.c -Lvendor -l:libjpeg.so.8 -lm

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	scp -q $(TARGET) $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

run:
	ssh $(BOARD) "cd $(BOARD_DIR) && ./$(TARGET)"

clean:
	rm -f $(TARGET)

.PHONY: all push run clean
