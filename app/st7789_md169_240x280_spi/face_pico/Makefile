TARGET     := face_pico
CROSS      ?= arm-linux-gnueabihf-
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard -Iinclude -Ivendor
BOARD      ?= opz
BOARD_DIR  ?= st7789_md169_240x280_spi/face_pico

all: $(TARGET)

$(TARGET): face_pico.c lcd.c spi_hw.c ref/lcd_fonts.c ref/lcd_font_1608.c vendor/pico/picornt.c vendor/pico/picornt.h facefinder_embed.h
	$(CROSS)gcc $(CROSS_FLAGS) -c face_pico.c -o face_pico.o
	$(CROSS)gcc $(CROSS_FLAGS) -c lcd.c -o lcd.o
	$(CROSS)gcc $(CROSS_FLAGS) -c spi_hw.c -o spi_hw.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_fonts.c -o ref/lcd_fonts.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_font_1608.c -o ref/lcd_font_1608.o
	$(CROSS)gcc $(CROSS_FLAGS) -c vendor/pico/picornt.c -o vendor/pico/picornt.o
	$(CROSS)gcc $(CROSS_FLAGS) -o $@ face_pico.o lcd.o spi_hw.o \
		ref/lcd_fonts.o ref/lcd_font_1608.o vendor/pico/picornt.o \
		-Lvendor -l:libjpeg.so.8 -lm -lpthread

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az $(TARGET) $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

run:
	ssh $(BOARD) "cd $(BOARD_DIR) && ./$(TARGET)"

clean:
	rm -f $(TARGET) *.o ref/*.o vendor/pico/*.o

.PHONY: all push run clean
