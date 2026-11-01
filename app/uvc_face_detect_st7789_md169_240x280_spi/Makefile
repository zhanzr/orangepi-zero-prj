TARGET     := face_detect
CROSS      ?= arm-linux-gnueabihf-
CXX        := $(CROSS)g++
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard -Iinclude -Ivendor
BOARD      ?= opz
BOARD_DIR  ?= uvc_face_detect_st7789_md169_240x280_spi

DETOBJ := vendor/facedetect/facedetectcnn.o \
          vendor/facedetect/facedetectcnn-model.o \
          vendor/facedetect/facedetectcnn-data.o \
          det_shim.o

all: $(TARGET)

vendor/facedetect/%.o: vendor/facedetect/%.cpp
	$(CXX) -O3 $(CROSS_FLAGS) -D_ENABLE_NEON -c $< -o $@

det_shim.o: det_shim.cpp det_shim.h
	$(CXX) -O3 $(CROSS_FLAGS) -c $< -o $@

$(TARGET): face_detect.c lcd.c spi_hw.c ref/lcd_fonts.c ref/lcd_font_1608.c $(DETOBJ)
	$(CROSS)gcc $(CROSS_FLAGS) -c face_detect.c -o face_detect.o
	$(CROSS)gcc $(CROSS_FLAGS) -c lcd.c -o lcd.o
	$(CROSS)gcc $(CROSS_FLAGS) -c spi_hw.c -o spi_hw.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_fonts.c -o ref/lcd_fonts.o
	$(CROSS)gcc $(CROSS_FLAGS) -c ref/lcd_font_1608.c -o ref/lcd_font_1608.o
	$(CXX) $(CROSS_FLAGS) -o $@ face_detect.o lcd.o spi_hw.o \
		ref/lcd_fonts.o ref/lcd_font_1608.o $(DETOBJ) \
		-Lvendor -l:libjpeg.so.8 -lm -lpthread

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	scp -q $(TARGET) $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

run:
	ssh $(BOARD) "cd $(BOARD_DIR) && ./$(TARGET)"

clean:
	rm -f $(TARGET) *.o ref/*.o vendor/facedetect/*.o

.PHONY: all push run clean
