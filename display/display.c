#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>

// SH1106 OLED display width and height
#define SH1106_WIDTH  128
#define SH1106_HEIGHT 64

// 5x7 font table (ASCII characters 32-127)
unsigned char font[96][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00},  // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00},  // !
    {0x00, 0x07, 0x00, 0x07, 0x00},  // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14},  // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},  // $
    {0x63, 0x13, 0x08, 0x64, 0x63},  // %
    {0x36, 0x49, 0x56, 0x20, 0x50},  // &
    {0x00, 0x08, 0x07, 0x03, 0x00},  // '
    {0x00, 0x1C, 0x22, 0x41, 0x00},  // (
    {0x00, 0x41, 0x22, 0x1C, 0x00},  // )
    {0x14, 0x08, 0x3E, 0x08, 0x14},  // *
    {0x08, 0x08, 0x3E, 0x08, 0x08},  // +
    {0x00, 0x50, 0x30, 0x00, 0x00},  // ,
    {0x08, 0x08, 0x08, 0x08, 0x08},  // -
    {0x00, 0x60, 0x60, 0x00, 0x00},  // .
    {0x20, 0x10, 0x08, 0x04, 0x02},  // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E},  // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00},  // 1
    {0x42, 0x61, 0x51, 0x49, 0x46},  // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31},  // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10},  // 4
    {0x27, 0x45, 0x45, 0x45, 0x39},  // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30},  // 6
    {0x01, 0x71, 0x09, 0x05, 0x03},  // 7
    {0x36, 0x49, 0x49, 0x49, 0x36},  // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E},  // 9
    {0x00, 0x36, 0x36, 0x00, 0x00},  // :
    {0x00, 0x56, 0x36, 0x00, 0x00},  // ;
    {0x08, 0x14, 0x22, 0x41, 0x00},  // <
    {0x14, 0x14, 0x14, 0x14, 0x14},  // =
    {0x00, 0x41, 0x22, 0x14, 0x08},  // >
    {0x02, 0x01, 0x51, 0x09, 0x06},  // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E},  // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E},  // A
    {0x7F, 0x49, 0x49, 0x49, 0x36},  // B
    {0x3E, 0x41, 0x41, 0x41, 0x22},  // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C},  // D
    {0x7F, 0x49, 0x49, 0x49, 0x41},  // E
    {0x7F, 0x09, 0x09, 0x09, 0x01},  // F
    {0x3E, 0x41, 0x41, 0x51, 0x32},  // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F},  // H
    {0x00, 0x41, 0x7F, 0x41, 0x00},  // I
    {0x20, 0x40, 0x41, 0x3F, 0x01},  // J
    {0x7F, 0x08, 0x14, 0x22, 0x41},  // K
    {0x7F, 0x40, 0x40, 0x40, 0x40},  // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},  // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F},  // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E},  // O
    {0x7F, 0x09, 0x09, 0x09, 0x06},  // P
    {0x3E, 0x41, 0x41, 0x61, 0x7E},  // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46},  // R
    {0x46, 0x49, 0x49, 0x49, 0x31},  // S
    {0x01, 0x01, 0x7F, 0x01, 0x01},  // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F},  // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F},  // V
    {0x3F, 0x40, 0x3C, 0x40, 0x3F},  // W
    {0x63, 0x14, 0x08, 0x14, 0x63},  // X
    {0x07, 0x08, 0x70, 0x08, 0x07},  // Y
    {0x71, 0x49, 0x45, 0x43, 0x71},  // Z
};

// Function declarations
void init_display(int fd);
void write_command(int fd, unsigned char cmd);
void write_data(int fd, unsigned char *data, int size);
void clear_display(int fd);
void print_text(int fd, int page, int col, const char *text);

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
    
    // Clear Display
    clear_display(fd);
    
    print_text(fd, 0, 0, "Speed Limit Assist"); 
    print_text(fd, 1, 0, "Client RPi");
    
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

void clear_display(int fd) {
  for (int i = 0; i < SH1106_HEIGHT / 8; i++) {
        write_command(fd, 0xB0 + i); // Set page address
        write_command(fd, 0x00); // Set low column address
        write_command(fd, 0x10); // Set high column address
        for (int i = 0; i < SH1106_WIDTH; i++) {
            write_data(fd, 0, 1);
        }
  
    }
}

void print_text(int fd, int page, int col, const char *text) {
    // Set the position to start printing
    write_command(fd, 0xB0 + page);  // Set page address
    write_command(fd, 0x00 | (col & 0x0F));  // Set low column address
    write_command(fd, 0x10 | ((col >> 4) & 0x0F));  // Set high column address

    // Loop through each character in the text
    for (int i = 0; i < strlen(text); i++) {
        // Get the font data for the character
        unsigned char *char_data = font[text[i] - 32];  // ASCII offset by 32

        // Write the font data to the display
        write_data(fd, char_data, 5);  // Write the 5-byte character data
    }
}