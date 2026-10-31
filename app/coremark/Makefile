TARGETS  := coremark-arm coremark-thumb
CROSS    ?= arm-linux-gnueabihf-
BOARD    ?= opz
BOARD_DIR ?= coremark
ITERATIONS ?= 60000
SOURCES  := core_list_join.c core_main.c core_matrix.c core_state.c \
            core_util.c linux/core_portme.c linux/core_portme.h

FLAGS := -O3 -funroll-loops -flto \
         -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard \
         -I. -Ilinux \
         -DFLAGS_STR=\"\" \
         -DSEED_METHOD=SEED_VOLATILE -DMEM_METHOD=MEM_MALLOC \
         -DPERFORMANCE_RUN=1 -DITERATIONS=$(ITERATIONS)

all: $(TARGETS)

coremark-arm: $(SOURCES)
	$(CROSS)gcc $(FLAGS) -marm $(SOURCES) -o $@

coremark-thumb: $(SOURCES)
	$(CROSS)gcc $(FLAGS) -mthumb $(SOURCES) -o $@

push: all
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	scp -q $(TARGETS) $(BOARD):$(BOARD_DIR)/
	ssh $(BOARD) "cd $(BOARD_DIR) && chmod 755 $(TARGETS)"

run:
	ssh $(BOARD) "cd $(BOARD_DIR) && taskset -c 0 ./coremark-arm && taskset -c 0 ./coremark-thumb"

run-perf:
	ssh -t $(BOARD) "echo pocket | sudo -S sh -c 'echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor' 2>/dev/null && cd $(BOARD_DIR) && taskset -c 0 ./coremark-arm && taskset -c 0 ./coremark-thumb; echo pocket | sudo -S sh -c 'echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor' 2>/dev/null"

size: $(TARGETS)
	$(CROSS)size $(TARGETS)

clean:
	rm -f $(TARGETS)

.PHONY: all push run run-perf size clean
