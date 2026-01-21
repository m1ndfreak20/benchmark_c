CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra -pedantic -std=c11
LDFLAGS = 

SRC_DIR = src
BIN_DIR = bin

TARGET_DATETIME = $(BIN_DIR)/benchmark
TARGET_DICT = $(BIN_DIR)/benchmark_dict

.PHONY: all clean datetime dict run run-dict

all: datetime dict

datetime: $(TARGET_DATETIME)

dict: $(TARGET_DICT)

$(TARGET_DATETIME): $(SRC_DIR)/benchmark.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TARGET_DICT): $(SRC_DIR)/benchmark_dict.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

run: $(TARGET_DATETIME)
	./$(TARGET_DATETIME)

run-dict: $(TARGET_DICT)
	./$(TARGET_DICT)
