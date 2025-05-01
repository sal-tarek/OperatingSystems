#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include <gtk/gtk.h>

// Internal data keys
#define CONSOLE_BUFFER_KEY "console-buffer"
#define ENTRY_KEY "input-entry"
#define CONSOLE_WIDGET_KEY "console-widget"
#define DIALOG_WIDGET_KEY "dialog-widget"

typedef struct {
    char *text;
    GtkWidget *console;
} AppendData;

typedef struct {
    char *text;
    GtkTextBuffer *buffer;
} PrintData;

// Global async queue to store input lines
extern GtkWidget *console;
extern GtkWidget *entry;
extern GAsyncQueue *input_queue;
extern GAsyncQueue *action_queue;
extern gboolean is_prompted_input;

extern FILE *logFile;

GtkWidget* console_view_new(GtkWidget **entry_out);
void console_printf(const char *format, ...);
char* console_scanf(char *buffer, size_t size);
void on_entry_activate(GtkWidget *widget, gpointer user_data);
void console_view_cleanup(void);

#endif