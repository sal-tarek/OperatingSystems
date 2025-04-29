#ifndef SIMULATOR_CONTROLLER_H
#define SIMULATOR_CONTROLLER_H

#include <gtk/gtk.h>
#include "simulator_view.h"

// Forward declaration of structs defined elsewhere
struct Queue;

// SimulatorController manages the simulation logic
typedef struct {
    SimulatorView *view;            // Reference to the View
    GThread *simulation_thread;     // Thread for periodic memory updates
    int process_id_counter;         // Counter for assigning process IDs
    gboolean running;               // Flag to control the simulation loop
} SimulatorController;

// Function declarations
SimulatorController *simulator_controller_new(SimulatorView *view);
void simulator_controller_start(SimulatorController *controller);
void simulator_controller_create_process(GtkButton *button, gpointer user_data);
void simulator_controller_free(SimulatorController *controller);

#endif // SIMULATOR_CONTROLLER_H