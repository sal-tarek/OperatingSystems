#include <gtk/gtk.h>
#include <stdarg.h>
#include "console_view.h"

// Global async queue to store input lines
static GAsyncQueue *input_queue = NULL;

// Forward declaration of idle callback
static gboolean append_text_idle(gpointer data);

// Function to append text to the console (thread-safe)
void console_view_append_text(GtkWidget *console, const char *text) {
    // Get the text buffer from the console widget
    GtkTextBuffer *buffer = g_object_get_data(G_OBJECT(console), CONSOLE_BUFFER_KEY);
    if (!buffer) {
        g_warning("Console buffer not found!");
        return;
    }
    
    // Schedule text appending on the main thread
    g_idle_add(append_text_idle, g_strdup_printf("%p|%s", buffer, text));
}

// Idle callback to safely append text on the main thread
static gboolean append_text_idle(gpointer data) {
    char *combined = (char *)data;
    char *separator = strchr(combined, '|');
    if (!separator) {
        g_free(combined);
        return G_SOURCE_REMOVE;
    }
    
    *separator = '\0';
    GtkTextBuffer *buffer = (GtkTextBuffer *)g_ascii_strtoull(combined, NULL, 16);
    char *text = separator + 1;
    
    // Get end iterator
    GtkTextIter end_iter;
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    
    // Insert text
    gtk_text_buffer_insert(buffer, &end_iter, text, -1);
    
    // Scroll to end more safely
    GtkTextMark *end_mark = gtk_text_buffer_get_mark(buffer, "end_mark");
    if (end_mark) {
        // Move the mark to the end
        gtk_text_buffer_get_end_iter(buffer, &end_iter);
        gtk_text_buffer_move_mark(buffer, end_mark, &end_iter);
        
        // Find the text view and scroll to mark
        GtkWidget *console = g_object_get_data(G_OBJECT(buffer), "console_widget");
        if (console) {
            GtkWidget *scrolled_window = gtk_widget_get_first_child(console);
            if (GTK_IS_SCROLLED_WINDOW(scrolled_window)) {
                GtkWidget *text_view = gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(scrolled_window));
                if (GTK_IS_TEXT_VIEW(text_view)) {
                    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(text_view), end_mark);
                }
            }
        }
    }
    
    g_free(combined);
    return G_SOURCE_REMOVE;
}

// --- printf-like function ---
void console_view_printf(GtkWidget *console, const char *format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    console_view_append_text(console, buffer);
}

// --- scanf-like function ---
void console_view_scanf(GtkWidget *console, char *buffer, size_t size) {
    if (!input_queue) {
        g_warning("Input queue not initialized!");
        return;
    }
    
    // Clear the buffer before filling
    if (buffer && size > 0) {
        buffer[0] = '\0';
    }
    
    gchar *line = g_async_queue_pop(input_queue);  // blocks until something is available
    if (line) {
        // Validate and sanitize input
        g_strstrip(line);  // Remove leading/trailing whitespace
        
        // Copy safely to output buffer
        if (buffer && size > 0) {
            snprintf(buffer, size, "%s", line);
            buffer[size - 1] = '\0';  // Ensure null termination
        }
        
        // Echo input to console
        char echo_buffer[1024];
        snprintf(echo_buffer, sizeof(echo_buffer), "> %s\n", line);
        console_view_append_text(console, echo_buffer);
        
        g_free(line);
    }
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