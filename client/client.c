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
#include "display.h"
#include "speed_control.h"

#define PORT 5000
#define SERVER_IP_ADDR "10.0.0.91"

bool st_kill_process = false;
int client_fd = -1;

void cleanup_on_exit();
void sig_handler(int signal_number);

// Signal handler for cleaning up and exiting the process
void sig_handler(int signal_number)
{
    st_kill_process = true;
    syslog(LOG_INFO, "SIGINT/ SIGTERM encountered: exiting the process...");
    cleanup_on_exit();
    exit(EXIT_SUCCESS);
}

// Cleanup function to close connections and exit gracefully
void cleanup_on_exit()
{
    if (client_fd >= 0) {
        close(client_fd);
    }
    closelog();
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    int display_fd;
    int fd;
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

    // Initialize display
    init_display(fd);

    // Clear display
    clear_display(fd);

    openlog(NULL, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER); // Open connection for syslogging

    // Set up signal handlers for graceful exit
    if ((signal(SIGINT, sig_handler) == SIG_ERR) || (signal(SIGTERM, sig_handler) == SIG_ERR))
    {
        syslog(LOG_ERR, "Signal handler error");
        cleanup_on_exit();
        exit(EXIT_FAILURE);
    }

    // Create a socket for the client
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        syslog(LOG_ERR, "Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, SERVER_IP_ADDR, &serv_addr.sin_addr) <= 0)
    {
        syslog(LOG_ERR, "Invalid address/Address not supported");
        cleanup_on_exit();
        exit(EXIT_FAILURE);
    }

    // Attempt to connect to the server
    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        syslog(LOG_ERR, "Connection failed");
        cleanup_on_exit();
        exit(EXIT_FAILURE);
    }

    while (!st_kill_process)
    {
        // Add the socket client logic here (e.g., read or write data)
    }

    cleanup_on_exit(); // Ensure proper cleanup before exiting
    return 0;
}
