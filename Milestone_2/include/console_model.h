#ifndef CONSOLE_MODEL_H
#define CONSOLE_MODEL_H

// Initializes the console model (e.g., opens log file)
void console_model_init(void);

// Cleans up model resources and saves logs
void console_model_cleanup(void);

// Processes user input (logs and stores for simulation)
void console_model_process_input(const char *input);

// Logs a message to the file and console
void console_model_log_output(const char *format, ...);

// Sends program output to the console
void console_model_program_output(const char *format, ...);

// Requests user input with an optional prompt
char* console_model_request_input(const char *prompt);

#endif // CONSOLE_MODEL_H