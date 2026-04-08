CC := gcc
PKG_CONFIG := pkg-config

CFLAGS := -Wall -Wextra -Wpedantic -std=c11 $(shell $(PKG_CONFIG) --cflags raylib)
LDFLAGS := $(shell $(PKG_CONFIG) --libs raylib) -lm

SRC_DIR := src
BIN_DIR := bin
TARGET := $(BIN_DIR)/visualizer
SOURCES := $(wildcard $(SRC_DIR)/*.c)

.PHONY: all build run clean

all: build

build: $(TARGET)

$(TARGET): $(SOURCES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
