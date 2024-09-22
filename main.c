
// Responsibility: Acts as the entry point for the application.
// It will parse command-line arguments (port numbers and machine names),
// initialize necessary connections, and start the threads.

// Key Functions: Initialization, argument parsing, thread execution, etc..

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "thread_manager.h"
#include "network.h"

int main(int argc, char *argv[])
{
    // Make sure correct number of args
    if (argc != 4)
    {
        printf("Usage: %s <local_port> <remote_name> <remote_port>\n", argv[0]);
        return 1;
    }

    int local_port = atoi(argv[1]);
    char *remote_name = argv[2];
    int remote_port = atoi(argv[3]);

    // Setup the connection
    setup_socket(local_port, remote_name, remote_port);

    // Begin the threads
    init_thread_manager();

    // Close the connection when threads have joined
    close_socket();

    return 0;
}