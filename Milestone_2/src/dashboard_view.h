#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <gtk/gtk.h>
#include "dashboard_controller.h" // Include to get OverviewWidgets definition

// Function to create the Overview Section
GtkWidget* create_overview_section(OverviewWidgets **overview_widgets);

#endif // DASHBOARD_VIEW_H