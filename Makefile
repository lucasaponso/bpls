# Compiler and flags
CC = gcc
CFLAGS = -pthread

# Target executable
TARGET = bin/bpls

# Source files
SRC = src/main.c src/pay_load_adv.c src/rx_thread.c src/queue.c src/socket.c src/crc.c

# Object files
OBJ = $(SRC:.c=.o)

# Default target: build the executable
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# Rule to compile the source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJ) $(TARGET)

# Rebuild everything
rebuild: clean all
