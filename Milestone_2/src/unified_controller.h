#ifndef UNIFIED_CONTROLLER_H
#define UNIFIED_CONTROLLER_H

#include <gtk/gtk.h>
#include "dashboard_view.h"
#include "simulator_view.h"

typedef struct {
    DashboardView *dashboard_view;
    SimulatorView *simulator_view;
    GThread *simulation_thread;
    int process_id_counter;
    gboolean running;
} UnifiedController;

UnifiedController *unified_controller_new(DashboardView *dashboard_view, SimulatorView *simulator_view);
void unified_controller_start(UnifiedController *controller);
void unified_controller_create_process(GtkButton *button, gpointer user_data);
void unified_controller_free(UnifiedController *controller);

#endif