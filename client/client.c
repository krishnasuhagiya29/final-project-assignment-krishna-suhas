#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdbool.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>       
#include <sys/ioctl.h>     
#include <linux/i2c-dev.h> 
#include <pthread.h>
#include <errno.h>
#include "display.h"
#include "speed_control.h"

#define PORT 5000
#define GPIO_PIN 4
#define MOTOR_SCRIPT_PATH "motor.sh"

bool st_kill_process = false;
int client_fd = -1;

void cleanup_on_exit();
void sig_handler(int signal_number);
int get_speed();
void connect_to_server(const char* server_ip);

int get_speed() {
    unsigned char speed = 0;
    ssize_t retval;

    // Attempt to read one byte from the socket
    retval = read(client_fd, &speed, sizeof(speed));
    if (retval < 0) {
        perror("Failed to read from socket");
        return -1; // Indicate error in reading
    }
    if (retval == 1) {
        syslog(LOG_INFO, "Speed received as %d", speed);
    }
    return speed; // Return the speed received
}

void cleanup_on_exit() {
    if (client_fd != -1) {
        close(client_fd);
    }
    closelog();
}

// Signal handler for closing the connection
void sig_handler(int signal_number) {
    st_kill_process = true;
    syslog(LOG_INFO, "SIGINT/SIGTERM encountered: exiting the process...");
    cleanup_on_exit();
    exit(EXIT_SUCCESS);
}

void connect_to_server(const char* server_ip) {
    struct sockaddr_in serv_addr;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(client_fd);
        client_fd = -1;
    }
}


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *server_ip = argv[1];
    
    struct sockaddr_in serv_addr;
    unsigned char speed;
    system("modprobe i2c-dev");
    
    int gpio_state;
    int last_gpio_state = 0; // To track changes in GPIO state
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
    clear_display(fd);
    print_text(fd, 0, 0, " SPEED LIMIT ASSIST"); 
    print_text(fd, 1, 0, " CLIENT RPI 3B");
    
    init_sw_gpio4();
    char command[100];
    snprintf(command, sizeof(command), "%s start 5000", MOTOR_SCRIPT_PATH);
    system(command);

    // Initialize syslog
    openlog(NULL, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER);

    if ((signal(SIGINT, sig_handler) == SIG_ERR) || (signal(SIGTERM, sig_handler) == SIG_ERR)) {
        syslog(LOG_ERR, "Signal handler error");
        cleanup_on_exit();
        exit(EXIT_FAILURE);
    }

    // Create a socket client
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Initiate a connection to socket
    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    while (!st_kill_process) {
        gpio_state = read_sw_gpio(GPIO_PIN);
        
        // Check if the GPIO state has changed
        if (gpio_state != last_gpio_state) {
            print_text(fd, 4, 0, " SWITCH TOGGLED");
            if (gpio_state == 1) {
                snprintf(command, sizeof(command), "%s adjust 18000", MOTOR_SCRIPT_PATH);
                system(command);
            } else {
                snprintf(command, sizeof(command), "%s adjust 8000", MOTOR_SCRIPT_PATH);
                system(command);
            }
            
            last_gpio_state = gpio_state;
        }

        if (gpio_state < 0) {
            printf("Error reading GPIO pin\n");
            return 1;
        }
    
        if (client_fd == -1) {
            printf("Attempting to reconnect...\n");
            connect_to_server(server_ip);
            if (client_fd == -1) {
                sleep(1); // Sleep for a second before trying again
                continue;
            }
        }
        
        // Get speed from the server
        speed = get_speed();
        if (speed <= 0) {
            close(client_fd);
            client_fd = -1;
            continue;
        }
        
        if ((speed == 60) && (gpio_state == 0)) {
            clear_display(fd);
            print_text(fd, 0, 0, " SPEED LIMIT ASSIST"); 
            print_text(fd, 1, 0, " CLIENT RPI 3B");
            print_text(fd, 2, 0, " SPEED MAINTAINED");
            print_text(fd, 3, 0, " 60");
        } else if ((speed == 80) && (gpio_state == 0)) {
            clear_display(fd);
            print_text(fd, 0, 0, " SPEED LIMIT ASSIST"); 
            print_text(fd, 1, 0, " WARNING");
            print_text(fd, 2, 0, " LOW SPEED: 60");
            print_text(fd, 3, 0, " 80 REQUIRED");
        } else if ((speed == 80) && (gpio_state == 1)) {
            clear_display(fd);
            print_text(fd, 0, 0, " SPEED LIMIT ASSIST"); 
            print_text(fd, 1, 0, " CLIENT RPI 3B");
            print_text(fd, 2, 0, " SPEED MAINTAINED");
            print_text(fd, 3, 0, " 80");
        } else if ((speed == 60) && (gpio_state == 1)) {
            clear_display(fd);
            print_text(fd, 0, 0, " SPEED LIMIT ASSIST"); 
            print_text(fd, 1, 0, " WARNING");
            print_text(fd, 2, 0, " HIGH SPEED: 80");
            print_text(fd, 3, 0, " 60 REQUIRED");
        } else if ((speed < 60) && (gpio_state == 0)) {
            clear_display(fd);
            print_text(fd, 0, 0, " SPEED LIMIT ASSIST"); 
            print_text(fd, 1, 0, " WARNING");
            print_text(fd, 2, 0, " HIGH SPEED: 60");
        } else if ((speed < 80) && (gpio_state == 1)) {
            clear_display(fd);
            print_text(fd, 0, 0, " SPEED LIMIT ASSIST"); 
            print_text(fd, 1, 0, " WARNING");
            print_text(fd, 2, 0, " HIGH SPEED: 80");
        }
        
        usleep(100000);
    }

    cleanup_on_exit();
    return 0;
}
