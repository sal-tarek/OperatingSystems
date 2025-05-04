#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "memory_manager.h"
#include "process.h"
#include "Queue.h"
#include "memory.h"
#include "MLFQ.h"
#include "RoundRobin.h"
#include "FCFS.h"
#include "index.h"
#include "PCB.h"
#include "mutex.h"
#include "controller.h"
#include "view.h"
#include "console_view.h"
#include "console_controller.h"
#include "console_model.h"

#define MAX_NUM_PROCESSES 10 // Maximum number of processes to support
#define MAX_NUM_QUEUES 4     // Maximum number of queues

// Global variables
Queue *readyQueues[MAX_NUM_QUEUES]; // Ready Queue holding processes waiting to run
Process *runningProcess = NULL;     // Currently running process (or NULL if none)
int clockCycle;                     // Current clock cycle of the simulation
Queue *job_pool = NULL;
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
Queue *global_blocked_queue = NULL;
int numProcesses = 0; // Number of processes in the simulation

// Callback for application activation
static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Operating System Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 950, 900);

    // Create main box with vertical orientation
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);

    // Pass main_box to controller_init
    controller_init(app, window, main_box);

    // Create separator between scheduler view and console
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(separator, 10);
    gtk_widget_set_margin_bottom(separator, 10);
    gtk_box_append(GTK_BOX(main_box), separator);

    // Create a header for the Log & Console Panel
    GtkWidget *console_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    // Create a label for the console section with styled text
    GtkWidget *console_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(console_label),
                         "<span font_weight='bold' font_size='large'>Log &amp; Console Panel</span>");
    gtk_widget_set_halign(console_label, GTK_ALIGN_START);

    // Add an info label explaining the console sections
    GtkWidget *info_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(info_label),
                         "<span font_size='small' style='italic'>Left: Execution Log | Right: Program Output</span>");
    gtk_widget_set_halign(info_label, GTK_ALIGN_END);
    gtk_widget_set_hexpand(info_label, TRUE);

    gtk_box_append(GTK_BOX(console_header), console_label);
    gtk_box_append(GTK_BOX(console_header), info_label);
    gtk_box_append(GTK_BOX(main_box), console_header);

    // Create console view with entry widget
    GtkWidget *entry = NULL;
    GtkWidget *console = console_view_new(&entry);

    if (console == NULL || !GTK_IS_WIDGET(console))
    {
        g_warning("Error: console_view_new returned invalid widget");
    }
    else
    {
        // Make console expand to fill available space
        gtk_widget_set_vexpand(console, TRUE);
        gtk_widget_set_hexpand(console, TRUE);

        // Set minimum height to make the console more prominent
        gtk_widget_set_size_request(console, -1, 250); // Increased height from 200 to 250

        // Add a subtle frame around the console
        GtkWidget *console_frame = gtk_frame_new(NULL);
        gtk_frame_set_child(GTK_FRAME(console_frame), console);
        gtk_widget_set_margin_bottom(console_frame, 5);

        gtk_box_append(GTK_BOX(main_box), console_frame);
    }

    // Connect entry signal for input handling
    if (entry && GTK_IS_WIDGET(entry))
    {
        g_signal_connect(entry, "activate", G_CALLBACK(console_controller_on_entry_activate), NULL);

        // Style the entry widget
        gtk_widget_set_margin_start(entry, 5);
        gtk_widget_set_margin_end(entry, 5);
        gtk_widget_set_margin_bottom(entry, 5);
    }
    else
    {
        g_warning("Error: console entry widget is invalid");
    }

    // Set window content
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Initialize entry widget state
    if (entry && GTK_IS_WIDGET(entry))
    {
        console_set_entry_sensitive(FALSE);
    }

    // Welcome log message
    console_model_log_output("[SYSTEM] OS Scheduler Simulation started\n");
    console_model_log_output("[SYSTEM] Clock cycle: %d\n", clockCycle);
    console_model_log_output("[SYSTEM] Processes loaded: %d\n", numProcesses);

    gtk_window_present(GTK_WINDOW(window));
}

// Main function for UI and simulation
static int start_simulation_and_ui(int argc, char **argv)
{
    // Initialize console
    console_model_init();
    console_controller_init();

    // Create GTK application
    GtkApplication *app = gtk_application_new("org.example.ossimulator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run application
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup
    console_controller_cleanup();
    console_model_cleanup();
    controller_cleanup();
    g_object_unref(app);

    return status;
}

int main(int argc, char *argv[])
{
    clockCycle = 0;

    // Initialize memory
    memory = NULL;

    // Create job_pool queue
    job_pool = createQueue();
    if (!job_pool)
    {
        fprintf(stderr, "Failed to create job_pool\n");
        return 1;
    }

    // Create ready queues
    for (int i = 0; i < MAX_NUM_QUEUES; i++)
        readyQueues[i] = createQueue();

    // Create global blocked queue
    global_blocked_queue = createQueue();

    // Create processes
    Process *p1 = createProcess(1, "../programs/Program_1.txt", 0);
    numProcesses++;
    Process *p2 = createProcess(2, "../programs/Program_2.txt", 3);
    numProcesses++;
    Process *p3 = createProcess(3, "../programs/Program_3.txt", 0);
    numProcesses++;
    if (!p1 || !p2 || !p3)
    {
        fprintf(stderr, "Failed to create processes\n");
        freeQueue(job_pool);
        for (int i = 0; i < MAX_NUM_QUEUES; i++)
            freeQueue(readyQueues[i]);
        freeQueue(global_blocked_queue);
        return 1;
    }

    // Enqueue processes
    enqueue(job_pool, p1);
    enqueue(job_pool, p2);
    enqueue(job_pool, p3);

    // Start the UI and simulation
    int status = start_simulation_and_ui(argc, argv);

    // Cleanup
    freeMemoryWord();
    freeIndex(&index_table);
    freeQueue(job_pool);
    for (int i = 0; i < MAX_NUM_QUEUES; i++)
        freeQueue(readyQueues[i]);
    freeQueue(global_blocked_queue);

    return status;
}