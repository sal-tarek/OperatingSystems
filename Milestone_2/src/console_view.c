#include <gtk/gtk.h>
#include <stdarg.h>
#include "console_view.h"

// Forward declaration of idle callback
static gboolean append_text_idle(gpointer data);

gboolean is_prompted_input = FALSE;

// --- Public function to create the console view ---
GtkWidget* console_view_new(GtkWidget **entry_out) {
    // Create a vertical box for overall layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Create a horizontal box for side-by-side text views
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_vexpand(hbox, TRUE);  // Allow vertical expansion
    gtk_widget_set_hexpand(hbox, TRUE);  // Allow horizontal expansion
    
    // Create scrolled window for program output
    GtkWidget *output_scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(output_scrolled, TRUE);
    gtk_widget_set_hexpand(output_scrolled, TRUE);
    
    // Create text view for program output
    GtkWidget *output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_WORD);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(output_scrolled), output_view);
    
    // Get output buffer and create end mark
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    GtkTextIter output_end;
    gtk_text_buffer_get_end_iter(output_buffer, &output_end);
    gtk_text_buffer_create_mark(output_buffer, "end_mark", &output_end, FALSE);
    
    // Create scrolled window for execution logging
    GtkWidget *log_scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(log_scrolled, TRUE);
    gtk_widget_set_hexpand(log_scrolled, TRUE);
    
    // Create text view for execution logging
    GtkWidget *log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(log_view), GTK_WRAP_WORD);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(log_scrolled), log_view);
    
    // Get log buffer and create end mark
    GtkTextBuffer *log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
    GtkTextIter log_end;
    gtk_text_buffer_get_end_iter(log_buffer, &log_end);
    gtk_text_buffer_create_mark(log_buffer, "end_mark", &log_end, FALSE);
    
    // Store buffers in the vbox for easy access
    g_object_set_data(G_OBJECT(vbox), CONSOLE_BUFFER_KEY, output_buffer);
    g_object_set_data(G_OBJECT(vbox), LOG_BUFFER_KEY, log_buffer);
    
    // Store scrolled windows for scrolling access
    g_object_set_data(G_OBJECT(output_buffer), "scrolled_window", output_scrolled);
    g_object_set_data(G_OBJECT(log_buffer), "scrolled_window", log_scrolled);
    
    // Add scrolled windows to horizontal box
    gtk_box_append(GTK_BOX(hbox), log_scrolled);
    gtk_box_append(GTK_BOX(hbox), output_scrolled);
    
    // Add horizontal box to vertical box
    gtk_box_append(GTK_BOX(vbox), hbox);
    
    // Create entry field for input
    GtkWidget *entry = gtk_entry_new();
    gtk_widget_set_margin_top(entry, 5);
    gtk_widget_set_margin_bottom(entry, 5);
    gtk_box_append(GTK_BOX(vbox), entry);
    
    // Create input queue if not already done
    if (!input_queue) {
        input_queue = g_async_queue_new_full(g_free);
    }
    
    // Connect Enter key signal
    g_signal_connect(entry, "activate", G_CALLBACK(on_entry_activate), NULL);
    
    if (entry_out) {
        *entry_out = entry;
    }
    
    return vbox;
}

// Callback for Enter key in the entry widget
void on_entry_activate(GtkWidget *widget, gpointer user_data)
{
    // Get the entry buffer
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(widget));
    
    // Get text from the buffer
    const char *text = gtk_entry_buffer_get_text(buffer);
    
    if (strlen(text) > 0)
    {
        char *text_copy = g_strdup(text);
        g_async_queue_push(input_queue, text_copy);
        // Clear the buffer
        gtk_entry_buffer_set_text(buffer, "", -1);
    }
}

static gboolean console_printf_idle(gpointer user_data) {
    PrintData *data = (PrintData *)user_data;
    if (data->buffer) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(data->buffer, &end);
        gtk_text_buffer_insert(data->buffer, &end, data->text, -1);
        
        // Update the end mark
        GtkTextMark *end_mark = gtk_text_buffer_get_mark(data->buffer, "end_mark");
        if (end_mark) {
            gtk_text_buffer_move_mark(data->buffer, end_mark, &end);
        }
        
        // Scroll to the end mark
        GtkWidget *scrolled_window = g_object_get_data(G_OBJECT(data->buffer), "scrolled_window");
        if (scrolled_window) {
            GtkWidget *text_view = gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(scrolled_window));
            if (text_view && GTK_IS_TEXT_VIEW(text_view)) {
                gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text_view), end_mark, 0.0, FALSE, 0.0, 0.0);
            }
        }
    }
    g_free(data->text);
    g_free(data);
    return G_SOURCE_REMOVE;
}

void console_log_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    PrintData *data = g_new(PrintData, 1);
    data->text = g_strdup(buffer);
    data->buffer = g_object_get_data(G_OBJECT(console), LOG_BUFFER_KEY);
    g_idle_add(console_printf_idle, data);
    writeToLogFile(data->text);
}

void console_program_output(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    PrintData *data = g_new(PrintData, 1);
    data->text = g_strdup(buffer);
    data->buffer = g_object_get_data(G_OBJECT(console), CONSOLE_BUFFER_KEY);
    g_idle_add(console_printf_idle, data);
    writeToLogFile(data->text);
}

char* console_scanf(char *buffer, size_t size)
{
    is_prompted_input = TRUE;
    gpointer data = g_async_queue_pop(input_queue);
    strncpy(buffer, (char *)data, size - 1);
    buffer[size - 1] = '\0';
    char formattedString[1024];
    sprintf(formattedString, "User entered: %s\n", data);
    writeToLogFile(formattedString);
    g_free(data);
    return buffer;
}

// Write to Log file
void writeToLogFile(char *content)
{
    fprintf(logFile, "%s", content);
}

// Clean up resources
void console_view_cleanup(void) {
    if (console) {
        // Retrieve buffers
        GtkTextBuffer *output_buffer = g_object_get_data(G_OBJECT(console), CONSOLE_BUFFER_KEY);
        GtkTextBuffer *log_buffer = g_object_get_data(G_OBJECT(console), LOG_BUFFER_KEY);
        
        // Open file for writing
        FILE *file = fopen("console_log.txt", "w");
        if (file) {
            // Save program output
            if (output_buffer) {
                fprintf(file, "=== Program Output ===\n");
                GtkTextIter start, end;
                gtk_text_buffer_get_bounds(output_buffer, &start, &end);
                char *text = gtk_text_buffer_get_text(output_buffer, &start, &end, FALSE);
                if (text) {
                    fwrite(text, sizeof(char), strlen(text), file);
                    g_free(text);
                }
                fprintf(file, "\n\n");
            }
            
            // Save execution log
            if (log_buffer) {
                fprintf(file, "=== Execution Log ===\n");
                GtkTextIter start, end;
                gtk_text_buffer_get_bounds(log_buffer, &start, &end);
                char *text = gtk_text_buffer_get_text(log_buffer, &start, &end, FALSE);
                if (text) {
                    fwrite(text, sizeof(char), strlen(text), file);
                    g_free(text);
                }
            }
            
            fclose(file);
        }
    }
    
    // Clean up input queue
    if (input_queue) {
        g_async_queue_unref(input_queue);
        input_queue = NULL;
    }
}