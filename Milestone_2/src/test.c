#include <gtk/gtk.h>
#include "dashboard_controller.h"
#include "Queue.h"
#include "process.h"
#include "memory_manager.h"

// Global variables (placeholders for backend data)
extern int numberOfProcesses ;
extern int clockCycle ;
extern char* schedulingAlgorithm;
Queue * processes = NULL;
Queue * job_pool = NULL;

// Main application code
static void activate(GtkApplication *app, gpointer user_data) {
    // Initialize the processes queue with dummy process IDs
    job_pool = createQueue();
    if (!job_pool) {
        fprintf(stderr, "Failed to create job_pool\n");
        return;
    }
    processes = createQueue();
    Process *p1 = createProcess(1, "../programs/Program_1.txt", 0);
    Process *p2 = createProcess(2, "../programs/Program_2.txt", 0);
    Process *p3 = createProcess(3, "../programs/Program_3.txt", 0);
    if (!p1 || !p2 || !p3) {
        fprintf(stderr, "Failed to create processes\n");
        freeQueue(processes);
        return;
    }

    // Enqueue processes
    enqueue(job_pool, p1);
    enqueue(job_pool, p2);
    enqueue(job_pool, p3);
    populateMemory();

    


    DashboardController *controller = dashboard_controller_new(app);
    
    // Show the dashboard
    dashboard_controller_show(controller);
    
    // Store the controller in the application data for cleanup
    g_object_set_data_full(G_OBJECT(app), "controller", controller, 
                          (GDestroyNotify)dashboard_controller_free);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.dashboard", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}