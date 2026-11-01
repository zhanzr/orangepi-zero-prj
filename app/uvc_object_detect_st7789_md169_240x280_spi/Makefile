TARGET     := object_detect
CROSS      ?= arm-linux-gnueabihf-
CXX        := $(CROSS)g++
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard -Iinclude -Ivendor -Ivendor/ncnn/include -Ivendor/ncnn/include/ncnn
BOARD      ?= opz
BOARD_DIR  ?= uvc_object_detect_st7789_md169_240x280_spi

all: $(TARGET)

$(TARGET): object_detect.c lcd.c spi_hw.c ref/lcd_fonts.c ref/lcd_font_1608.c yolo_shim.cpp yolo_shim.h vendor/ncnn/lib/libncnn.a
	$(CROSS)gcc $(CROSS_FLAGS) -c object_detect.c -o object_detect.o
	$(CROSS)gcc $(CROSS_FLAGS) -c lcd.c -o lcd.o
	$(CROSS)gcc $(CROSS_FLAGS) -c spi_hw.c -o spi_hw.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_fonts.c -o ref/lcd_fonts.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_font_1608.c -o ref/lcd_font_1608.o
	$(CXX) -O2 $(CROSS_FLAGS) -c yolo_shim.cpp -o yolo_shim.o
	$(CXX) $(CROSS_FLAGS) -o $@ object_detect.o lcd.o spi_hw.o \
		ref/lcd_fonts.o ref/lcd_font_1608.o yolo_shim.o \
		vendor/ncnn/lib/libncnn.a \
		-Lvendor -l:libjpeg.so.8 -lm -lpthread -fopenmp

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)/vendor/ncnn_model"
	scp -q $(TARGET) $(BOARD):$(BOARD_DIR)/
	scp -q vendor/ncnn_model/yolo-fastest-1.1.param vendor/ncnn_model/yolo-fastest-1.1.bin $(BOARD):$(BOARD_DIR)/vendor/ncnn_model/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

run:
	ssh $(BOARD) "cd $(BOARD_DIR) && ./$(TARGET)"

clean:
	rm -f $(TARGET) *.o ref/*.o

.PHONY: all push run clean
