TARGET     := whiteband_test
CROSS      ?= arm-linux-gnueabihf-
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard
BOARD      ?= opz
BOARD_DIR  ?= st7365_md350_320x480/whiteband_test

all: $(TARGET)

$(TARGET): whiteband_test.c lcd.c spi_hw.c ref/lcd_fonts.c ref/lcd_font_1608.c
	$(CROSS)gcc $(CROSS_FLAGS) -o $@ whiteband_test.c lcd.c spi_hw.c \
		ref/lcd_fonts.c ref/lcd_font_1608.c -lm

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az $(TARGET) $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

run:
	ssh $(BOARD) "cd $(BOARD_DIR) && ./$(TARGET)"

clean:
	rm -f $(TARGET)

.PHONY: all push run clean
