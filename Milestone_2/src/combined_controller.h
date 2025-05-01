#ifndef COMBINED_CONTROLLER_H
#define COMBINED_CONTROLLER_H

#include <gtk/gtk.h>
#include "dashboard_controller.h"
#include "simulator_controller.h"

typedef struct _CombinedController CombinedController;

struct _CombinedController {
    DashboardController *dashboard_controller;
    SimulatorController *simulator_controller;
};

CombinedController* combined_controller_new(GtkApplication *app);
void combined_controller_start(CombinedController *controller);
void combined_controller_free(CombinedController *controller);

#endif // COMBINED_CONTROLLER_H