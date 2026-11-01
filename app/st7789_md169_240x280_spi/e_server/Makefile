# e_server reference backend (host-side).
#
# Targets:
#   make            build web_assets.h + e_server
#   make run        build and run on :8080
#   make clean
#
# The web bundle (web_assets.h) is regenerated whenever web/ or public/
# changes. Needs Python for build_web.py and a host C compiler (gcc).

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE -Iinclude -Ivendor
LDLIBS  ?= -lpthread -lm -Lvendor -l:libjpeg.so.8
EXT     :=

ifeq ($(OS),Windows_NT)
  LDLIBS += -lws2_32
  EXT    := .exe
endif

WEB_SRC   := web/index.html web/style.css web/app.js
PUB_SRC   := $(wildcard public/*)

all: e_server$(EXT)

web_assets.h: build_web.py $(WEB_SRC) $(PUB_SRC)
	python build_web.py

server.o: server.c web_assets.h
	$(CC) $(CFLAGS) -c server.c

lcd.o: lcd.c lcd.h
	$(CC) $(CFLAGS) -c lcd.c

spi_hw.o: spi_hw.c spi_hw.h
	$(CC) $(CFLAGS) -c spi_hw.c

lcd_font_1608.o: ref/lcd_font_1608.c ref/lcd_font_1608.h ref/lcd_fonts.h
	$(CC) $(CFLAGS) -c ref/lcd_font_1608.c

e_server$(EXT): server.o lcd.o spi_hw.o lcd_font_1608.o
	$(CC) $(CFLAGS) -o $@ server.o lcd.o spi_hw.o lcd_font_1608.o $(LDLIBS)

run: all
	./e_server$(EXT) 8080

clean:
	rm -f server.o lcd.o spi_hw.o lcd_font_1608.o e_server e_server.exe

BOARD ?= opz
BOARD_DIR ?= e_server

push: e_server$(EXT)
	ssh $(BOARD) "mkdir -p $(BOARD_DIR)"
	rsync -az e_server$(EXT) $(BOARD):$(BOARD_DIR)/

run-remote:
	ssh -t $(BOARD) "cd $(BOARD_DIR) && ./e_server 8080"

.PHONY: all run run-remote push clean
