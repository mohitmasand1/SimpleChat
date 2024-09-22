
// Responsibility: Manages thread creation and synchronization.
// It includes functions to start threads and to enforce mutual exclusion and condition signaling.

// Key Functions: start thread functions, lock mutex, unlock mutex, wait condition, signal condition, etc..

#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

// FUNCTIONS

// Initialize and runs all the threads
// Initializes mutex's and condition variable's and lists
void init_thread_manager();

// Shuts down all running threads
// Destroys mutex/consitions vars and frees list
void cleanup_thread_manager();

// Function prototypes for threads

// These 2 threads will work with one list (Producer-Consumer Problem #1)
// Mutex and condition variable management is done here (before & after editting the list ADT)

// Workflow - If list is not full, grab input value (from io_handler.c) and insert to list
void *keyboard_input_thread(void *arg); // Producer
// Workflow - If list is not empty, grab newest value in list and send it with udp
void *udp_send_thread(void *arg); // Consumer

// These 2 threads will work with another list (Producer-Consumer Problem #2)
// Mutex and condition variable management is done here (before & after editting the list ADT)

// Workflow - If list is not full, grab the received message and insert to list
// If user sent a '!', then break out of while loop so all threads end
void *udp_receive_thread(void *arg); // Producer
// Workflow - If list is not empty, grab the newest value in list and display it on screen (from io_handler.c)
void *screen_output_thread(void *arg); // Consumer

// Wakes up all the threads which are waiting on the condition variable to proceed
void wakeup_all_waiting_threads();

// Trigger the cancel flag for all threads, called after user sends '!' from keyboard
void set_threads_cancel_flag();

#endif