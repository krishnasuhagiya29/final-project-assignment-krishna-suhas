# Compiler settings
CC ?= $(CROSS_COMPILE)gcc

# Add debugging information and enable compiler warnings
CFLAGS  = -g -Wall -Werror

LDFLAGS ?= -lpthread -lrt

# Set the target name
TARGET = server

.PHONY: default all clean

default: $(TARGET)

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o $(LDFLAGS)

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) -c $(TARGET).c

# Remove the aesdsocket application and all .o files
clean:
	$(RM) $(TARGET) *.o
