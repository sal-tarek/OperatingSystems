#ifndef GUI_GLOBALS_H
#define GUI_GLOBALS_H

#include "console_view.h"

// Global GUI variables
extern GtkWidget *console;
extern GtkWidget *entry;
extern GAsyncQueue *input_queue;
extern GAsyncQueue *action_queue;
extern gboolean is_prompted_input;

// Function declarations
void console_printf(const char* format, ...);
void console_scanf(char *buffer, size_t size);

#endif // GUI_GLOBALS_H