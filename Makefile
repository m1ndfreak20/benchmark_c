CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra -pedantic -std=c11
LDFLAGS = 

SRC_DIR = src
BIN_DIR = bin
INC_DIR = include

TARGET_DATETIME = $(BIN_DIR)/benchmark
TARGET_DICT = $(BIN_DIR)/benchmark_dict
TARGET_CONSOLE = $(BIN_DIR)/benchmark_console
TARGET_DICT_EXAMPLE = $(BIN_DIR)/dict_example
TARGET_DICT_GENERIC = $(BIN_DIR)/benchmark_dict_generic

.PHONY: all clean datetime dict console dict-example dict-generic run run-dict run-console run-dict-example run-dict-generic

all: datetime dict console dict-example dict-generic

datetime: $(TARGET_DATETIME)

dict: $(TARGET_DICT)

console: $(TARGET_CONSOLE)

dict-example: $(TARGET_DICT_EXAMPLE)

dict-generic: $(TARGET_DICT_GENERIC)

$(TARGET_DATETIME): $(SRC_DIR)/benchmark.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TARGET_DICT): $(SRC_DIR)/benchmark_dict.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TARGET_CONSOLE): $(SRC_DIR)/benchmark_console.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(TARGET_DICT_EXAMPLE): $(SRC_DIR)/dict_example.c $(INC_DIR)/dict.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $@ $(SRC_DIR)/dict_example.c $(LDFLAGS)

$(TARGET_DICT_GENERIC): $(SRC_DIR)/benchmark_dict_generic.c $(INC_DIR)/dict.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $@ $(SRC_DIR)/benchmark_dict_generic.c $(LDFLAGS)

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

run-dict-example: $(TARGET_DICT_EXAMPLE)
	./$(TARGET_DICT_EXAMPLE)

run-dict-generic: $(TARGET_DICT_GENERIC)
	./$(TARGET_DICT_GENERIC)
