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
#include "unified_controller.h"
#include "dashboard_view.h"
#include "simulator_view.h"
#include "clock_controller.h"

#define MAX_NUM_PROCESSES 10 // Maximum number of processes to support
#define MAX_NUM_QUEUES 4     // Maximum number of queues
#define QUEUE_CAPACITY 10

// Global variables
Process *runningProcess = NULL; // Currently running process (or NULL if none)
int clockCycle = 0;             // Current clock cycle of the simulation (managed by clock_controller)
Queue *job_pool = NULL;
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
int numberOfProcesses = 0;
Process *processes[MAX_NUM_PROCESSES] = {NULL};
Queue *global_blocked_queue = NULL;
Queue *readyQueues[MAX_NUM_QUEUES] = {NULL};

// Forward declaration of cleanup function
static void cleanup_resources(void);

// Callback for application activation
static void activate(GtkApplication *app, gpointer user_data)
{
    fprintf(stderr, "activate called\n");

    // Initialize queues
    job_pool = createQueue();
    if (!job_pool)
    {
        fprintf(stderr, "Error: Failed to create job_pool\n");
        return;
    }

    global_blocked_queue = createQueue();
    if (!global_blocked_queue)
    {
        fprintf(stderr, "Error: Failed to create global_blocked_queue\n");
        return;
    }
    for (int i = 0; i < MAX_NUM_QUEUES; i++)
    {
        readyQueues[i] = createQueue();
        if (!readyQueues[i])
        {
            fprintf(stderr, "Failed to create readyQueues[%d]\n", i);
            return;
        }
    }
    memory = NULL;
    index_table = NULL;
    numberOfProcesses = 0;

    // Create the main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Process Scheduler Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    // Create a scrolled window for the main content
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_window_set_child(GTK_WINDOW(window), scrolled_window);

    // Create the main vertical box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), main_box);

    // ---- TOP SECTION: PROCESS LIST ----
    // Process list view (dashboard)
    DashboardView *dashboard_view = dashboard_view_new();
    dashboard_view_init(dashboard_view, app, main_box, GTK_WINDOW(window));
    gtk_widget_set_size_request(dashboard_view->main_container, -1, 150);
    gtk_widget_set_hexpand(dashboard_view->main_container, TRUE);
    gtk_widget_set_margin_bottom(dashboard_view->main_container, 5);

    // First Horizontal separator
    GtkWidget *separator1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(separator1, 5);
    gtk_widget_set_margin_bottom(separator1, 5);
    gtk_box_append(GTK_BOX(main_box), separator1);    
    
    // ---- MIDDLE SECTION: MEMORY, RESOURCES, AND CONTROLS ----

    // Middle container organized horizontally
    GtkWidget *middle_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_vexpand(middle_container, TRUE);
    gtk_box_append(GTK_BOX(main_box), middle_container);

    // Left side: Memory view 
    GtkWidget *memory_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_hexpand(memory_container, FALSE); 
    gtk_widget_set_size_request(memory_container, 280, -1);

    // Memory simulator view
    SimulatorView *simulator_view = simulator_view_new(app, memory_container, GTK_WINDOW(window));
    gtk_widget_set_size_request(simulator_view->main_container, 280, -1);
    gtk_widget_set_vexpand(simulator_view->main_container, TRUE);    
    gtk_box_append(GTK_BOX(middle_container), memory_container);

    // First vertical separator (between memory and resource panel)
    GtkWidget *vseparator1 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_start(vseparator1, 5);
    gtk_widget_set_margin_end(vseparator1, 5);    
    gtk_box_append(GTK_BOX(middle_container), vseparator1);

    // Middle section: Resource panel container
    GtkWidget *resource_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_hexpand(resource_container, TRUE); // allow it to expand
    gtk_widget_set_halign(resource_container, GTK_ALIGN_CENTER); // center in the middle
    gtk_widget_set_size_request(resource_container, 220, -1);
    gtk_box_append(GTK_BOX(middle_container), resource_container);

    // Second vertical separator (between resource panel and controls)
    GtkWidget *vseparator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_margin_start(vseparator2, 5);
    gtk_widget_set_margin_end(vseparator2, 5);  
    gtk_box_append(GTK_BOX(middle_container), vseparator2);    

    // Right side: Controls section (queues)
    GtkWidget *controls_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_size_request(controls_container, 730, -1);
    gtk_widget_set_hexpand(controls_container, FALSE);
    gtk_widget_set_halign(controls_container, GTK_ALIGN_END); // Fix to the right
    gtk_widget_set_valign(controls_container, GTK_ALIGN_FILL);
    gtk_widget_set_margin_end(controls_container, 40); 
    gtk_widget_set_margin_start(controls_container, 10); 
    gtk_widget_set_margin_top(controls_container, 10); 
    gtk_box_append(GTK_BOX(middle_container), controls_container);

    // Create unified controller
    UnifiedController *controller = unified_controller_new(dashboard_view, simulator_view);
    if (!controller)
    {
        fprintf(stderr, "Failed to create UnifiedController\n");
        dashboard_view_free(dashboard_view);
        simulator_view_free(simulator_view);
        return;
    }

    // Initialize clock controller first
    clock_controller_init();    
    
    // Initialize controller with the controls container
    controller_init(app, window, controls_container, controller);
    
    // Create the resource panel in the middle section AFTER initializing the view
    view_create_resource_panel(view, resource_container);

    // Second separator
    GtkWidget *separator2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(separator2, 5);
    gtk_widget_set_margin_bottom(separator2, 2);
    gtk_box_append(GTK_BOX(main_box), separator2);

    // ---- BOTTOM SECTION: CONSOLE ----
    // Console view with proper styling
    GtkWidget *entry = NULL;
    GtkWidget *console = console_create_view(&entry);

    if (console == NULL || !GTK_IS_WIDGET(console))
    {
        g_warning("Error: console_create_view returned invalid widget");
    }
    else
    {
        // Style the console
        gtk_widget_set_vexpand(console, FALSE);
        gtk_widget_set_hexpand(console, TRUE);
        gtk_widget_set_size_request(console, -1, 250);

        // Add frame with better padding for visibility
        GtkWidget *console_frame = gtk_frame_new(NULL);
        gtk_frame_set_child(GTK_FRAME(console_frame), console);
        gtk_widget_set_margin_bottom(console_frame, 10);
        gtk_widget_set_margin_start(console_frame, 5);
        gtk_widget_set_margin_end(console_frame, 5);

        gtk_box_append(GTK_BOX(main_box), console_frame);
    }

    // Connect entry signal and style it
    if (entry && GTK_IS_WIDGET(entry))
    {
        g_signal_connect(entry, "activate", G_CALLBACK(console_controller_on_entry_activate), NULL);
        gtk_widget_set_sensitive(entry, FALSE);
    }
    else
    {
        g_warning("Error: console entry widget is invalid");
    }

    // Connect signals
    simulator_view_connect_create_process(simulator_view, G_CALLBACK(unified_controller_create_process), controller);
    unified_controller_start(controller);

    // Welcome log messages
    console_model_log_output("[SYSTEM] OS Scheduler Simulation started\n");
    console_model_log_output("[SYSTEM] Processes loaded: %d\n", numberOfProcesses);

    // Show views and present window
    dashboard_view_show(dashboard_view);
    simulator_view_show(simulator_view);
    gtk_window_present(GTK_WINDOW(window));

    // Connect destroy signals
    g_signal_connect(window, "destroy", G_CALLBACK(unified_controller_free), controller);
    g_signal_connect(window, "destroy", G_CALLBACK(dashboard_view_free), dashboard_view);
    g_signal_connect(window, "destroy", G_CALLBACK(simulator_view_free), simulator_view);
    g_signal_connect(window, "destroy", G_CALLBACK(cleanup_resources), NULL);
}

// Called when the application is shutting down to free all allocated resources
static void cleanup_resources(void)
{
    console_controller_cleanup();
    console_model_cleanup();
    controller_cleanup();

    while (!isEmpty(job_pool))
    {
        Process *process = dequeue(job_pool);
        if (process)
            freeProcess(process);
    }
    freeQueue(job_pool);

    for (int i = 0; i < numberOfProcesses; i++)
    {
        if (processes[i])
        {
            freeProcess(processes[i]);
            processes[i] = NULL;
        }
    }
    numberOfProcesses = 0;

    while (!isEmpty(global_blocked_queue))
    {
        Process *process = dequeue(global_blocked_queue);
        if (process)
            freeProcess(process);
    }
    freeQueue(global_blocked_queue);

    for (int i = 0; i < MAX_NUM_QUEUES; i++)
    {
        while (!isEmpty(readyQueues[i]))
        {
            Process *process = dequeue(readyQueues[i]);
            if (process)
                freeProcess(process);
        }
        freeQueue(readyQueues[i]);
    }

    freeMemoryWord();
    freeIndex(&index_table);
}

int main(int argc, char *argv[])
{
    // Reset clockCycle - though this will be managed by clock_controller
    clockCycle = 0;

    // Initialize console
    console_model_init();
    console_controller_init();

    // Create GTK application
    GtkApplication *app = gtk_application_new("org.example.ossimulator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run application
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleanup & Free resources done by the destroy signal handler
    g_object_unref(app);

    return status;
}