# Compiler and CFlAGS selection 'CROSS_COMPILE' is used to set custom arm compiler
CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Werror -fPIC  # -fPIC is required for shared libraries

# Sets the source as display.c and object as display.o
SRC := display.c
OBJ := $(SRC:.c=.o)

# The target is now a shared library named display.so
TARGET := display.so

# This is executed by default and is redirected to compilation steps
all: $(TARGET)

# Link object files into a shared library
$(TARGET): $(OBJ)
	$(CC) -shared -o $@ $^

# Generates object when source is available
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Removes previously created objects and targets
clean:
	rm -f $(TARGET) $(OBJ)

# Doesn't execute if artifacts are already present
.PHONY: all clean
