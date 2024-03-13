EXECBIN  = myproxy

SRC_DIR = src
BIN_DIR = bin

# Automatically include all .c files in SRC_DIR
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Convert .c filenames to .o filenames in the BIN_DIR
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

CC       = clang
CFLAGS   = -Wall -Wpedantic -Werror -Wextra -Isrc -I/opt/homebrew/opt/openssl@3/include
LDFLAGS  = -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -lpthread
.PHONY: all clean format

all: $(BIN_DIR) $(BIN_DIR)/$(EXECBIN)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Rule to build the server executable
$(BIN_DIR)/$(EXECBIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to build object files for server
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean rule
clean:
	rm -rf $(BIN_DIR)

# Format rule (optional, requires clang-format)
format:
	clang-format -i $(SRC_DIR)/*.{c,h}
