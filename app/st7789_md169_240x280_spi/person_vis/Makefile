# person_vis - "is a person visible?" via MLPerf-Tiny person detection
# (TFLM). Cross-built in WSL; links a vendored TFLM static lib
# (ncnn-style: built from tflite-micro 2024-era tree, gcc-12 cross).
TARGET     := person_vis
CROSS      ?= arm-linux-gnueabihf-
CXX        := $(CROSS)g++-12
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard \
               -Iinclude -I. -Iref \
               -Iref -I../../vendor/tflm/inc
BOARD      ?= opz
BOARD_DIR  ?= st7789_md169_240x280_spi/person_vis

all: $(TARGET)

$(TARGET): person_vis.cpp model_settings.cc model_settings.h lcd.c spi_hw.c ref/lcd_fonts.c ref/lcd_font_1608.c ../../vendor/tflm/lib/libtensorflow-microlite.a
	$(CROSS)gcc $(CROSS_FLAGS) -c lcd.c -o lcd.o
	$(CROSS)gcc $(CROSS_FLAGS) -c spi_hw.c -o spi_hw.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_fonts.c -o ref/lcd_fonts.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_font_1608.c -o ref/lcd_font_1608.o
	$(CXX) $(CROSS_FLAGS) -c model_settings.cc -o model_settings.o
	$(CXX) $(CROSS_FLAGS) -c person_vis.cpp -o person_vis.o
	$(CXX) $(CROSS_FLAGS) -o $@ person_vis.o model_settings.o lcd.o spi_hw.o \
		ref/lcd_fonts.o ref/lcd_font_1608.o ../../vendor/tflm/lib/libtensorflow-microlite.a \
		-lm -lpthread

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az $(TARGET) person_detect.tflite $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

run:
	ssh $(BOARD) "cd $(BOARD_DIR) && ./$(TARGET)"

clean:
	rm -f $(TARGET) *.o

.PHONY: all push run clean
