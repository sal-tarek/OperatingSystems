#include "views/console_view.h"

// Structure to hold widgets for easy access
typedef struct {
    GtkWidget *text_view;
    GtkTextBuffer *buffer;
} AppWidgets;

// Function to append text to the text view
static void append_to_text_view(AppWidgets *widgets, const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(widgets->buffer, &end);
    gtk_text_buffer_insert(widgets->buffer, &end, text, -1);
    gtk_text_buffer_insert(widgets->buffer, &end, "\n", -1);
    
    // Scroll to the end
    GtkTextMark *mark = gtk_text_buffer_get_insert(widgets->buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(widgets->text_view), mark, 0.0, TRUE, 0.0, 1.0);
}

// Callback for the dialog's submit button
static void on_submit_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    GtkWidget *entry = g_object_get_data(G_OBJECT(button), "entry");
    GtkWidget *dialog = g_object_get_data(G_OBJECT(button), "dialog");
    
    // Get the input
    const char *input = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry)));
    
    // Append input to text view
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "> %s", input);
    append_to_text_view(widgets, buffer);
    
    // Example: Echo the input as output (replace with your logic)
    snprintf(buffer, sizeof(buffer), "Received: %s", input);
    append_to_text_view(widgets, buffer);
    
    // Close the dialog
    gtk_window_destroy(GTK_WINDOW(dialog));
}

// Callback for the dialog's cancel button
static void on_cancel_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *dialog = g_object_get_data(G_OBJECT(button), "dialog");
    gtk_window_destroy(GTK_WINDOW(dialog));
}

// Create and show the input dialog
static void show_input_dialog(AppWidgets *widgets, GtkWindow *parent) {
    // Create a new dialog
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Input");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 100);

    // Create a box for layout
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    // Create the entry
    GtkWidget *entry = gtk_entry_new();
    gtk_box_append(GTK_BOX(box), entry);

    // Create a button box for Submit and Cancel
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(box), button_box);

    // Create Submit button
    GtkWidget *submit_button = gtk_button_new_with_label("Submit");
    g_object_set_data(G_OBJECT(submit_button), "entry", entry);
    g_object_set_data(G_OBJECT(submit_button), "dialog", dialog);
    g_signal_connect(submit_button, "clicked", G_CALLBACK(on_submit_clicked), widgets);
    gtk_box_append(GTK_BOX(button_box), submit_button);

    // Create Cancel button
    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    g_object_set_data(G_OBJECT(cancel_button), "dialog", dialog);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_cancel_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), cancel_button);

    // Show the dialog
    gtk_window_present(GTK_WINDOW(dialog));

    // Allow submitting with Enter key
    g_signal_connect(entry, "activate", G_CALLBACK(on_submit_clicked), widgets);
    g_object_set_data(G_OBJECT(entry), "entry", entry);
    g_object_set_data(G_OBJECT(entry), "dialog", dialog);
}

// Callback for the trigger button in the main window
static void on_trigger_button_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    show_input_dialog(widgets, parent);
}

// Initialize the application window and widgets
static void activate(GtkApplication *app, gpointer user_data) {
    // Create the main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "OS Simulator Console");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    // Create a vertical box for layout
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), box);

    // Create a scrolled window for the text view
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_box_append(GTK_BOX(box), scrolled_window);
    gtk_widget_set_vexpand(scrolled_window, TRUE);

    // Create the text view
    AppWidgets *widgets = g_new(AppWidgets, 1);
    widgets->text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(widgets->text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widgets->text_view), GTK_WRAP_WORD);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), widgets->text_view);

    // Get the text buffer
    widgets->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->text_view));

    // Create a button to trigger the input dialog
    GtkWidget *trigger_button = gtk_button_new_with_label("Prompt for Input");
    g_signal_connect(trigger_button, "clicked", G_CALLBACK(on_trigger_button_clicked), widgets);
    gtk_box_append(GTK_BOX(box), trigger_button);

    // Show the window
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    // Initialize the GTK application
    GtkApplication *app = gtk_application_new("org.example.console", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run the application
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Clean up
    g_object_unref(app);
    return status;
}