#include "simulator_controller.h"
#include "memory.h"
#include "PCB.h"
#include "Queue.h"
#include "process.h"
#include "memory_manager.h"
#include <stdio.h>
#include <string.h>

// External global variables (defined in simulator.c)
extern struct Queue *job_pool;
extern int clockCycle;

// Update GUI callback (called on the main thread)
static gboolean update_gui(gpointer user_data) {
    SimulatorController *controller = (SimulatorController *)user_data;
    if (!controller || !controller->view) {
        fprintf(stderr, "Error: Cannot update GUI - controller or view is NULL\n");
        return G_SOURCE_REMOVE;
    }
    simulator_view_update_memory(controller->view);
    return G_SOURCE_REMOVE;
}

// Simulation loop to update memory every clock cycle
static gpointer simulation_loop(gpointer user_data) {
    SimulatorController *controller = (SimulatorController *)user_data;
    if (!controller) {
        fprintf(stderr, "Error: Simulation loop - controller is NULL\n");
        return NULL;
    }

    while (controller->running) {
        // Call back-end to populate memory
        populateMemory();

        // Call front-end to update memory display
        g_idle_add(update_gui, controller);

        // Increment clock cycle
        clockCycle++;

        // Sleep for 1 second (1 clock cycle = 1 second)
        g_usleep(1000000);
    }
    return NULL;
}

// Create a new SimulatorController
SimulatorController *simulator_controller_new(SimulatorView *view) {
    if (!view) {
        fprintf(stderr, "Error: Cannot create SimulatorController - view is NULL\n");
        return NULL;
    }

    SimulatorController *controller = g_new(SimulatorController, 1);
    controller->view = view;
    controller->simulation_thread = NULL;
    controller->process_id_counter = 0;
    controller->running = FALSE;
    return controller;
}

// Start the simulation (memory updates every clock cycle)
void simulator_controller_start(SimulatorController *controller) {
    if (!controller) {
        fprintf(stderr, "Error: Cannot start SimulatorController - controller is NULL\n");
        return;
    }
    controller->running = TRUE;
    controller->simulation_thread = g_thread_new("simulation-loop", simulation_loop, controller);
}

// Create a new process when "Done" is clicked
void simulator_controller_create_process(GtkButton *button, gpointer user_data) {
    SimulatorController *controller = (SimulatorController *)user_data;
    if (!controller || !controller->view) {
        fprintf(stderr, "Error: Cannot create process - controller or view is NULL\n");
        return;
    }

    char *file_path;
    int arrival_time;

    // Get input from the View
    simulator_view_get_process_input(controller->view, &file_path, &arrival_time);

    // Validate input
    gboolean has_errors = FALSE;
    char message[512] = {0};

    // Validate file path
    if (!file_path || strlen(file_path) == 0) {
        snprintf(message, sizeof(message), "Error: File path cannot be empty\n");
        has_errors = TRUE;
    } else {
        FILE *file = fopen(file_path, "r");
        if (!file) {
            snprintf(message, sizeof(message), "Error: File %s not found\n", file_path);
            has_errors = TRUE;
        } else {
            fclose(file);
        }
    }

    // Validate arrival time
    if (arrival_time < 0) {
        snprintf(message, sizeof(message), "Error: Cannot have negative arrival time\n");
        has_errors = TRUE;
    }

    // If there are errors, display the message and return
    if (has_errors) {
        simulator_view_append_dialog_text(controller->view, message);
        g_free(file_path);
        return;
    }

    // Create the process
    int pid = controller->process_id_counter + 1;
    struct Process *process = createProcess(pid, file_path, arrival_time);
    if (!process) {
        snprintf(message, sizeof(message), "Error: Failed to create process p%d\n", pid);
        simulator_view_append_dialog_text(controller->view, message);
        g_free(file_path);
        return;
    }

    // Add to job pool
    enqueue(job_pool, process);

    // Update the job pool display
    simulator_view_update_job_pool(controller->view);
    // get the instructions of the process its stored in process struct



    // Update the dialog text area with success message
    snprintf(message, sizeof(message), 
             "Added process p%d:\nInstructions: %s\nArrival Time: %d\n", 
             pid,process -> instructions , arrival_time);
    simulator_view_append_dialog_text(controller->view, message);

    // Increment process ID counter
    controller->process_id_counter++;

    // Free allocated memory
    g_free(file_path);
}

// Free the SimulatorController
void simulator_controller_free(SimulatorController *controller) {
    if (controller) {
        controller->running = FALSE;
        if (controller->simulation_thread) {
            g_thread_join(controller->simulation_thread);
            controller->simulation_thread = NULL;
        }
        g_free(controller);
    }
}