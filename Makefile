CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra -pedantic -std=c11
LDFLAGS = 

SRC_DIR = src
BIN_DIR = bin

TARGET_DATETIME = $(BIN_DIR)/benchmark
TARGET_DICT = $(BIN_DIR)/benchmark_dict
TARGET_CONSOLE = $(BIN_DIR)/benchmark_console

.PHONY: all clean datetime dict console run run-dict run-console

all: datetime dict console

datetime: $(TARGET_DATETIME)

dict: $(TARGET_DICT)

console: $(TARGET_CONSOLE)

$(TARGET_DATETIME): $(SRC_DIR)/benchmark.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TARGET_DICT): $(SRC_DIR)/benchmark_dict.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TARGET_CONSOLE): $(SRC_DIR)/benchmark_console.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

run: $(TARGET_DATETIME)
	./$(TARGET_DATETIME)

run-dict: $(TARGET_DICT)
	./$(TARGET_DICT)

run-console: $(TARGET_CONSOLE)
	./$(TARGET_CONSOLE)
