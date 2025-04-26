#include "console_view.h"

// Internal data keys
#define CONSOLE_BUFFER_KEY "console-buffer"
#define ENTRY_KEY "input-entry"
#define CONSOLE_WIDGET_KEY "console-widget"
#define DIALOG_WIDGET_KEY "dialog-widget"

// --- Helper function to append text ---
static void append_text(GtkTextBuffer *buffer, const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);
}

// --- Public function to create the console ---
GtkWidget* console_view_new(void) {
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    GtkWidget *text_view = gtk_text_view_new();

    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Store buffer inside scrolled window for easy access later
    g_object_set_data(G_OBJECT(scrolled_window), CONSOLE_BUFFER_KEY, buffer);

    return scrolled_window;
}

// --- Public function to append text ---
void console_view_append_text(GtkWidget *console, const char *text) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(g_object_get_data(G_OBJECT(console), CONSOLE_BUFFER_KEY));
    if (buffer != NULL) {
        append_text(buffer, text);
    }
}

// --- Handlers ---

// Submit button clicked
static void on_submit_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *dialog = GTK_WIDGET(user_data);
    GtkWidget *entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), ENTRY_KEY));
    GtkWidget *console = GTK_WIDGET(g_object_get_data(G_OBJECT(button), CONSOLE_WIDGET_KEY));

    const char *input = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry)));

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "> %s", input);
    console_view_append_text(console, buffer);

    snprintf(buffer, sizeof(buffer), "Received: %s", input);
    console_view_append_text(console, buffer);

    gtk_window_destroy(GTK_WINDOW(dialog));
}

// Cancel button clicked
static void on_cancel_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *dialog = GTK_WIDGET(user_data);
    gtk_window_destroy(GTK_WINDOW(dialog));
}

// --- Public function to prompt for input ---
void console_view_prompt_input(GtkWidget *console, GtkWindow *parent) {
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Input");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 100);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    GtkWidget *entry = gtk_entry_new();
    gtk_box_append(GTK_BOX(box), entry);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(box), button_box);

    GtkWidget *submit_button = gtk_button_new_with_label("Submit");
    gtk_box_append(GTK_BOX(button_box), submit_button);

    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    gtk_box_append(GTK_BOX(button_box), cancel_button);

    // Store references inside submit button
    g_object_set_data(G_OBJECT(submit_button), ENTRY_KEY, entry);
    g_object_set_data(G_OBJECT(submit_button), CONSOLE_WIDGET_KEY, console);

    // Connect signals
    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_submit_clicked), dialog);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_cancel_clicked), dialog);

    gtk_window_present(GTK_WINDOW(dialog));
}
