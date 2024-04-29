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
#define SERVER_IP_ADDR "10.0.0.91"

bool st_kill_process = false;
int client_fd = -1;

void cleanup_on_exit();
void sig_handler(int signal_number);
int get_speed();

int get_speed() {
    unsigned char speed = 0; // Change to read only one byte
    ssize_t retval;

    // Attempt to read one byte from the socket
    retval = read(client_fd, &speed, sizeof(speed));
    if (retval < 0) {
        perror("Failed to read from socket");
        return -1; // Indicate error in reading
    } else if (retval == 0) {
        printf("Connection closed by server\n");
        return -1; // Server closed the connection
    }

    syslog(LOG_INFO, "Speed received as %d\n", speed);

    // Send Acknowledgement
    const char *ackMsg = "Received";
    retval = write(client_fd, ackMsg, strlen(ackMsg) + 1); // Ensure null terminator is included
    if (retval < 0) {
        perror("Failed to send acknowledgement");
        return -1;
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

int main() {
    struct sockaddr_in serv_addr;
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
    if (inet_pton(AF_INET, SERVER_IP_ADDR, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Initiate a connection to socket
    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    while (!st_kill_process) {
        // Get speed from the server
        if (get_speed() == -1) {
            syslog(LOG_ERR, "Failed to get speed from server");
            cleanup_on_exit();
            exit(EXIT_FAILURE);
        }
    }

    cleanup_on_exit();
    return 0;
}
