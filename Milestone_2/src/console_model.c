#include "console_model.h"
#include "console_controller.h"
#include "console_view.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static FILE *log_file = NULL;

void console_model_init(void) {
    log_file = fopen("console_log.txt", "w");
}

void console_model_cleanup(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void console_model_process_input(const char *input) {
    if (input) {
        char formatted[1024];
        snprintf(formatted, sizeof(formatted), "User entered: %s\n", input);
        console_model_log_output("%s", formatted);
    }
}

void console_model_log_output(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Write to log file
    if (log_file) {
        fprintf(log_file, "%s", buffer);
        fflush(log_file);
    }

    // Update console log view
    console_update_log_output(buffer); // View
}

void console_model_program_output(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Write to log file
    if (log_file) {
        fprintf(log_file, "%s", buffer);
        fflush(log_file);
    }

    // Update console program output view
    console_update_program_output(buffer); // View
}

char* console_model_request_input(const char *prompt) {
    return console_controller_request_input(prompt); // Controller
}