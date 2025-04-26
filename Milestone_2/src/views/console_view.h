#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include <gtk/gtk.h>

// Create a new console widget
GtkWidget* console_view_new(void);

// Append text to the console
void console_view_append_text(GtkWidget *console, const char *text);

// Optionally show an input dialog (optional if you want user interaction)
void console_view_prompt_input(GtkWidget *console, GtkWindow *parent);

#endif
