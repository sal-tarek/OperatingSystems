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

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);

    // Pass main_box to controller_init
    controller_init(app, window, main_box);
    
    GtkWidget *entry = NULL;
    GtkWidget *console = console_view_new(&entry);
    g_print("Console widget: %p, Entry widget: %p\n", console, entry);
    if (console == NULL || !GTK_IS_WIDGET(console))
    {
        g_printerr("Error: console_view_new returned invalid widget\n");
    }
    else
    {
        gtk_widget_set_vexpand(console, FALSE);
        gtk_widget_set_hexpand(console, TRUE);
        gtk_widget_set_size_request(console, -1, -1); // Remove fixed height
        gtk_box_append(GTK_BOX(main_box), console);
        gtk_widget_set_visible(console, TRUE);
    }

    if (entry && GTK_IS_WIDGET(entry))
    {
        g_signal_connect(entry, "activate", G_CALLBACK(console_controller_on_entry_activate), NULL);
    }
    else
    {
        g_printerr("Error: console entry widget is invalid\n");
    }

    gtk_window_set_child(GTK_WINDOW(window), main_box);

    if (entry && GTK_IS_WIDGET(entry))
    {
        console_set_entry_sensitive(FALSE);
    }
    else
    {
        g_printerr("Error: Cannot set console entry sensitivity, entry is invalid\n");
    }

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
    printf("Job Pool ");
    displayQueue(job_pool);
    printf("\n");

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