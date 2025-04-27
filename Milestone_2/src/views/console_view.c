#include <gtk/gtk.h>
#include <stdarg.h>
#include "console_view.h"

// Forward declaration of idle callback
static gboolean append_text_idle(gpointer data);

typedef struct {
    char *text;
    GtkWidget *console;
} AppendData;

static gboolean console_view_append_text_idle(gpointer user_data) {
    AppendData *data = (AppendData *)user_data;
    GtkTextBuffer *buffer = g_object_get_data(G_OBJECT(data->console), CONSOLE_BUFFER_KEY);
    if (buffer) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, data->text, -1);
        GtkTextMark *end_mark = gtk_text_buffer_get_mark(buffer, "end_mark");
        if (end_mark) {
            gtk_text_buffer_move_mark(buffer, end_mark, &end);
        }
    }
    g_free(data->text);
    g_free(data);
    return G_SOURCE_REMOVE; // Run once
}

void console_view_append_text(GtkWidget *console, const char *text) {
    if (!console) return;
    AppendData *data = g_new(AppendData, 1);
    data->text = g_strdup(text);
    data->console = console;
    g_idle_add(console_view_append_text_idle, data);
}

// --- New signal handler: when Enter pressed ---
static void on_entry_activate(GtkEntry *entry, gpointer user_data) {
    if (!input_queue) return;

    const char *text = gtk_entry_buffer_get_text(gtk_entry_get_buffer(entry));
    if (text && *text) {
        g_async_queue_push(input_queue, g_strdup(text));  // push into queue
        
        // In GTK4, use buffer to set text instead of direct entry method
        GtkEntryBuffer *buffer = gtk_entry_get_buffer(entry);
        gtk_entry_buffer_set_text(buffer, "", 0);
    }
}

// --- Public function to create the console view ---
GtkWidget* console_view_new(GtkWidget **entry_out) {
    // Create a box with vertical orientation
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Create scrolled window that will contain the text view
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);  // Allow vertical expansion
    gtk_widget_set_hexpand(scrolled_window, TRUE);  // Allow horizontal expansion
    
    // Create text view for output
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    
    // Add the text view to the scrolled window
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
    
    // Get the text buffer
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Create a persistent end mark
    GtkTextIter end_iter;
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_create_mark(buffer, "end_mark", &end_iter, FALSE);
    
    // Store buffer in the vbox for easy access
    g_object_set_data(G_OBJECT(vbox), CONSOLE_BUFFER_KEY, buffer);
    
    // Store reference to console widget in buffer for scrolling
    g_object_set_data(G_OBJECT(buffer), "console_widget", vbox);
    
    // Add scrolled window to the vbox (with expand=TRUE)
    gtk_box_append(GTK_BOX(vbox), scrolled_window);
    
    // Create entry field for input (won't expand)
    GtkWidget *entry = gtk_entry_new();
    gtk_widget_set_margin_top(entry, 5);    // Add some spacing
    gtk_widget_set_margin_bottom(entry, 5); // Add some spacing
    gtk_box_append(GTK_BOX(vbox), entry);
    
    // Create input queue if not already done
    if (!input_queue) {
        input_queue = g_async_queue_new_full(g_free);  // With destroy function
    }
    
    // Connect Enter key signal
    g_signal_connect(entry, "activate", G_CALLBACK(on_entry_activate), NULL);
    
    if (entry_out) {
        *entry_out = entry;
    }
    
    return vbox;
}

// Clean up resources
void console_view_cleanup(void) {
    if (input_queue) {
        g_async_queue_unref(input_queue);
        input_queue = NULL;
    }
}