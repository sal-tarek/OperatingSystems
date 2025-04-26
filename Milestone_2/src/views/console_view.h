#ifndef CONSOLE_VIEW_H
#define CONSOLE_VIEW_H

#pragma once

#include <gtk/gtk.h>

// Internal data keys
#define CONSOLE_BUFFER_KEY "console-buffer"
#define ENTRY_KEY "input-entry"
#define CONSOLE_WIDGET_KEY "console-widget"
#define DIALOG_WIDGET_KEY "dialog-widget"

GtkWidget* console_view_new(GtkWidget **entry_out);
void console_view_printf(GtkWidget *console, const char *format, ...);
void console_view_scanf(GtkWidget *console, char *buffer, size_t size);
void console_view_append_text(GtkWidget *console, const char *text);

#endif