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
// #include "controller.h"
#include "console_view.h"
#include "console_controller.h"
#include "console_model.h"

#define MAX_NUM_PROCESSES 10    // Maximum number of processes to support
#define MAX_NUM_QUEUES 4        // Maximum number of queues

// Global variables
Queue *readyQueues[MAX_NUM_QUEUES]; // Ready Queue holding processes waiting to run
Process *runningProcess = NULL; // Currently running process (or NULL if none)
int clockCycle; // Current clock cycle of the simulation
Queue *job_pool = NULL;
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
Queue *global_blocked_queue = NULL;
int numProcesses = 0; // Number of processes in the simulation

// Placeholder for simulation step (replace with actual function)
static gboolean run_simulation_step(gpointer user_data) {
    // TODO: Replace with actual simulation step function (e.g., from controller.h)
    // Example: Check if there are processes to run
    if (numProcesses > 0 && job_pool && !isEmpty(job_pool)) {
        clockCycle++;
        console_model_log_output("Simulation step %d\n", clockCycle);
        // Simulate process execution (replace with actual logic)
        Process *p = dequeue(job_pool);
        if (p) {
            console_model_program_output("Running process %d\n", p->pid);
            // Simulate input request (example)
            if (clockCycle % 5 == 0) {
                char buffer[MAX_ARG_LEN];
                sprintf(buffer, "Enter input for process %d: ", p->pid);
                char *input = console_model_request_input(buffer);
                if (input && strlen(input) > 0) {
                    console_model_log_output("Process %d received input: %s\n", p->pid, input);
                    g_free(input);
                }
            }
            enqueue(job_pool, p); // Requeue for testing
        }
        return G_SOURCE_CONTINUE; // Keep running
    }
    console_model_log_output("Simulation complete\n");
    return G_SOURCE_REMOVE; // Stop when done
}

// Callback for application activation
static void activate(GtkApplication *app, gpointer user_data) {
    // Create main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Operating System Simulator");
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

    // Ensure entry is disabled initially
    console_set_entry_sensitive(FALSE);

    // Show the window
    gtk_window_present(GTK_WINDOW(window));

    // Start simulation (run every 500ms)
    g_timeout_add(500, run_simulation_step, NULL);
}

// Main function for UI and simulation
static int start_simulation_and_ui(int argc, char **argv) {
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
    g_object_unref(app);

    return status;
}

int main(int argc, char *argv[]) {
    clockCycle = 0;

    // Initialize memory
    memory = NULL; 
    // Create job_pool queue
    job_pool = createQueue();
    if (!job_pool) {
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
    if (!p1 || !p2 || !p3) {
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