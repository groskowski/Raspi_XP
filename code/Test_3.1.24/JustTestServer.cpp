#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    // Set socket to allow reuse of local addresses
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    error("ERROR setting socket option");

    // Clear address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 18200; // Server port number

    // Setup the host_addr structure for use in bind call
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(portno); 

    // Bind the socket to the current IP address on port, portno
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

    // Listen on the socket for connections
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    int returnCode = system("/usr/bin/python3 /home/pi/Sensor-Client.py");

    // Accept a connection from a client
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) 
          error("ERROR on accept");

    bzero(buffer,256);
    
    // Read the message from the client
    n = read(newsockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);

    // Here, you could add your logic to handle the received message,
    // such as preparing it for the radio transmission payload

    close(newsockfd);
    close(sockfd);
    return 0; 
}
