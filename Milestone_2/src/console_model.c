#include "console_model.h"
#include "console_controller.h"
#include "console_view.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static FILE *log_file = NULL;

typedef struct
{
    GMainLoop *loop;
    char *input;
    gboolean completed;
} InputRequest;

void console_model_init(void)
{
    log_file = fopen("console_log.txt", "w");
    if (!log_file)
    {
        g_warning("Failed to open console log file");
    }
}

void console_model_cleanup(void)
{
    if (log_file)
    {
        fclose(log_file);
        log_file = NULL;
    }
}

static void on_input_received(InputRequest *req, char *input)
{
    req->input = input; // Store input (caller frees)
    req->completed = TRUE;
    g_main_loop_quit(req->loop); // Exit the loop
}

char *console_model_request_input(const char *prompt)
{
    InputRequest req = {0};
    req.loop = g_main_loop_new(NULL, FALSE);
    req.completed = FALSE;
    req.input = NULL;

    // Request input and pass callback
    console_controller_request_input_with_callback(prompt, (void (*)(void *, char *))on_input_received, &req);

    // Run a nested main loop until input is received
    g_main_loop_run(req.loop);

    // Clean up
    g_main_loop_unref(req.loop);

    if (!req.completed || !req.input)
    {
        g_warning("Input request failed or was cancelled");
        return g_strdup(""); // Return empty string on failure
    }

    return req.input; // Caller (input) must free
}

void console_model_log_output(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Write to log file
    if (log_file)
    {
        fprintf(log_file, "%s", buffer);
        fflush(log_file);
    }

    // Update console log view
    console_update_log_output(buffer); // View
}

void console_model_program_output(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Write to log file
    if (log_file)
    {
        fprintf(log_file, "%s", buffer);
        fflush(log_file);
    }

    // Update console program output view
    console_update_program_output(buffer); // View
}

void console_model_process_input(const char *input)
{
    if (input)
    {
        char formatted[1024];
        snprintf(formatted, sizeof(formatted), "User entered: %s\n", input);
        console_model_log_output("%s", formatted);
    }
}