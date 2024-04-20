#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

// SH1106 OLED display width and height
#define SH1106_WIDTH  128
#define SH1106_HEIGHT 64

// Function declarations
void init_display(int fd);
void write_command(int fd, unsigned char cmd);
void write_data(int fd, unsigned char *data, int size);

int main() {
    int fd;
    char *dev = "/dev/i2c-1";

    // Open the I2C device
    if ((fd = open(dev, O_RDWR)) < 0) {
        printf("Failed to open the I2C device.\n");
        exit(1);
    }

    // Set the OLED display address
    if (ioctl(fd, I2C_SLAVE, 0x3C) < 0) {
        printf("Failed to set I2C slave address.\n");
        close(fd);
        exit(1);
    }

    // Initialize the display
    init_display(fd);

    // Display a simple stripe pattern 
    unsigned char line[SH1106_WIDTH];
    for (int i = 0; i < SH1106_WIDTH; i++) {
        line[i] = (i % 2) == 0 ? 0xFF : 0x00;
    }

    // Write pattern to the display
    for (int i = 0; i < SH1106_HEIGHT / 8; i++) {
        write_command(fd, 0xB0 + i); // Set page address
        write_command(fd, 0x00); // Set low column address
        write_command(fd, 0x10); // Set high column address
        write_data(fd, line, SH1106_WIDTH);
    }

    // Close the I2C device
    close(fd);
    return 0;
}

void init_display(int fd) {
    // Initialization sequence for SH1106
    write_command(fd, 0xAE); // Display off
    write_command(fd, 0xD5); // Set display clock divide ratio/oscillator frequency
    write_command(fd, 0x80); // Set divide ratio
    write_command(fd, 0xA8); // Set multiplex ratio
    write_command(fd, 0x3F); // 1/64 duty
    write_command(fd, 0xD3); // Set display offset
    write_command(fd, 0x00); // No offset
    write_command(fd, 0x40); // Set start line at 0
    write_command(fd, 0x8D); // Charge pump
    write_command(fd, 0x14); // Enable charge pump
    write_command(fd, 0x20); // Memory addressing mode
    write_command(fd, 0x00); // Horizontal addressing mode
    write_command(fd, 0xA1); // Segment remap
    write_command(fd, 0xC8); // COM output scan direction
    write_command(fd, 0xDA); // Set COM pins hardware configuration
    write_command(fd, 0x12);
    write_command(fd, 0x81); // Contrast control
    write_command(fd, 0xCF);
    write_command(fd, 0xD9); // Pre-charge period
    write_command(fd, 0xF1);
    write_command(fd, 0xDB); // Set VCOMH deselect level
    write_command(fd, 0x40);
    write_command(fd, 0xA4); // Entire display on, resume to RAM content display
    write_command(fd, 0xA6); // Set normal display
    write_command(fd, 0xAF); // Display ON
}

void write_command(int fd, unsigned char cmd) {
    unsigned char buffer[2];
    buffer[0] = 0x00;
    buffer[1] = cmd;
    if (write(fd, buffer, 2) != 2) {
        printf("Error writing to I2C device (write_command)\n");
    }
}

void write_data(int fd, unsigned char *data, int size) {
    unsigned char *buffer = (unsigned char *)malloc(size + 1);
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    buffer[0] = 0x40;
    for (int i = 0; i < size; i++) {
        buffer[i + 1] = data[i];
    }
    if (write(fd, buffer, size + 1) != size + 1) {
        printf("Error writing to I2C device (write_data)\n");
    }
    free(buffer);
}
