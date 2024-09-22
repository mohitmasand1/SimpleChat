
// Responsibility: Manages thread creation and synchronization.
// It includes functions to start threads and to enforce mutual exclusion and condition signaling.

// Key Functions: start thread functions, lock mutex, unlock mutex, wait condition, signal condition, etc..

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "network.h"
#include "list.h"
#include "io_handler.h"
#include "thread_manager.h"
#define BUFFER_SIZE 65506 // Max length of udp message (65,535 bytes − 8-byte UDP header − 20-byte IP header - Null bit)

// Threads
static pthread_t keyboard_tid, send_tid, receive_tid, screen_tid;
// Transmit & Receive mutexes
static pthread_mutex_t t_list_mutex, r_list_mutex, t_cond_mutex, r_cond_mutex = PTHREAD_MUTEX_INITIALIZER;
// Transmit & Receive conditions
static pthread_cond_t t_condition, r_condition = PTHREAD_COND_INITIALIZER;
// Transmit & Receive lists
static List *t_list, *r_list;

void *keyboard_input_thread(void *arg)
{
    while (1)
    {
        // Wait until user inputs '\n', then save the message
        char *message = read_keyboard_input();
        if (message == NULL) // If malloc failed or error occured, show error
        {
            perror("Error reading input\n");
        }

        // CRITICAL SECTION BEGIN
        pthread_mutex_lock(&t_list_mutex);
        // Prepend message to list and check if it was inserted, if not throw error
        if (List_prepend(t_list, (void *)message) == -1)
        {
            perror("Error: Senders List became full.\n");
        }
        // Unlock the mutex so udp_send_thread can access the list safely
        pthread_mutex_unlock(&t_list_mutex);
        // CRITICAL SECTION END

        // Safely signal the udp_send_thread that there is a new message to send
        pthread_mutex_lock(&t_cond_mutex);
        {
            pthread_cond_signal(&t_condition);
        }
        pthread_mutex_unlock(&t_cond_mutex);

        // Check if any other thread has set cancel flag, if so, cancel this thread
        pthread_testcancel();
    }
    // If loop ends, free all waiting threads and exit this thread
    wakeup_all_waiting_threads();
    pthread_exit(NULL);
}

void *udp_send_thread(void *arg)
{
    while (1)
    {
        // Wait until keyboard_input_thread singals that there is a message available to send
        pthread_mutex_lock(&t_cond_mutex);
        {
            pthread_cond_wait(&t_condition, &t_cond_mutex);
        }
        pthread_mutex_unlock(&t_cond_mutex);

        // CRITICAL SECTION BEGIN
        pthread_mutex_lock(&t_list_mutex);
        // Grab the message from the list once it's been added
        char *message = NULL;
        if (List_count(t_list) > 0)
        {
            message = (char *)List_trim(t_list);
            // If message to be sent was not in the list, don't process the message
            if (message != NULL)
            {
                // If sending the message fails, show error message
                if (send_message(message) < 0)
                {
                    perror(" Error: Failed to send message\n");
                }
                // If the message being sent is a '!', trigger cancel flags for all threads
                if (strcmp(message, "!\n") == 0)
                {
                    set_threads_cancel_flag();
                }
                // Free the memory of the message once it's done being used
                free(message);
            }
        }
        // Unlock the mutex so keyboard_input_thread can access the list safely again
        pthread_mutex_unlock(&t_list_mutex);
        // CRITICAL SECTION END

        // Check if any other thread has set cancel flag, if so, cancel this thread
        pthread_testcancel();
    }
    // If loop ends, free all waiting threads and exit this thread
    wakeup_all_waiting_threads();
    pthread_exit(NULL);
}

void *udp_receive_thread(void *arg)
{
    // Static buffer to store received message of maximum size, for temporary use
    static char buffer[BUFFER_SIZE];

    while (1)
    {
        // Clear the buffer for another use
        memset(buffer, 0, BUFFER_SIZE);
        // Receive message from UDP and store the number of bytes and pass buffer var
        int bytes_received = receive_message(buffer, BUFFER_SIZE - 1);
        // Add the C string null-termination
        buffer[bytes_received] = '\0';
        // Allocate the needed memory to store the actual size of the message
        char *message = (char *)malloc((bytes_received + 1) * sizeof(char));
        // If malloc fails, show error message
        if (message == NULL)
        {
            perror("Error allocating memory\n");
        }
        // Copy the received message to the newly allocated memory
        strcpy(message, buffer);

        // CRITICAL SECTION BEGIN
        pthread_mutex_lock(&r_list_mutex);
        // If message was not empty and didn't fail
        if (bytes_received > 0)
        {
            // Prepend it to the list, if that fails, send an error message
            if (List_prepend(r_list, (void *)message) == -1)
            {
                perror("Error: Receivers List became full.\n");
            }
            // If the message received is a '!', trigger cancel flags for all threads
            if (strcmp(message, "!\n") == 0)
            {
                write_screen_custom_ouput("The other user has ended the session.\n");
                set_threads_cancel_flag();
            }
        }
        pthread_mutex_unlock(&r_list_mutex);
        // CRITICAL SECTION END

        // Safely signal the screen_output_thread that there is a new message to print
        pthread_mutex_lock(&r_cond_mutex);
        {
            pthread_cond_signal(&r_condition);
        }
        pthread_mutex_unlock(&r_cond_mutex);

        // Check if any other thread has set cancel flag, if so, cancel this thread
        pthread_testcancel();
    }
    // If loop ends, free all waiting threads and exit this thread
    wakeup_all_waiting_threads();
    pthread_exit(NULL);
}

void *screen_output_thread(void *arg)
{
    while (1)
    {
        // Wait until udp_receive_thread singals that there is a message available to print
        pthread_mutex_lock(&r_cond_mutex);
        {
            pthread_cond_wait(&r_condition, &r_cond_mutex);
        }
        pthread_mutex_unlock(&r_cond_mutex);

        // CRITICAL SECTION BEGIN
        pthread_mutex_lock(&r_list_mutex);
        // Once message has been added, extract it from the list
        char *message = NULL;
        if (List_count(r_list) > 0)
        {
            message = (char *)List_trim(r_list);
            // If received message was not in the list, don't process the message
            if (message != NULL)
            {
                // Output the message on the screen
                write_screen_output(message);
                // Free the allocated space of the message since not in use anymore
                free(message);
            }
        }
        // Unlock the mutex so udp_receive_thread can access the list safely
        pthread_mutex_unlock(&r_list_mutex);
        // CRITICAL SECTION END

        // Check if any other thread has set cancel flag, if so, cancel this thread
        pthread_testcancel();
    }
    // If loop ends, free all waiting threads and exit this thread
    wakeup_all_waiting_threads();
    pthread_exit(NULL);
}

void cleanup_thread_manager()
{
    // Wake up all threads that might be waiting to be signaled
    wakeup_all_waiting_threads();

    // Join all the threads
    pthread_join(keyboard_tid, NULL);
    pthread_join(send_tid, NULL);
    pthread_join(receive_tid, NULL);
    pthread_join(screen_tid, NULL);

    // Destroy mutexes and condition variables
    pthread_cond_destroy(&t_condition);
    pthread_cond_destroy(&r_condition);
    pthread_mutex_destroy(&t_list_mutex);
    pthread_mutex_destroy(&r_list_mutex);
    pthread_mutex_destroy(&t_cond_mutex);
    pthread_mutex_destroy(&r_cond_mutex);

    // Free the used lists
    List_free(t_list, free);
    List_free(r_list, free);
}

void init_thread_manager()
{
    // Create lists
    t_list = List_create();
    r_list = List_create();

    // Create and start the execution of all threads
    pthread_create(&keyboard_tid, NULL, keyboard_input_thread, NULL);
    pthread_create(&send_tid, NULL, udp_send_thread, NULL);
    pthread_create(&receive_tid, NULL, udp_receive_thread, NULL);
    pthread_create(&screen_tid, NULL, screen_output_thread, NULL);

    // Cleanup will run after threads have been canceled
    cleanup_thread_manager();
}

void wakeup_all_waiting_threads()
{
    // Wake up any asleep threads that might be waiting
    pthread_cond_broadcast(&t_condition);
    pthread_cond_broadcast(&r_condition);
}

void set_threads_cancel_flag()
{
    // Trigger the cancel flag for all threads, called after user sends '!'
    pthread_cancel(keyboard_tid);
    pthread_cancel(send_tid);
    pthread_cancel(receive_tid);
    pthread_cancel(screen_tid);
}