#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>
#include "display.h"
#include "speed_control.h"

#define GPIO_PIN 4
#define MOTOR_SCRIPT_PATH "motor.sh"

void init_sw_gpio4() {
    system("echo 4 > /sys/class/gpio/export");
    system("echo in > /sys/class/gpio/gpio4/direction");
}

int read_sw_gpio(int pin) {
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Failed to open GPIO pin");
        return -1;
    }
    int value = fgetc(file) - '0';
    fclose(file);
    return value;
}

int main() {
    init_sw_gpio4();
    int gpio_state = read_sw_gpio(GPIO_PIN);
    int last_gpio_state = -1; // To track changes in GPIO state
    
    int fd; // This should be the I2C file descriptor
    char *dev = "/dev/i2c-1";

    // Open the I2C device
    if ((fd = open(dev, O_RDWR)) < 0) {
        perror("Failed to open the I2C device");
        exit(EXIT_FAILURE);
    }

    // Set the OLED display address
    if (ioctl(fd, I2C_SLAVE, 0x3C) < 0) {
        perror("Failed to set I2C slave address");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Initialize and clear display (assuming these functions are defined correctly)
    init_display(fd);

    while (1) {
        gpio_state = read_sw_gpio(GPIO_PIN);

        if (gpio_state < 0) {
            printf("Error reading GPIO pin\n");
            return 1;
        }
        
        char command[100];
        snprintf(command, sizeof(command), "%s start 0", MOTOR_SCRIPT_PATH);
        system(command);

        // Check if the GPIO state has changed
        if (gpio_state != last_gpio_state) {

            if (gpio_state == 1) {
                // Start the motor with a high duty cycle if not already high
                printf("High Speed\n");
                clear_display(fd);
                print_text(fd, 0, 0, " SPEED LIMIT ASSIST"); 
                print_text(fd, 1, 0, " CLIENT RPI 3B");
                print_text(fd, 2, 0, " HIGH SPEED");

                // Optionally adjust duty cycle to a high value
                snprintf(command, sizeof(command), "%s adjust 18000", MOTOR_SCRIPT_PATH);
                system(command);
            } else {
                // Start the motor with a low duty cycle if not already low
                printf("Low Speed\n");
                clear_display(fd);
                print_text(fd, 0, 0, " SPEED LIMIT ASSIST"); 
                print_text(fd, 1, 0, " CLIENT RPI 3B");
                print_text(fd, 2, 0, " LOW SPEED");

                // Optionally adjust duty cycle to a lower value
                snprintf(command, sizeof(command), "%s adjust 8000", MOTOR_SCRIPT_PATH);
                system(command);
            }

            // Update last_gpio_state
            last_gpio_state = gpio_state;
        }

        // Sleep for a short time to prevent high CPU usage
        usleep(100000); // 100 milliseconds
    }

    return 0;
}
