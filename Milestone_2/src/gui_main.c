#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "console_view.h"
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

#define numProcesses 3
#define numQueues 4

// Global variables
Queue *readyQueues[numQueues];  // Ready Queue holding processes waiting to run by the chosen Scheduler
Process *runningProcess = NULL; // currently running process (or NULL if none)
int clockCycle;                 // current clock cycle of the simulation
Queue *job_pool = NULL;
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
Queue *global_blocked_queue = NULL;
int simulation_running = 1;  // Flag to control simulation loop
int selected_scheduler = -1; // 0=FCFS, 1=RR, 2=MLFQ
int quantum = 2;             // Default quantum for RR

FILE *logFile;

// GUI Global variables

GtkWidget *console = NULL;
GtkWidget *entry = NULL;
GAsyncQueue *input_queue = NULL;
GAsyncQueue *action_queue = NULL;

// Initialize the OS simulation components
void initialize_simulation()
{
    clockCycle = 0;
    memory = NULL;
    job_pool = createQueue();
    if (!job_pool)
    {
        console_log_printf("Failed to create job_pool\n");
        return;
    }
    for (int i = 0; i < numQueues; i++)
        readyQueues[i] = createQueue();
    global_blocked_queue = createQueue();
    Process *p1 = createProcess(1, "../programs/Program_1.txt", 0);
    Process *p2 = createProcess(2, "../programs/Program_2.txt", 3);
    Process *p3 = createProcess(3, "../programs/Program_3.txt", 0);
    if (!p1 || !p2 || !p3)
    {
        console_log_printf("Failed to create processes\n");
        return;
    }
    enqueue(job_pool, p1);
    enqueue(job_pool, p2);
    enqueue(job_pool, p3);
    console_log_printf("Job Pool: ");
    displayQueue(job_pool);
    console_log_printf("\n");
}

void cleanup_simulation()
{
    freeMemoryWord();
    freeIndex(&index_table);
    freeQueue(job_pool);
    for (int i = 0; i < numQueues; i++)
        freeQueue(readyQueues[i]);
    if (global_blocked_queue)
        freeQueue(global_blocked_queue);
}

// Simulation thread
static void *simulation_thread(void *data)
{
    initialize_simulation();
    console_log_printf("Starting OS simulation with ");
    switch (selected_scheduler)
    {
    case 0:
        console_log_printf("First-Come First-Served scheduler\n");
        break;
    case 1:
        console_log_printf("Round Robin scheduler (quantum=%d)\n", quantum);
        break;
    case 2:
        console_log_printf("Multi-Level Feedback Queue scheduler\n");
        break;
    }
    while (simulation_running &&
           (getProcessState(1) != TERMINATED ||
            getProcessState(2) != TERMINATED ||
            getProcessState(3) != TERMINATED))
    {
        populateMemory();
        console_log_printf("\n--- Clock Cycle: %d ---\n", clockCycle);
        if (runningProcess)
        {
            console_log_printf("Running Process: ");
            displayProcess(runningProcess);
        }
        else
        {
            console_log_printf("No process currently running\n");
        }
        switch (selected_scheduler)
        {
        case 0:
            runFCFS();
            break;
        case 1:
            runRR(quantum);
            break;
        case 2:
            runMLFQ();
            break;
        }
        clockCycle++;
        g_usleep(500000);                      // 500ms delay
        g_main_context_iteration(NULL, FALSE); // Process GTK events
    }
    console_log_printf("\n--- Simulation Complete ---\n");
    console_log_printf("All processes have terminated after %d clock cycles\n", clockCycle);
    cleanup_simulation();
    return NULL;
}

// Callback for key press events on the window
static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType state, gpointer user_data)
{
    if (gtk_widget_has_focus(entry))
    {
        return FALSE; // Let entry handle Enter
    }

    if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter)
    {
        char *action = g_strdup("enter_action");
        g_async_queue_push(action_queue, action);
        return TRUE;
    }
    return FALSE;
}

// UI thread
static void *ui_thread(void *data)
{
    char buffer[256];
    console_log_printf("=== OS Simulation Console ===\n");
    console_log_printf("Available commands:\n");
    console_log_printf("  start fcfs    - Start simulation with First-Come First-Served scheduler\n");
    console_log_printf("  start rr <q>  - Start with Round Robin (quantum q)\n");
    console_log_printf("  start mlfq    - Start with Multi-Level Feedback Queue\n");
    console_log_printf("  stop          - Stop the current simulation\n");
    console_log_printf("  memory        - Print memory contents\n");
    console_log_printf("  exit          - Exit the program\n");
    console_log_printf("Press Enter to start with default scheduler (FCFS)\n\n");
    pthread_t sim_thread;
    int sim_thread_active = 0;

    while (1)
    {
        if (!simulation_running) continue;

        // Check input_queue and action_queue
        gpointer data = g_async_queue_timeout_pop(input_queue, 100000);
        if (!data)
        {
            data = g_async_queue_timeout_pop(action_queue, 100000);
            if (data)
            {
                if (strcmp((char *)data, "enter_action") == 0)
                {
                    if (!sim_thread_active && selected_scheduler == -1)
                    {
                        selected_scheduler = 0;
                        pthread_create(&sim_thread, NULL, simulation_thread, NULL);
                        sim_thread_active = 1;
                        console_log_printf("Started FCFS simulation (default).\n");
                    }
                    else if (sim_thread_active)
                    {
                        console_log_printf("--- Memory Contents ---\n");
                        printMemory();
                    }
                }
                g_free(data);
                continue;
            }
        }

        if (!data)
        {
            continue;
        }

        strncpy(buffer, (char *)data, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        g_free(data);

        // Skip command validation for prompted input
        if (is_prompted_input)
        {
            // Backend will handle this input (e.g., simulation parameters)
            // Reset flag (set by console_scanf)
            is_prompted_input = FALSE;
            continue; // Let backend process it
        }

        // Validate user-initiated input
        if (selected_scheduler == -1)
        {
            // Allow scheduler selection only once
            if (strncmp(buffer, "start fcfs", 10) == 0)
            {
                if (sim_thread_active)
                {
                    console_log_printf("A simulation is already running. Stop it first.\n");
                }
                else
                {
                    selected_scheduler = 0;
                    pthread_create(&sim_thread, NULL, simulation_thread, NULL);
                    sim_thread_active = 1;
                }
            }
            else if (strncmp(buffer, "start rr", 8) == 0)
            {
                if (sim_thread_active)
                {
                    console_log_printf("A simulation is already running. Stop it first.\n");
                }
                else
                {
                    int q = 2;
                    sscanf(buffer + 8, "%d", &q);
                    if (q <= 0)
                    {
                        console_log_printf("Invalid quantum. Using default value of 2.\n");
                        q = 2;
                    }
                    quantum = q;
                    selected_scheduler = 1;
                    pthread_create(&sim_thread, NULL, simulation_thread, NULL);
                    sim_thread_active = 1;
                }
            }
            else if (strncmp(buffer, "start mlfq", 10) == 0)
            {
                if (sim_thread_active)
                {
                    console_log_printf("A simulation is already running. Stop it first.\n");
                }
                else
                {
                    selected_scheduler = 2;
                    pthread_create(&sim_thread, NULL, simulation_thread, NULL);
                    sim_thread_active = 1;
                }
            }
            else if (strcmp(buffer, "stop") == 0)
            {
                console_log_printf("No simulation is running.\n");
            }
            else if (strcmp(buffer, "memory") == 0)
            {
                console_log_printf("--- Memory Contents ---\n");
                printMemory();
            }
            else if (strcmp(buffer, "exit") == 0)
            {
                console_log_printf("Exiting program.\n");
                gtk_window_close(GTK_WINDOW(gtk_widget_get_ancestor(console, GTK_TYPE_WINDOW)));
                break;
            }
            else
            {
                console_log_printf("Unknown command: %s\n", buffer);
            }
        }
        else
        {
            // Scheduler is locked, only allow stop, memory, exit
            if (strcmp(buffer, "stop") == 0)
            {
                if (sim_thread_active)
                {
                    simulation_running = 0;
                    pthread_join(sim_thread, NULL);
                    sim_thread_active = 0;
                    console_log_printf("Simulation stopped.\n");
                }
                else
                {
                    console_log_printf("No simulation is currently running.\n");
                }
            }
            else if (strcmp(buffer, "memory") == 0)
            {
                console_log_printf("--- Memory Contents ---\n");
                printMemory();
            }
            else if (strcmp(buffer, "exit") == 0)
            {
                if (sim_thread_active)
                {
                    simulation_running = 0;
                    pthread_join(sim_thread, NULL);
                }
                console_log_printf("Exiting program.\n");
                gtk_window_close(GTK_WINDOW(gtk_widget_get_ancestor(console, GTK_TYPE_WINDOW)));
                break;
            }
            else if (strncmp(buffer, "start ", 6) == 0)
            {
                console_log_printf("Scheduler cannot be changed after initial selection.\n");
            }
            else
            {
                console_log_printf("Unknown command: %s\n", buffer);
            }
        }
    }
    return NULL;
}

// Activate function
static void activate(GtkApplication *app, gpointer user_data)
{
    input_queue = g_async_queue_new();
    action_queue = g_async_queue_new();

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "OS Simulation");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkEventController *key_controller = gtk_event_controller_key_new();
    gtk_widget_add_controller(window, key_controller);
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), NULL);

    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(content_box, 10);
    gtk_widget_set_margin_end(content_box, 10);
    gtk_widget_set_margin_top(content_box, 10);
    gtk_widget_set_margin_bottom(content_box, 10);
    gtk_widget_set_hexpand(content_box, TRUE);
    gtk_widget_set_vexpand(content_box, TRUE);

    console = console_view_new(&entry);
    gtk_widget_set_hexpand(console, TRUE);
    gtk_widget_set_vexpand(console, TRUE);
    gtk_box_append(GTK_BOX(content_box), console);

    g_signal_connect(entry, "activate", G_CALLBACK(on_entry_activate), NULL);

    gtk_window_set_child(GTK_WINDOW(window), content_box);

    pthread_t ui_tid;
    pthread_create(&ui_tid, NULL, ui_thread, NULL);

    gtk_window_present(GTK_WINDOW(window));
}

// Main function
int main(int argc, char *argv[])
{
    logFile = fopen("G:/SEM6/CSEN_602/Project/OperatingSystems/Milestone_2/log/log.txt", "w");

    GtkApplication *app = gtk_application_new("org.example.ossimulation", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    console_view_cleanup();
    if (input_queue)
        g_async_queue_unref(input_queue);
    if (action_queue)
        g_async_queue_unref(action_queue);
    g_object_unref(app);

    console_view_cleanup();
    return status;
}
