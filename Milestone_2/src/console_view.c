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
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); // Increased spacing to 5px

    // Create horizontal box for text views
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10); // Increased spacing to 10px
    gtk_widget_set_vexpand(hbox, TRUE);
    gtk_widget_set_hexpand(hbox, TRUE);

    // Add margin around the console area
    gtk_widget_set_margin_start(hbox, 5);
    gtk_widget_set_margin_end(hbox, 5);
    gtk_widget_set_margin_top(hbox, 5);
    gtk_widget_set_margin_bottom(hbox, 5);

    // Program output scrolled window
    GtkWidget *output_scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(output_scrolled, FALSE); // Changed to FALSE to prevent expansion
    gtk_widget_set_hexpand(output_scrolled, TRUE);
    gtk_widget_set_size_request(output_scrolled, 300, 50);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(output_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Program output text view
    GtkWidget *output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_WORD);
    gtk_widget_set_size_request(output_view, 300, 50);
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
    gtk_widget_set_vexpand(log_scrolled, FALSE); // Changed to FALSE to prevent expansion
    gtk_widget_set_hexpand(log_scrolled, TRUE);
    gtk_widget_set_size_request(log_scrolled, 300, 50);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(log_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Execution log text view
    GtkWidget *log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(log_view), GTK_WRAP_WORD);
    gtk_widget_set_size_request(log_view, 300, 50);
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

    // Create a horizontal box for the input area to keep label and entry on the same line
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(input_box, 5);
    gtk_widget_set_margin_end(input_box, 5);
    gtk_widget_set_margin_top(input_box, 0);
    gtk_widget_set_margin_bottom(input_box, 5);

    // Add label for the input area
    GtkWidget *input_label = gtk_label_new("Console Input:");
    gtk_widget_set_margin_start(input_label, 5);
    gtk_widget_set_margin_end(input_label, 10);
    gtk_box_append(GTK_BOX(input_box), input_label);

    // Create entry with increased height and better styling
    GtkWidget *entry = gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE); // Allow entry to expand horizontally
    gtk_widget_set_margin_start(entry, 0);
    gtk_widget_set_margin_end(entry, 5);
    gtk_widget_set_size_request(entry, -1, 40);

    // Add a stylesheet to make entry more visible
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
                                    "entry { background-color: #f0f0f0; color: #000000; font-size: 14px; font-weight: bold; }", -1);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                               GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    gtk_box_append(GTK_BOX(input_box), entry);
    gtk_box_append(GTK_BOX(vbox), input_box);

    // Add status label below input box to indicate when input is enabled
    GtkWidget *status_label = gtk_label_new("Input disabled - waiting for program request");
    gtk_widget_set_margin_bottom(status_label, 5);
    gtk_widget_set_margin_top(status_label, 5);
    gtk_widget_set_halign(status_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(vbox), status_label);

    // Store status label for later updates
    g_object_set_data(G_OBJECT(entry), "status_label", status_label);

    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Type your input here and press Enter...");
    gtk_widget_set_sensitive(entry, FALSE); // Disabled by default

    // Store globally
    console_widget = vbox;
    entry_widget = entry;

    if (entry_out)
    {
        *entry_out = entry;
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

// Thread-safe version that runs on the main thread
static gboolean set_input_focus_idle(gpointer user_data)
{
    if (entry_widget && GTK_IS_WIDGET(entry_widget) && gtk_widget_get_realized(entry_widget))
    {
        gtk_widget_set_sensitive(entry_widget, TRUE);
        gtk_widget_grab_focus(entry_widget);
        return G_SOURCE_REMOVE;
    }

    g_warning("Cannot set focus on input field - widget not available");
    return G_SOURCE_REMOVE;
}

void console_set_input_focus(void)
{
    // Schedule the focus operation on the main thread to avoid threading issues
    g_idle_add_full(G_PRIORITY_HIGH_IDLE, set_input_focus_idle, NULL, NULL);
}

char *console_get_input_text(void)
{
    if (entry_widget)
    {
        GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(entry_widget));
        const char *text = gtk_entry_buffer_get_text(buffer);
        return g_strdup(text);
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

void console_view_reset(void)
{
    if (console_widget)
    {
        GtkTextBuffer *log_buffer = g_object_get_data(G_OBJECT(console_widget), LOG_BUFFER_KEY);
        GtkTextBuffer *console_buffer = g_object_get_data(G_OBJECT(console_widget), CONSOLE_BUFFER_KEY);
        if (log_buffer)
        {
            gtk_text_buffer_set_text(log_buffer, "", -1);
        }
        if (console_buffer)
        {
            gtk_text_buffer_set_text(console_buffer, "", -1);
        }
    }
}