#include <stdarg.h>
#include <string.h>
#include <gtk/gtk.h>
#include "gui_globals.h"

gboolean is_prompted_input = FALSE;

typedef struct {
    char *text;
    GtkTextBuffer *buffer;
} PrintData;

static gboolean console_printf_idle(gpointer user_data) {
    PrintData *data = (PrintData *)user_data;
    if (data->buffer) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(data->buffer, &end);
        gtk_text_buffer_insert(data->buffer, &end, data->text, -1);
        GtkTextMark *end_mark = gtk_text_buffer_get_mark(data->buffer, "end_mark");
        if (end_mark) {
            gtk_text_buffer_move_mark(data->buffer, end_mark, &end);
        }
    }
    g_free(data->text);
    g_free(data);
    return G_SOURCE_REMOVE;
}

void console_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    PrintData *data = g_new(PrintData, 1);
    data->text = g_strdup(buffer);
    data->buffer = g_object_get_data(G_OBJECT(console), CONSOLE_BUFFER_KEY);
    g_idle_add(console_printf_idle, data);
}

static void on_entry_activate(GtkEntry *entry, gpointer user_data) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(entry);
    const char *text = gtk_entry_buffer_get_text(buffer);
    if (text && *text) {
        g_print("Entry activated: %s\n", text);
        g_async_queue_push(input_queue, g_strdup(text));
        gtk_entry_buffer_set_text(buffer, "", -1); // Clear the entry
    }
}

// Modified console_scanf to set prompted input flag
void console_scanf(char *buffer, size_t size)
{
    is_prompted_input = TRUE; // Mark as prompted input
    gpointer data = g_async_queue_pop(input_queue); // Blocks until input is available
    strncpy(buffer, (char *)data, size - 1);
    buffer[size - 1] = '\0';
    g_free(data);
    // Note: is_prompted_input is reset in ui_thread after processing
}