
// Responsibility: Handles all network-related operations,
// including setting up UDP sockets, sending messages,
// and receiving messages.

// Key Functions: setup socket, send message, receive message, etc..

#ifndef NETWORK_H
#define NETWORK_H

// Initializes the UDP socket for communication.
// Returns the socket descriptor on success, or -1 on error.
int setup_socket(int local_port, char *ip, int port);

// Sends a message using the given socket descriptor.
// Returns 0 on success, or -1 on error.
int send_message(const char *message);

// Receives a message using the given socket descriptor.
// `buffer` should be a pre-allocated space where the message will be stored.
// Returns the number of bytes received, or -1 on error.
int receive_message(char *buffer, int buffer_length);

void close_socket();

#endif // NETWORK_H