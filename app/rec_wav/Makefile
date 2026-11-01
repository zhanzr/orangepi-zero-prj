TARGET     := rec_wav
CROSS      ?= arm-linux-gnueabihf-
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard -Ivendor
BOARD      ?= opz
BOARD_DIR  ?= rec_wav
SOURCES    := rec_wav.c Makefile README.md

all: $(TARGET)

# links against the BOARD's libasound.so.2 (vendored copy; SONAME
# resolves from the board's own /usr/lib at runtime)
$(TARGET): rec_wav.c vendor/libasound.so.2 Makefile
	$(CROSS)gcc $(CROSS_FLAGS) -o $@ rec_wav.c -Lvendor -l:libasound.so.2

vendor/libasound.so.2:
	rsync -az $(BOARD):/usr/lib/arm-linux-gnueabihf/libasound.so.2.0.0 vendor/ && \
	ln -sf libasound.so.2.0.0 vendor/libasound.so.2

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az $(TARGET) $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

run:
	ssh -t $(BOARD) "cd $(BOARD_DIR) && ./$(TARGET)"

clean:
	rm -f $(TARGET)

.PHONY: all push run clean
