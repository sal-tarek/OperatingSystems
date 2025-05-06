#ifndef CONSOLE_CONTROLLER_H
#define CONSOLE_CONTROLLER_H

#include <gtk/gtk.h>
#include "console_view.h"
#include "console_model.h"

// Callback type for input requests
typedef void (*InputCallback)(void *user_data, char *input);

// Structure for callback data
typedef struct
{
    InputCallback callback;
    void *user_data;
} InputCallbackData;

// Initialize the console controller
void console_controller_init(void);

// Clean up resources
void console_controller_cleanup(void);

// Reset the console view (clear both log and program output windows)
void console_controller_reset_view(void);

// Handle entry widget activation (Enter key)
void console_controller_on_entry_activate(GtkWidget *widget, gpointer user_data);

// Request input with a callback
void console_controller_request_input_with_callback(const char *prompt, InputCallback callback, void *user_data);

// Request input without a callback (backward compatibility)
void console_controller_request_input(const char *prompt);

// Process input by passing it to the model
void console_controller_process_input(const char *input);

#endif // CONSOLE_CONTROLLER_H