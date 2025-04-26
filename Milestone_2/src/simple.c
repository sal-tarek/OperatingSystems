#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "Queue.h"
#include "memory_manager.h"
#include "memory.h"
#include "MLFQ.h"
#include "RoundRobin.h"
#include "FCFS.h"
#include "index.h"
#include "PCB.h"
#include "mutex.h"

#define MAX_PROCESSES 10
#define MEMORY_SIZE 60
//gcc `pkg-config --cflags --libs gtk4` -o simulator simulator.c process.c Queue.c memory_manager.c memory.c PCB.c mutex.c index.c

extern struct Queue *job_pool; // Back-end job pool (linked list)

// Structure to pass data to dialog callbacks
typedef struct {
    GtkTextView *text_view;
    GtkListBox *job_pool_display; // Front-end job pool (UI)
    GtkListBox *memory_list;
    GtkEntry *file_entry;
    GtkEntry *arrival_entry;
    GtkWindow *dialog;
    GtkWindow *parent_window;
} DialogData;

// Structure to pass data to main window callbacks
typedef struct {
    GtkListBox *job_pool_display; // Front-end job pool (UI)
    GtkListBox *memory_list;
    GtkWindow *window;
} MainWindowData;

// Counter for process IDs
static int process_id_counter = 0;

// Update front-end job pool display based on back-end job_pool
static void update_job_pool_display(GtkListBox *job_pool_display) {
    // Clear current display
    GtkListBoxRow *row;
    while ((row = gtk_list_box_get_row_at_index(job_pool_display, 0)) != NULL) {
        gtk_list_box_remove(job_pool_display, GTK_WIDGET(row));
    }

    // Use the back-end job_pool to get the size
    int size = getQueueSize(job_pool);
    for (int i = 0; i < size; i++) {
        char pid_str[16];
        snprintf(pid_str, sizeof(pid_str), "p%d", i + 1);
        GtkWidget *row = gtk_label_new(pid_str);
        gtk_list_box_append(job_pool_display, row);
    }
}

// Update memory display (initially empty)
static void update_memory_display(GtkListBox *memory_list) {
    // Clear current display
    GtkListBoxRow *row;
    while ((row = gtk_list_box_get_row_at_index(memory_list, 0)) != NULL) {
        gtk_list_box_remove(memory_list, GTK_WIDGET(row));
    }

    // Display memory slots as empty
    for (int i = 0; i < MEMORY_SIZE; i++) {
        char label[32];
        snprintf(label, sizeof(label), "Slot %d: Empty", i);
        GtkWidget *row = gtk_label_new(label);
        gtk_list_box_append(memory_list, row);
    }
}

// Callback for the "Done" button in the dialog
static void on_done_clicked(GtkButton *button, gpointer user_data) {
    DialogData *data = (DialogData *)user_data;

    // Get input values
    const char *file_path = gtk_editable_get_text(GTK_EDITABLE(data->file_entry));
    const char *arrival_time_str = gtk_editable_get_text(GTK_EDITABLE(data->arrival_entry));
    int arrival_time = 0; // Default to 0 if empty or invalid
    if (arrival_time_str && strlen(arrival_time_str) > 0) {
        arrival_time = atoi(arrival_time_str);
        if (arrival_time < 0) {
            arrival_time = 0; // Default to 0 if negative
        }
    }

    // Clear the text area
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(data->text_view);
    gtk_text_buffer_set_text(buffer, "", -1);

    // Validate file path
    char message[512];
    FILE *file = fopen(file_path, "r");
    if (!file) {
        snprintf(message, sizeof(message), "Error: File %s not found\n", file_path);
        gtk_text_buffer_insert_at_cursor(buffer, message, -1);
        return;
    }
    fclose(file);

    // Call createProcess to add to back-end job_pool
    int pid = process_id_counter + 1;
    struct Process *process = createProcess(pid, (char *)file_path, arrival_time);

    // Update text area with process attributes
    if (process && process->instructions) {
        snprintf(message, sizeof(message), 
                 "Added process p%d:\nInstructions: %s\nArrival Time: %d\n", 
                 pid, process->instructions, process->arrival_time);
    } else {
        snprintf(message, sizeof(message), "Error: Failed to create process p%d\n", pid);
    }
    gtk_text_buffer_insert_at_cursor(buffer, message, -1);

    enqueue(job_pool, process);
    // Update front-end job pool display
    update_job_pool_display(data->job_pool_display);

    // Increment process ID counter
    process_id_counter++;

    // Do not close dialog (allow manual close via X button)
}

// Dialog destroy callback to free the dialog data
static void on_dialog_destroy(GtkWidget *dialog, gpointer user_data) {
    DialogData *data = (DialogData *)user_data;
    g_free(data); // Free the dialog data when dialog is destroyed
}

// Callback for the "+" (Create Process) button
static void on_create_process_clicked(GtkButton *button, gpointer user_data) {
    MainWindowData *main_data = (MainWindowData *)user_data;

    // Create dialog (using GtkWindow)
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Create Process");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), main_data->window);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

    // Create dialog content
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    // File path label and input
    GtkWidget *file_label = gtk_label_new("Enter program file path:");
    gtk_box_append(GTK_BOX(box), file_label);
    GtkWidget *file_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(file_entry), "../programs/Program_1.txt");
    gtk_widget_add_css_class(file_entry, "custom-entry");
    gtk_box_append(GTK_BOX(box), file_entry);

    // Arrival time label and input
    GtkWidget *arrival_label = gtk_label_new("Enter program arrival time:");
    gtk_box_append(GTK_BOX(box), arrival_label);
    GtkWidget *arrival_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(arrival_entry), "ex: 0");
    gtk_widget_add_css_class(arrival_entry, "custom-entry");
    gtk_box_append(GTK_BOX(box), arrival_entry);

    // Done button
    GtkWidget *done_button = gtk_button_new_with_label("Done");
    gtk_widget_add_css_class(done_button, "custom-button");
    gtk_box_append(GTK_BOX(box), done_button);

    // Text view for output
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_widget_add_css_class(text_view, "custom-textview");
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_box_append(GTK_BOX(box), scrolled_window);

    // Allocate and initialize dialog data on heap
    DialogData *dialog_data = g_new(DialogData, 1);
    dialog_data->text_view = GTK_TEXT_VIEW(text_view);
    dialog_data->job_pool_display = main_data->job_pool_display;
    dialog_data->memory_list = main_data->memory_list;
    dialog_data->file_entry = GTK_ENTRY(file_entry);
    dialog_data->arrival_entry = GTK_ENTRY(arrival_entry);
    dialog_data->dialog = GTK_WINDOW(dialog);
    dialog_data->parent_window = main_data->window;

    // Connect Done button with dialog data
    g_signal_connect(done_button, "clicked", G_CALLBACK(on_done_clicked), dialog_data);
    
    // Connect dialog destroy signal to free dialog data
    g_signal_connect(dialog, "destroy", G_CALLBACK(on_dialog_destroy), dialog_data);

    // Show dialog
    gtk_widget_set_visible(dialog, TRUE);
}

// Application activation callback
static void activate(GtkApplication *app, gpointer user_data) {
    // Create main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Operating Systems Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create main horizontal box (for left and right sections)
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Left section (Job Pool and Memory)
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(left_box, 200, -1);
    gtk_box_append(GTK_BOX(main_box), left_box);

    // Job Pool section
    GtkWidget *job_pool_frame = gtk_frame_new("Job pool");
    gtk_widget_add_css_class(job_pool_frame, "custom-frame");
    gtk_box_append(GTK_BOX(left_box), job_pool_frame);

    GtkWidget *job_pool_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_frame_set_child(GTK_FRAME(job_pool_frame), job_pool_box);

    GtkWidget *job_pool_display = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(job_pool_display), GTK_SELECTION_NONE);
    gtk_widget_add_css_class(job_pool_display, "custom-listbox");
    gtk_box_append(GTK_BOX(job_pool_box), job_pool_display);

    // "+" button
    GtkWidget *create_button = gtk_button_new_with_label("+");
    gtk_widget_add_css_class(create_button, "custom-button");
    gtk_box_append(GTK_BOX(job_pool_box), create_button);

    // Memory section
    GtkWidget *memory_frame = gtk_frame_new("Memory");
    gtk_widget_add_css_class(memory_frame, "custom-frame");
    gtk_widget_set_vexpand(memory_frame, TRUE);
    gtk_box_append(GTK_BOX(left_box), memory_frame);

    GtkWidget *memory_scrolled = gtk_scrolled_window_new();
    gtk_frame_set_child(GTK_FRAME(memory_frame), memory_scrolled);

    GtkWidget *memory_list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(memory_list), GTK_SELECTION_NONE);
    gtk_widget_add_css_class(memory_list, "custom-listbox");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(memory_scrolled), memory_list);

    // Right section (placeholder for other components)
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(right_box, TRUE);
    gtk_box_append(GTK_BOX(main_box), right_box);

    // Load CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "window { background-color: #343a40; }"
        "frame.custom-frame { background-color: #e8ecef; border: 1px solid #ced4da; }"
        "label { color: #212529; padding: 5px; font-size: 14px; }"
        "listbox.custom-listbox { background-color: #e8ecef; }"
        "listbox.custom-listbox label { padding: 5px; }"
        "button.custom-button { background-color: #17a2b8; color: white; border-radius: 5px; padding: 5px; }"
        "button.custom-button:hover { background-color: #138496; }"
        "entry.custom-entry { background-color: #f8f9fa; border: 1px solid #ced4da; border-radius: 5px; padding: 5px; }"
        "textview.custom-textview { background-color: #f8f9fa; border: 1px solid #ced4da; padding: 5px; font-size: 12px; }"
    );
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Initialize displays
    update_job_pool_display(GTK_LIST_BOX(job_pool_display));
    update_memory_display(GTK_LIST_BOX(memory_list));

    // Set up main window data for callbacks
    MainWindowData *main_data = g_new(MainWindowData, 1);
    main_data->job_pool_display = GTK_LIST_BOX(job_pool_display);
    main_data->memory_list = GTK_LIST_BOX(memory_list);
    main_data->window = GTK_WINDOW(window);

    // Connect signals with main data
    g_signal_connect(create_button, "clicked", G_CALLBACK(on_create_process_clicked), main_data);

    // Free main_data when window is destroyed
    g_signal_connect_data(window, "destroy", G_CALLBACK(gtk_window_destroy), NULL, NULL, 0);
    g_signal_connect_data(window, "destroy", G_CALLBACK(g_free), main_data, NULL, 0);

    // Show window
    gtk_widget_set_visible(window, TRUE);
}

int main(int argc, char *argv[]) {
    // Initialize back-end job_pool
    job_pool = createQueue();
    if (!job_pool) {
        fprintf(stderr, "Failed to create job_pool\n");
        return 1;
    } 

    // Create and run the application
    GtkApplication *app = gtk_application_new("org.example.ossimulator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Clean up job_pool
    int job_pool_size = getQueueSize(job_pool);
    for (int i = 0; i < job_pool_size; i++) {
        Process *process = dequeue(job_pool);
        if (process) {
            freeProcess(process);
        }
    }
    free(job_pool);

    return status;
}