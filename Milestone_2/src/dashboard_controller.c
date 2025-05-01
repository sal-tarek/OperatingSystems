#include "dashboard_controller.h"
#include <string.h>

// Backend functions (replace with your actual backend functions)
int get_number_of_processes(void) {
    // Placeholder: replace with actual backend call
    return 15;
}

int get_current_clock_cycle(void) {
    // Placeholder: replace with actual backend call
    return 42;
}

const char* get_active_scheduling_algorithm(void) {
    // Placeholder: replace with actual backend call
    return "Round Robin";
}

DashboardController* dashboard_controller_new(GtkApplication *app) {
    DashboardController *controller = g_new0(DashboardController, 1);
    
    // Create the view
    controller->view = dashboard_view_new();
    dashboard_view_init(controller->view, app);
    
    return controller;
}

void dashboard_controller_show(DashboardController *controller) {
    // Initial update
    dashboard_controller_update(controller);
    
    // Show the view
    dashboard_view_show(controller->view);
    
    // Set up a timer to update the dashboard every second
    g_timeout_add(1000, (GSourceFunc)dashboard_controller_update, controller);
}

void dashboard_controller_update(DashboardController *controller) {
    // Fetch data from the backend
    int process_count = get_number_of_processes();
    int clock_cycle = get_current_clock_cycle();
    const char *algorithm = get_active_scheduling_algorithm();

    // Update the view
    dashboard_view_set_process_count(controller->view, process_count);
    dashboard_view_set_clock_cycle(controller->view, clock_cycle);
    dashboard_view_set_algorithm(controller->view, algorithm);
}

void dashboard_controller_free(DashboardController *controller) {
    if (controller) {
        dashboard_view_free(controller->view);
        g_free(controller);
    }
}