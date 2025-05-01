#include <gtk/gtk.h>
#include "dashboard_controller.h"

// Global variables (placeholders for backend data)
extern int numberOfProcesses ;
extern int clockCycle ;
extern char* schedulingAlgorithm;

// Global queue of process IDs
GQueue *processes;

// Main application code
static void activate(GtkApplication *app, gpointer user_data) {
    // Initialize the processes queue with dummy process IDs
    processes = g_queue_new();
    g_queue_push_tail(processes, GINT_TO_POINTER(1));
    g_queue_push_tail(processes, GINT_TO_POINTER(2));
    g_queue_push_tail(processes, GINT_TO_POINTER(3));
    g_queue_push_tail(processes, GINT_TO_POINTER(4));
    g_queue_push_tail(processes, GINT_TO_POINTER(5));

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

    // Free the processes queue
    g_queue_free(processes);

    return status;
}