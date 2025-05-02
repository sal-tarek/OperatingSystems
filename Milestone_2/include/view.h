#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#include <gtk/gtk.h>
#include <stdarg.h>
#include <string.h>

// Internal data keys
#define CONSOLE_BUFFER_KEY "console-buffer"
#define LOG_BUFFER_KEY "log-buffer"
#define ENTRY_KEY "input-entry"
#define CONSOLE_WIDGET_KEY "console-widget"
#define DIALOG_WIDGET_KEY "dialog-widget"

typedef struct {
    GtkWidget *console;
    char *text;
    const char *buffer_key;
} AppendData;

typedef struct {
    GtkTextBuffer *buffer;
    char *text;
} PrintData;

// Global async queue to store input lines
extern GtkWidget *console;
extern GtkWidget *entry;
extern GAsyncQueue *input_queue;
extern GAsyncQueue *action_queue;
extern gboolean is_prompted_input;

extern FILE *logFile;

GtkWidget* console_view_new(GtkWidget **entry_out);
void console_log_printf(const char *format, ...);
void console_program_output(const char *format, ...);
char* console_scanf(char *buffer, size_t size);
void on_entry_activate(GtkWidget *widget, gpointer user_data);
void writeToLogFile(char *content);
void console_view_cleanup(void);

#endif