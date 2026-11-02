TARGET     := lcm_test
CROSS      ?= arm-linux-gnueabihf-
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard
LDLIBS     := -lm
BOARD      ?= opz
BOARD_DIR  ?= st7365_md350_320x480/lcm_test
SOURCES    := main.c lcd.c spi_hw.c lcd.h spi_hw.h logo64.h ref/lcd_fonts.c ref/lcd_fonts.h ref/lcd_font_1608.c ref/lcd_font_1608.h Makefile README.md

all: $(TARGET)

$(TARGET): main.c lcd.c spi_hw.c lcd.h spi_hw.h logo64.h ref/lcd_fonts.c ref/lcd_fonts.h ref/lcd_font_1608.c ref/lcd_font_1608.h
	$(CROSS)gcc $(CROSS_FLAGS) -o $@ main.c lcd.c spi_hw.c ref/lcd_fonts.c ref/lcd_font_1608.c -lm

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az $(TARGET) $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

run:
	ssh $(BOARD) "cd $(BOARD_DIR) && ./$(TARGET)"

clean:
	rm -f $(TARGET)

.PHONY: all push run clean
