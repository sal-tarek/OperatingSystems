#include "console_test_driver.h"
#include "console_view.h"
#include "console_controller.h"
#include "console_model.h"

// Callback for sending program output
static void on_send_program_output_clicked(GtkButton *button, gpointer user_data) {
    console_model_program_output("Test program output: %s\n", "Hello, World!");
}

// Callback for sending log output
static void on_send_log_output_clicked(GtkButton *button, gpointer user_data) {
    console_model_log_output("Test log event: %s\n", "Simulation event occurred");
}

// Callback for requesting input
static void on_request_input_clicked(GtkButton *button, gpointer user_data) {
    console_model_request_input("Enter test input: ");
    // Input is logged asynchronously by check_input_queue
}

// Callback for application activation
static void activate(GtkApplication *app, gpointer user_data) {
    // Create main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Console Test Driver");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create main vertical box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Create console widget
    GtkWidget *entry = NULL;
    GtkWidget *console = console_view_new(&entry);
    gtk_widget_set_vexpand(console, TRUE);
    gtk_widget_set_hexpand(console, TRUE);
    gtk_box_append(GTK_BOX(main_box), console);

    // Connect entry signal
    if (entry) {
        g_signal_connect(entry, "activate", G_CALLBACK(console_controller_on_entry_activate), NULL);
    }

    // Create button box for test controls
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(main_box), button_box);

    // Create buttons for testing
    GtkWidget *program_button = gtk_button_new_with_label("Send Program Output");
    g_signal_connect(program_button, "clicked", G_CALLBACK(on_send_program_output_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), program_button);

    GtkWidget *log_button = gtk_button_new_with_label("Send Log Output");
    g_signal_connect(log_button, "clicked", G_CALLBACK(on_send_log_output_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), log_button);

    GtkWidget *input_button = gtk_button_new_with_label("Request Input");
    g_signal_connect(input_button, "clicked", G_CALLBACK(on_request_input_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), input_button);

    // Ensure entry is disabled initially
    console_set_entry_sensitive(FALSE);

    // Show the window
    gtk_window_present(GTK_WINDOW(window));
}

// Main function for the test driver
int console_test_driver_run(int argc, char **argv) {
    // Initialize console model and controller
    console_model_init();
    console_controller_init();

    // Create GTK application
    GtkApplication *app = gtk_application_new("org.example.consoletest", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run application
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    console_controller_cleanup();
    console_model_cleanup();
    g_object_unref(app);

    return status;
}

// Entry point
// int main(int argc, char **argv) {
//     return console_test_driver_run(argc, argv);
// }