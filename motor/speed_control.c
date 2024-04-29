#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

    while (1) {
        gpio_state = read_sw_gpio(GPIO_PIN);

        if (gpio_state < 0) {
            printf("Error reading GPIO pin\n");
            return 1;
        }

        // Check if the GPIO state has changed
        if (gpio_state != last_gpio_state) {
            char command[100];

            if (gpio_state == 1) {
                // Start the motor with a high duty cycle if not already high
                printf("High Speed\n");
                snprintf(command, sizeof(command), "%s start 20000", MOTOR_SCRIPT_PATH);
                system(command);

                // Optionally adjust duty cycle to a high value
                snprintf(command, sizeof(command), "%s adjust 18000", MOTOR_SCRIPT_PATH);
                system(command);
            } else {
                // Start the motor with a low duty cycle if not already low
                printf("Low Speed\n");
                snprintf(command, sizeof(command), "%s start 10000", MOTOR_SCRIPT_PATH);
                system(command);

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
