EXECBIN  = myproxy

SRC_DIR = src
BIN_DIR = bin

SERVER_SRC = $(wildcard $(SRC_DIR)/proxy.c)

SERVER_OBJ = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SERVER_SRC))

CC       = clang
CFLAGS   = -Wall -Wpedantic -Werror -Wextra
.PHONY: all clean format

SERVER_TARGET = $(BIN_DIR)/myproxy

all: $(BIN_DIR) $(SERVER_TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Rule to build the server executable
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

# Rule to build object files for server
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean rule
clean:
	rm -rf $(BIN_DIR)