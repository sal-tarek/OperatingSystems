#ifndef CONSOLE_CONTROLLER_H
#define CONSOLE_CONTROLLER_H

#include <gtk/gtk.h>

// Initializes the console controller
void console_controller_init(void);

// Cleans up controller resources
void console_controller_cleanup(void);

// Handles entry field activation (Enter key)
void console_controller_on_entry_activate(GtkWidget *widget, gpointer user_data);

// Requests input from the user and returns it
char* console_controller_request_input(const char *prompt);

// Processes input for the simulation
void console_controller_process_input(const char *input);

#endif // CONSOLE_CONTROLLER_H