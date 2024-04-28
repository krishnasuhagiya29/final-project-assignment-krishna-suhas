#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GPIO_PIN 4
#define MOTOR_SCRIPT_PATH "motor.sh"

int read_gpio(int pin) {
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
    int gpio_state = read_gpio(GPIO_PIN);

    if (gpio_state < 0) {
        printf("Error reading GPIO pin\n");
        return 1;
    }

    char command[100];

    if (gpio_state == 1) {
        snprintf(command, sizeof(command), "%s 20000", MOTOR_SCRIPT_PATH);
    } else {
        snprintf(command, sizeof(command), "%s 15000", MOTOR_SCRIPT_PATH);
    }

    system(command);

    return 0;
}
