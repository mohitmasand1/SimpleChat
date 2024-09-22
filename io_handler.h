
// Responsibility: Manages input/output operations, 
// like reading from the keyboard and writing to the screen.

// Key Functions: read keyboard, write screen, etc..

#ifndef _IO_HANDLER_H_
#define _IO_HANDLER_H_

// Reads a line of input from the keyboard (stdin)
// 'buffer' - destination where the input will be added (list node value address)
// 'buffer_size' - maximum number of charecters to read
// Returns the number of chars read, excluding the null terminator
char *read_keyboard_input();

// Outputs the message to the screen (stdout)
// 'message' - string to the displayed (without the \n, should be added here)
void write_screen_output(char* message);

// Outputs the custom message
void write_screen_custom_ouput(const char *message);


#endif