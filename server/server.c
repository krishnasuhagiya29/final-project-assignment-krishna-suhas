/*

Author: Krishna Suhagiya

*/


#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h> // For sleep function

#define PORT 5000

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, optval = 1;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    unsigned char speed;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <speed>\n", argv[0]);
        exit(1);
    }

    int input_speed = atoi(argv[1]);
    if (input_speed < 0 || input_speed > 255) {
        fprintf(stderr, "Speed must be a number between 0 and 255\n");
        exit(1);
    }
    speed = (unsigned char) input_speed;

    // Delay before starting a new instance
    sleep(1);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // allow rebinding to the same address by setting SO_REUSEADDR option
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        error("ERROR setting socket option");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);

    // Accept a connection
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    // Send the data provided as argument
	n = write(newsockfd, &speed, sizeof(speed));
    if (n < 0)
        error("ERROR writing to socket");

    // Close the connection
    close(newsockfd);
    close(sockfd); // Close the listening socket
    return 0;
}
