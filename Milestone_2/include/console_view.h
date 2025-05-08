#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include <gtk/gtk.h>

// Create the console view
GtkWidget *console_create_view(GtkWidget **entry_out);

// Reset the console view (clear both log and program output windows)
void console_view_reset(void);

// Update the log output window
void console_update_log_output(const char *text);

// Update the program output window
void console_update_program_output(const char *text);

// Get the current input text
char *console_get_input_text(void);

// Clear the input field and disable it
void console_clear_input(void);

// Set focus on the input field and enable it
void console_set_input_focus(void);

#endif // CONSOLE_VIEW_H