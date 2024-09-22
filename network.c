
// Responsibility: Handles all network-related operations,
// including setting up UDP sockets, sending messages,
// and receiving messages.

// Key Functions: setup socket, send message, receive message, etc..

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "io_handler.h"

int sockfd;
char *remote_ip;
int remote_port;

// Function to set up a UDP socket and bind it to a local port
int setup_socket(int local_port, char *hostname, int port)
{
    struct sockaddr_in local_addr; // Structure to hold local address information
    struct addrinfo hints, *result; // Variables for resolving hostname

    // Initialize remote address info
    remote_ip = NULL;
    remote_port = port;

    // Create the socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Failed to create socket");
        return -1;
    }

    // Zero out the structure
    memset(&local_addr, 0, sizeof(local_addr));

    local_addr.sin_family = AF_INET;                // IPv4
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    local_addr.sin_port = htons(local_port);        // Local port to listen on

    // Bind the socket to the local port
    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    // Resolve the provided hostname to an IP address
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // Restrict to IPv4 addresses
    hints.ai_socktype = SOCK_DGRAM; // Datagram socket for UDP

    int res = getaddrinfo(hostname, NULL, &hints, &result);
    if (res != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
        return -1; // Return error if resolution fails
    }
    // Convert generic address to IPv4 address and store the remote IP
    struct sockaddr_in *addr = (struct sockaddr_in *)result->ai_addr;
    remote_ip = inet_ntoa(addr->sin_addr); // Convert to string representation

    // Free the address information allocated by getaddrinfo
    freeaddrinfo(result);

    write_screen_custom_ouput("Ready to use.\n");

    return sockfd;
}

int send_message(const char *message)
{
    struct sockaddr_in remote_addr; // Structure to hold remote address information

    // Zero out the structure
    memset(&remote_addr, 0, sizeof(remote_addr));
    // Initialize the remote address structure
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);

    // Convert IPv4 or IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, remote_ip, &remote_addr.sin_addr) <= 0)
    {
        perror("inet_pton failed");
        return -1;
    }

    // Send the message to the remote address
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        perror("sendto failed");
        return -1;
    }

    return 0;
}

int receive_message(char *buffer, int buffer_length)
{
    struct sockaddr_in remote_addr; // Structure to hold the sender's address
    socklen_t addr_len = sizeof(remote_addr); // Variable to hold the length of the address

    // Zero out the buffer
    memset(buffer, 0, buffer_length);

    // Receive a message from the socket
    int len = recvfrom(sockfd, buffer, buffer_length, 0, (struct sockaddr *)&remote_addr, &addr_len);
    if (len < 0)
    {
        perror("Error: recvfrom failed");
        return -1;
    }

    return len; // Return the length of the received message
}

void close_socket()
{
    close(sockfd); // Close the socket file descriptor
}
