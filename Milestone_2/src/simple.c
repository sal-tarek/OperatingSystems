#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

// Structure to pass data to dialog callbacks
struct DialogData {
    GtkTextView *text_view;
    GtkListBox *job_pool;
    GtkEntry *file_entry;
    GtkEntry *arrival_entry;
    GtkWindow *dialog;
    GtkWindow *parent_window;
};

// Structure to pass data to main window callbacks
struct MainWindowData {
    GtkListBox *job_pool;
    GtkWindow *window;
};

// Counter for process IDs
static int process_id_counter = 0;

// Callback for the "Done" button in the dialog
static void on_done_clicked(GtkButton *button, gpointer user_data) {
    struct DialogData *data = user_data;

    // Get input values
    const char *file_path = gtk_editable_get_text(GTK_EDITABLE(data->file_entry));
    const char *arrival_time_str = gtk_editable_get_text(GTK_EDITABLE(data->arrival_entry));
    int arrival_time = atoi(arrival_time_str);

    // Validate arrival time
    char message[512];
    if (arrival_time < 0) {
        snprintf(message, sizeof(message), "Error: Invalid arrival time %s\n", arrival_time_str);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(data->text_view);
        gtk_text_buffer_insert_at_cursor(buffer, message, -1);
        return;
    }

    // Call createProcess
    int pid = process_id_counter + 1;
    struct Process *process = createProcess(pid, (char *)file_path, arrival_time);

    // Update text area with process attributes
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(data->text_view);
    if (process && process->instructions) {
        snprintf(message, sizeof(message), 
                 "Added process p%d:\nInstructions: %s\nArrival Time: %d\n", 
                 pid, process->instructions, process->arrival_time);
    } else {
        snprintf(message, sizeof(message), "Error: Failed to create process p%d\n", pid);
    }
    gtk_text_buffer_insert_at_cursor(buffer, message, -1);

    // Add process to job pool (e.g., "p1", "p2", etc.)
    char pid_str[16];
    snprintf(pid_str, sizeof(pid_str), "p%d", pid);
    GtkWidget *row = gtk_label_new(pid_str);
    gtk_list_box_append(data->job_pool, row);

    // Increment process ID counter
    process_id_counter++;

    // Do not close dialog (allow manual close via X button)
}

// Callback for the "Create Process" button
static void on_create_process_clicked(GtkButton *button, gpointer user_data) {
    struct MainWindowData *data = user_data;

    // Create dialog (using GtkWindow)
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Create Process");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), data->window);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

    // Create dialog content
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    // File path input
    GtkWidget *file_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(file_entry), "../programs/Program_1.txt");
    gtk_box_append(GTK_BOX(box), file_entry);

    // Arrival time input
    GtkWidget *arrival_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(arrival_entry), "0");
    gtk_box_append(GTK_BOX(box), arrival_entry);

    // Done button
    GtkWidget *done_button = gtk_button_new_with_label("Done");
    gtk_box_append(GTK_BOX(box), done_button);

    // Text view for output
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_box_append(GTK_BOX(box), scrolled_window);

    // Connect Done button
    struct DialogData dialog_data = {
        .text_view = GTK_TEXT_VIEW(text_view),
        .job_pool = data->job_pool,
        .file_entry = GTK_ENTRY(file_entry),
        .arrival_entry = GTK_ENTRY(arrival_entry),
        .dialog = GTK_WINDOW(dialog),
        .parent_window = data->window
    };
    // Store dialog_data to extend its lifetime
    static GQuark dialog_data_quark = 0;
    if (dialog_data_quark == 0) {
        dialog_data_quark = g_quark_from_static_string("dialog_data");
    }
    g_object_set_qdata_full(G_OBJECT(done_button), dialog_data_quark, 
                           g_memdup2(&dialog_data, sizeof(dialog_data)), g_free);
    g_signal_connect(done_button, "clicked", G_CALLBACK(on_done_clicked), 
                     g_object_get_qdata(G_OBJECT(done_button), dialog_data_quark));

    // Show dialog
    gtk_widget_set_visible(dialog, TRUE);
}

// Activate handler for GtkApplication
static void on_activate(GApplication *app, gpointer user_data) {
    // Create main window
    GtkWidget *window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(window), "Operating Systems Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create main vertical box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Create job pool (horizontal)
    GtkWidget *job_pool_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(main_box), job_pool_box);
    GtkWidget *job_pool_label = gtk_label_new("Job Pool: ");
    gtk_box_append(GTK_BOX(job_pool_box), job_pool_label);
    GtkWidget *job_pool = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(job_pool), GTK_SELECTION_NONE);
    gtk_widget_set_size_request(job_pool, 600, -1);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(job_pool), GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(job_pool_box), job_pool);

    // Create "Create Process" button
    GtkWidget *create_button = gtk_button_new_with_label("Create Process");
    gtk_box_append(GTK_BOX(main_box), create_button);

    // Connect signals
    static struct MainWindowData main_data;
    main_data.job_pool = GTK_LIST_BOX(job_pool);
    main_data.window = GTK_WINDOW(window);
    g_signal_connect(create_button, "clicked", G_CALLBACK(on_create_process_clicked), &main_data);

    // Show window
    gtk_widget_set_visible(window, TRUE);
}

int main(int argc, char *argv[]) {
    // Create GtkApplication
    GtkApplication *app = gtk_application_new("org.example.ossimulator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}