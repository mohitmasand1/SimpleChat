
// Responsibility: Manages input/output operations,
// like reading from the keyboard and writing to the screen.

// Key Functions: read keyboard, write screen, etc..
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "io_handler.h"
#define BUFFER_SIZE 65506 // Max length of udp message (65,535 bytes − 8-byte UDP header − 20-byte IP header - Null bit)


char *read_keyboard_input()
{
    // Static buffer to store typed message of maximum size, for temporary use
    static char buffer[BUFFER_SIZE];
    // Wait for user to press enter, then read the message typed and save in buffer
    ssize_t bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
    // Check if reading was successful, if not, send error message
    if (bytes_read == -1)
    {
        return NULL;
    }
    // Null-terminate the input string
    buffer[bytes_read] = '\0';
    // Allocate memory for the message string and copy the input buffer into it
    char *message = (char *)malloc((bytes_read + 1) * sizeof(char));
    // If malloc failed, show error message
    if (message == NULL)
    {
        perror("Error allocating memory");
        return NULL;
    }
    // Copy the typed message to the newly allocated memory
    strncpy(message, buffer, bytes_read + 1);

    return message;
}

void write_screen_output(char* message)
{
    const char prefix[] = "Received: ";
    // Write the prefix to standard output
    write(STDOUT_FILENO, prefix, strlen(prefix));
    // Write the message to standard output
    write(STDOUT_FILENO, message, strlen(message));
}

void write_screen_custom_ouput(const char *message)
{
    write(STDOUT_FILENO, message, strlen(message));
    fflush(stdout);
}