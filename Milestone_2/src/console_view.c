#include "console_view.h"

// Keys for storing buffers in the widget
#define CONSOLE_BUFFER_KEY "console_buffer"
#define LOG_BUFFER_KEY "log_buffer"

// Global console widget (set by console_view_new)
GtkWidget *console_widget = NULL;
GtkWidget *entry_widget = NULL;

// Internal function to update a text view
static gboolean update_text_view_idle(gpointer user_data)
{
    struct
    {
        GtkTextBuffer *buffer;
        char *text;
    } *data = user_data;

    if (data->buffer)
    {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(data->buffer, &end);
        gtk_text_buffer_insert(data->buffer, &end, data->text, -1);

        // Update the end mark
        GtkTextMark *end_mark = gtk_text_buffer_get_mark(data->buffer, "end_mark");
        if (end_mark)
        {
            gtk_text_buffer_move_mark(data->buffer, end_mark, &end);
        }

        // Scroll to the end
        GtkWidget *scrolled_window = g_object_get_data(G_OBJECT(data->buffer), "scrolled_window");
        if (scrolled_window)
        {
            GtkWidget *text_view = gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(scrolled_window));
            if (text_view && GTK_IS_TEXT_VIEW(text_view))
            {
                gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text_view), end_mark, 0.0, FALSE, 0.0, 0.0);
            }
        }
    }

    g_free(data->text);
    g_free(data);
    return G_SOURCE_REMOVE;
}

GtkWidget *console_view_new(GtkWidget **entry_out)
{

    // Create vertical box for layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Create horizontal box for text views
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_vexpand(hbox, TRUE);
    gtk_widget_set_hexpand(hbox, TRUE);

    // Program output scrolled window
    GtkWidget *output_scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(output_scrolled, TRUE);
    gtk_widget_set_hexpand(output_scrolled, TRUE);

    // Program output text view
    GtkWidget *output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_WORD);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(output_scrolled), output_view);

    // Program output buffer and end mark
    GtkTextBuffer *output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    GtkTextIter output_end;
    gtk_text_buffer_get_end_iter(output_buffer, &output_end);
    gtk_text_buffer_create_mark(output_buffer, "end_mark", &output_end, FALSE);
    g_object_set_data(G_OBJECT(vbox), CONSOLE_BUFFER_KEY, output_buffer);
    g_object_set_data(G_OBJECT(output_buffer), "scrolled_window", output_scrolled);

    // Execution log scrolled window
    GtkWidget *log_scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(log_scrolled, TRUE);
    gtk_widget_set_hexpand(log_scrolled, TRUE);

    // Execution log text view
    GtkWidget *log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(log_view), GTK_WRAP_WORD);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(log_scrolled), log_view);

    // Execution log buffer and end mark
    GtkTextBuffer *log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
    GtkTextIter log_end;
    gtk_text_buffer_get_end_iter(log_buffer, &log_end);
    gtk_text_buffer_create_mark(log_buffer, "end_mark", &log_end, FALSE);
    g_object_set_data(G_OBJECT(vbox), LOG_BUFFER_KEY, log_buffer);
    g_object_set_data(G_OBJECT(log_buffer), "scrolled_window", log_scrolled);

    // Add scrolled windows to horizontal box
    gtk_box_append(GTK_BOX(hbox), log_scrolled);
    gtk_box_append(GTK_BOX(hbox), output_scrolled);
    gtk_box_append(GTK_BOX(vbox), hbox);

    // Create input entry field
    GtkWidget *entry = gtk_entry_new();
    gtk_widget_set_margin_top(entry, 5);
    gtk_widget_set_margin_bottom(entry, 5);
    gtk_widget_set_sensitive(entry, FALSE); // Disabled by default
    gtk_box_append(GTK_BOX(vbox), entry);

    // Store globally
    console_widget = vbox;
    entry_widget = entry;

    if (entry_out)
    {
        *entry_out = entry;
    }
    if (console_widget)
    {
        // Return existing widget if already created
        if (entry_out)
        {
            *entry_out = entry_widget;
        }
        return console_widget;
    }

    return vbox;
}

void console_update_program_output(const char *text)
{
    if (console_widget && text)
    {
        GtkTextBuffer *buffer = g_object_get_data(G_OBJECT(console_widget), CONSOLE_BUFFER_KEY);
        if (buffer)
        {
            struct
            {
                GtkTextBuffer *buffer;
                char *text;
            } *data = g_new0(typeof(*data), 1);
            data->buffer = buffer;
            data->text = g_strdup(text);
            g_idle_add(update_text_view_idle, data);
        }
    }
}

void console_update_log_output(const char *text)
{
    if (console_widget && text)
    {
        GtkTextBuffer *buffer = g_object_get_data(G_OBJECT(console_widget), LOG_BUFFER_KEY);
        if (buffer)
        {
            struct
            {
                GtkTextBuffer *buffer;
                char *text;
            } *data = g_new0(typeof(*data), 1);
            data->buffer = buffer;
            data->text = g_strdup(text);
            g_idle_add(update_text_view_idle, data);
        }
    }
}

void console_clear_input(void)
{
    if (entry_widget)
    {
        GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry_widget));
        gtk_entry_buffer_set_text(buffer, "", -1);
        gtk_widget_set_sensitive(entry_widget, FALSE); // Disable after clearing
    }
}

void console_set_input_focus(void)
{
    if (entry_widget && GTK_IS_WIDGET(entry_widget))
    {
        gtk_widget_set_sensitive(entry_widget, TRUE);
        gtk_widget_grab_focus(entry_widget);
    }
    else
    {
        g_warning("console_set_input_focus: entry_widget is NULL or invalid!");
    }
}

char *console_get_input_text(void)
{
    if (entry_widget)
    {
        GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry_widget));
        const char *text = gtk_entry_buffer_get_text(buffer);
        char *text_copy = g_strdup(text);
        gtk_entry_buffer_set_text(buffer, "", -1);
        gtk_widget_set_sensitive(entry_widget, FALSE); // Disable after input
        return text_copy;
    }
    return g_strdup("");
}

void console_set_entry_sensitive(gboolean sensitive)
{
    if (entry_widget)
    {
        gtk_widget_set_sensitive(entry_widget, sensitive);
    }
}