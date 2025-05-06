#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include <gtk/gtk.h>

extern GtkWidget *console_widget;
extern GtkWidget *entry_widget;

// Creates the console view widget and returns the entry widget for external use
GtkWidget* console_view_new(GtkWidget **entry_out);

// Updates the program output text view with new text
void console_update_program_output(const char *text);

// Updates the execution log text view with new text
void console_update_log_output(const char *text);

// Clears the input entry field
void console_clear_input(void);

// Sets focus to the input entry field
void console_set_input_focus(void);

// Gets the text from the input entry field and clears it
char* console_get_input_text(void);

// Enables or disables the entry field
void console_set_entry_sensitive(gboolean sensitive);

// Resets both log and program output windows
void console_view_reset(void);

#endif // CONSOLE_VIEW_H