TARGET      := netbench
TARGET_HOST := netbench-host
CROSS       ?= arm-linux-gnueabihf-
CROSS_FLAGS := -O2 -Wall -Wextra -march=armv7-a+neon-vfpv4 -mfloat-abi=hard
BOARD       ?= opz
BOARD_DIR   ?= netbench
SOURCES     := netbench.c Makefile README.md

all: $(TARGET) $(TARGET_HOST)

# board binary (server) + host binary (client) - same source
$(TARGET): netbench.c Makefile
	$(CROSS)gcc $(CROSS_FLAGS) -o $@ netbench.c

$(TARGET_HOST): netbench.c Makefile
	gcc -O2 -Wall -Wextra -o $@ netbench.c

push: $(TARGET)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az $(TARGET) $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGET)"

# full 4-way benchmark: start the server on the board, run the host
# client against both interfaces, both directions (uses the current IPs
# from the board's end0/wld1)
bench: all push
	@bash ./run_bench.sh

clean:
	rm -f $(TARGET) $(TARGET_HOST)

.PHONY: all push bench clean
