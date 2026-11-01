# wake_word - "yes/no" keyword spotting (TFLM + ALSA), cross-built in WSL.
TARGET     := wake_word
CROSS      := arm-linux-gnueabihf-
CXX        := $(CROSS)g++-12
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard \
               -Iinclude -I. -Iref -I../../vendor/tflm/inc
BOARD      ?= opz
BOARD_DIR  ?= st7789_md169_240x280_spi/wake_word

all: $(TARGET)

$(TARGET): wake_word.cpp lcd.c spi_hw.c ref/lcd_fonts.c ref/lcd_font_1608.c ../../vendor/tflm/lib/libtensorflow-microlite.a vendor/libasound.so.2
	$(CROSS)gcc $(CROSS_FLAGS) -c lcd.c -o lcd.o
	$(CROSS)gcc $(CROSS_FLAGS) -c spi_hw.c -o spi_hw.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_fonts.c -o ref/lcd_fonts.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_font_1608.c -o ref/lcd_font_1608.o
	$(CXX) $(CROSS_FLAGS) -c wake_word.cpp -o wake_word.o
	$(CXX) $(CROSS_FLAGS) -o $@ wake_word.o lcd.o spi_hw.o \
		ref/lcd_fonts.o ref/lcd_font_1608.o ../../vendor/tflm/lib/libtensorflow-microlite.a \
		-Lvendor -l:libasound.so.2 -lm -lpthread

vendor/libasound.so.2:
	rsync -az $(BOARD):/usr/lib/arm-linux-gnueabihf/libasound.so.2.0.0 vendor/ && \
	ln -sf libasound.so.2.0.0 vendor/libasound.so.2

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az $(TARGET) audio_preprocessor_int8.tflite micro_speech_quantized.tflite $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

run:
	ssh $(BOARD) "cd $(BOARD_DIR) && ./$(TARGET)"

clean:
	rm -f $(TARGET) *.o

.PHONY: all push run clean
