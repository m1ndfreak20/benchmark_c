CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra -pedantic -std=c11
LDFLAGS = 

SRC_DIR = src
BIN_DIR = bin

TARGET = $(BIN_DIR)/benchmark

SRCS = $(wildcard $(SRC_DIR)/*.c)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

run: $(TARGET)
	./$(TARGET)
