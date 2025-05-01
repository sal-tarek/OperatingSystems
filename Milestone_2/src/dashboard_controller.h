#ifndef DASHBOARD_CONTROLLER_H
#define DASHBOARD_CONTROLLER_H

#include <gtk/gtk.h>
#include "dashboard_view.h"

typedef struct _DashboardController DashboardController;

// Controller structure
struct _DashboardController {
    DashboardView *view;
};

// Functions to create and show the controller
DashboardController* dashboard_controller_new(GtkApplication *app);
void dashboard_controller_show(DashboardController *controller);

// Function to update the view with backend data
void dashboard_controller_update(DashboardController *controller);

// Function to free the controller
void dashboard_controller_free(DashboardController *controller);

#endif // DASHBOARD_CONTROLLER_H