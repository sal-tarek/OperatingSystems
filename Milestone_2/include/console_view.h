#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include <gtk/gtk.h>

// Internal data keys
#define CONSOLE_BUFFER_KEY "console-buffer"
#define ENTRY_KEY "input-entry"
#define CONSOLE_WIDGET_KEY "console-widget"
#define DIALOG_WIDGET_KEY "dialog-widget"

// Global async queue to store input lines
extern GAsyncQueue *input_queue;

GtkWidget* console_view_new(GtkWidget **entry_out);
void console_view_printf(GtkWidget *console, const char *format, ...);
void console_view_scanf(GtkWidget *console, char *buffer, size_t size);
void console_view_append_text(GtkWidget *console, const char *text);
void console_view_cleanup(void);

#endif