#include "combined_controller.h"
#include "dashboard_view.h"
#include "simulator_view.h"

CombinedController* combined_controller_new(GtkApplication *app) {
    CombinedController *controller = g_new0(CombinedController, 1);
    
    // Create the Dashboard view and controller
    DashboardView *dashboard_view = dashboard_view_new();
    dashboard_view_init(dashboard_view, app);
    controller->dashboard_controller = dashboard_controller_new(dashboard_view, controller);
    
    // Create the Simulator view and controller
    SimulatorView *simulator_view = simulator_view_new(dashboard_view->window);
    controller->simulator_controller = simulator_controller_new(simulator_view, controller);
    
    // Connect the process creation callback
    simulator_view_connect_create_process(simulator_view, 
                                         G_CALLBACK(simulator_controller_create_process), 
                                         controller->simulator_controller);
    
    return controller;
}

void combined_controller_start(CombinedController *controller) {
    // Show the Dashboard
    dashboard_controller_show(controller->dashboard_controller);
    
    // Start the Simulator
    simulator_controller_start(controller->simulator_controller);
}

void combined_controller_free(CombinedController *controller) {
    if (controller) {
        simulator_controller_free(controller->simulator_controller);
        dashboard_controller_free(controller->dashboard_controller);
        g_free(controller);
    }
}