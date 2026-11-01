TARGET    := hello
CROSS     ?= arm-linux-gnueabihf-
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard
BOARD     ?= opz
BOARD_DIR ?= $(TARGET)
SOURCES   := $(TARGET).c Makefile README.md

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CROSS)gcc $(CROSS_FLAGS) -o $@ $<

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az $(TARGET) $(BOARD):$(BOARD_DIR)/

run:
	ssh $(BOARD) "./$(BOARD_DIR)/$(TARGET)"

deploy:
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az $(SOURCES) $(BOARD):$(BOARD_DIR)/

build-remote:
	ssh $(BOARD) "cd $(BOARD_DIR) && make CC=gcc"

clean:
	rm -f $(TARGET)

.PHONY: all push run deploy build-remote clean
