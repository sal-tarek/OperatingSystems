#include "dashboard_controller.h"

// Placeholder backend functions (replace with actual backend calls)
int get_number_of_processes() {
    // Simulate backend data
    return 42;
}

int get_current_clock_cycle() {
    // Simulate backend data
    return 1234;
}

const char* get_active_scheduling_algorithm() {
    // Simulate backend data
    return "Round Robin";
}

// Create a new controller
DashboardController* dashboard_controller_new(OverviewWidgets *overview_widgets) {
    DashboardController *controller = g_new(DashboardController, 1);
    controller->overview_widgets = overview_widgets;
    return controller;
}

// Update the Overview Section with backend data
void dashboard_controller_update_overview(DashboardController *controller) {
    OverviewWidgets *widgets = controller->overview_widgets;

    // Fetch data from the backend
    int num_processes = get_number_of_processes();
    int clock_cycle = get_current_clock_cycle();
    const char *algorithm = get_active_scheduling_algorithm();

    // Convert numbers to strings
    char processes_str[16];
    char clock_str[16];
    snprintf(processes_str, sizeof(processes_str), "%d", num_processes);
    snprintf(clock_str, sizeof(clock_str), "%d", clock_cycle);

    // Update the labels in the View
    gtk_label_set_text(GTK_LABEL(widgets->processes_value_label), processes_str);
    gtk_label_set_text(GTK_LABEL(widgets->clock_value_label), clock_str);
    gtk_label_set_text(GTK_LABEL(widgets->algorithm_value_label), algorithm);
}

// Free the controller
void dashboard_controller_free(DashboardController *controller) {
    g_free(controller);
}