#include <gtk/gtk.h>
#include "unified_controller.h"
#include "dashboard_view.h"
#include "simulator_view.h"
#include "Queue.h"
#include "process.h"
#include "memory.h"
#include "memory_manager.h"

#define numProcesses 3
#define numQueues 4
#define QUEUE_CAPACITY 10

// Global variables
Queue *readyQueues[numQueues];
Process *runningProcess = NULL;
int clockCycle = 0;
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
Queue *global_blocked_queue = NULL;
int numberOfProcesses = 0;
char *schedulingAlgorithm = NULL;
Queue *job_pool = NULL;
Queue *processes = NULL;

static void activate(GtkApplication *app, gpointer user_data) {
    fprintf(stderr, "activate called\n");
    // Initialize backend
    job_pool = createQueue();
    if (!job_pool) {
        fprintf(stderr, "Error: Failed to create job_pool\n");
        return;
    }
    processes = createQueue();
    if (!processes) {
        fprintf(stderr, "Error: Failed to create processes queue\n");
        return;
    }
    global_blocked_queue = createQueue();
    if (!global_blocked_queue) {
        fprintf(stderr, "Error: Failed to create global_blocked_queue\n");
        return;
    }
    for (int i = 0; i < numQueues; i++) {
        readyQueues[i] = createQueue();
        if (!readyQueues[i]) {
            fprintf(stderr, "Failed to create readyQueues[%d]\n", i);
            return;
        }
    }
    memory = NULL;
    index_table = NULL;
    numberOfProcesses = 0;
    schedulingAlgorithm = g_strdup("FCFS");

    // Create main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "OS Simulator and Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 600);

    // Main container (horizontal split)
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Initialize views (simulator on left, dashboard on right)
    SimulatorView *simulator_view = simulator_view_new(app, main_box, GTK_WINDOW(window));
    DashboardView *dashboard_view = dashboard_view_new();
    dashboard_view_init(dashboard_view, app, main_box);

    // Initialize unified controller
    UnifiedController *controller = unified_controller_new(dashboard_view, simulator_view);
    if (!controller) {
        fprintf(stderr, "Failed to create UnifiedController\n");
        dashboard_view_free(dashboard_view);
        simulator_view_free(simulator_view);
        return;
    }

    // Connect process creation callback
    simulator_view_connect_create_process(simulator_view, G_CALLBACK(unified_controller_create_process), controller);

    // Start simulation
    unified_controller_start(controller);

    // Show views
    simulator_view_show(simulator_view);
    dashboard_view_show(dashboard_view);
    gtk_widget_set_visible(window, TRUE);

    // Clean up on window destroy
    g_signal_connect(window, "destroy", G_CALLBACK(unified_controller_free), controller);
    g_signal_connect(window, "destroy", G_CALLBACK(dashboard_view_free), dashboard_view);
    g_signal_connect(window, "destroy", G_CALLBACK(simulator_view_free), simulator_view);
}

int main(int argc, char *argv[]) {
    fprintf(stderr, "main called\n");
    GtkApplication *app = gtk_application_new("org.example.ossimulator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Clean up backend
    while (!isEmpty(job_pool)) {
        Process *process = dequeue(job_pool);
        if (process) freeProcess(process);
    }
    freeQueue(job_pool);
    while (!isEmpty(processes)) {
        Process *process = dequeue(processes);
        if (process) freeProcess(process);
    }
    freeQueue(processes);
    while (!isEmpty(global_blocked_queue)) {
        Process *process = dequeue(global_blocked_queue);
        if (process) freeProcess(process);
    }
    freeQueue(global_blocked_queue);
    for (int i = 0; i < numQueues; i++) {
        while (!isEmpty(readyQueues[i])) {
            Process *process = dequeue(readyQueues[i]);
            if (process) freeProcess(process);
        }
        freeQueue(readyQueues[i]);
    }
    freeMemoryWord();
    index_table = NULL;
    g_free(schedulingAlgorithm);

    return status;
}