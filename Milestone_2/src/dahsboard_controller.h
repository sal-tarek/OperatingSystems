#ifndef DASHBOARD_CONTROLLER_H
#define DASHBOARD_CONTROLLER_H

#include <gtk/gtk.h>

// Structure to hold references to the Overview Section widgets
typedef struct {
    GtkWidget *processes_value_label;
    GtkWidget *clock_value_label;
    GtkWidget *algorithm_value_label;
} OverviewWidgets;

// Structure for the controller
typedef struct {
    OverviewWidgets *overview_widgets;
    // Add backend reference or data source here if needed
} DashboardController;

// Function to create a new controller
DashboardController* dashboard_controller_new(OverviewWidgets *overview_widgets);

// Function to update the Overview Section with backend data
void dashboard_controller_update_overview(DashboardController *controller);

// Function to free the controller
void dashboard_controller_free(DashboardController *controller);

#endif // DASHBOARD_CONTROLLER_H