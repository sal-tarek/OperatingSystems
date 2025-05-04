#include "unified_controller.h"
#include "memory.h"
#include "PCB.h"
#include "Queue.h"
#include "process.h"
#include "memory_manager.h"
#include <stdio.h>
#include <string.h>

extern Queue *job_pool;
extern Queue *processes;
extern int clockCycle;
extern int numberOfProcesses;
extern char *schedulingAlgorithm;


// gcc -o simulator main.c unified_controller.c dashboard_view.c simulator_view.c Queue.c process.c PCB.c memory.c memory_manager.c index.c RoundRobin.c FCFS.c MLFQ.c parser.c mutex.c instruction.c $(pkg-config --cflags --libs gtk4)

const char *process_state_to_string(ProcessState state) {
    switch (state) {
        case NEW: return "NEW";
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case BLOCKED: return "WAITING";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

int get_number_of_processes(void) {
    return numberOfProcesses;
}

int get_current_clock_cycle(void) {
    return clockCycle;
}

const char *get_active_scheduling_algorithm(void) {
    return schedulingAlgorithm ? schedulingAlgorithm : "None";
}


static gboolean update_gui(gpointer user_data) {
    UnifiedController *controller = (UnifiedController *)user_data;
    if (!controller || !controller->dashboard_view || !controller->simulator_view) {
        fprintf(stderr, "Error: update_gui - controller or views are NULL\n");
        return G_SOURCE_REMOVE;
    }
    
    simulator_view_update_memory(controller->simulator_view);
    
    // Update dashboard view (overview and process list)
    int process_count = get_number_of_processes();
    int clock_cycle = get_current_clock_cycle();
    const char *algorithm = get_active_scheduling_algorithm();
    dashboard_view_set_process_count(controller->dashboard_view, process_count);
    dashboard_view_set_clock_cycle(controller->dashboard_view, clock_cycle);
    dashboard_view_set_algorithm(controller->dashboard_view, algorithm);
    
    // Update process list
    GtkWidget *process_list_box = controller->dashboard_view->process_list_widgets->process_list_box;
    if (!process_list_box) {
        fprintf(stderr, "Error: process_list_box is NULL\n");
        return G_SOURCE_REMOVE;
    }
    
    // Clear current process list
    GtkWidget *child = gtk_widget_get_first_child(process_list_box);
    while (child) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(process_list_box), child);
        child = next;
    }
   
    // Loop through the processes queue (PCB logic unchanged)
    for (int i = 0; i < process_count; i++) {
        Process *curr = dequeue(processes);
        int pid = curr->pid;
        DataType type; // Define type as DataType, not a pointer
        char varKey[15];
        snprintf(varKey, 15, "P%d_PCB", pid);
        PCB *pcb = (PCB *)fetchDataByIndex(varKey, &type); // Pass &type
        if (type != TYPE_PCB) {
            enqueue(processes, curr);
            continue;
        }
        const char *state_str = process_state_to_string(pcb->state);
        dashboard_view_add_process(controller->dashboard_view, pcb->id, state_str, pcb->priority, pcb->memLowerBound, pcb->memUpperBound, pcb->programCounter);
        enqueue(processes, curr);
    }
    
    return G_SOURCE_REMOVE; // Only update once per idle add
}

static gpointer simulation_loop(gpointer user_data) {
    UnifiedController *controller = (UnifiedController *)user_data;
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

UnifiedController *unified_controller_new(DashboardView *dashboard_view, SimulatorView *simulator_view) {
    if (!dashboard_view || !simulator_view) {
        fprintf(stderr, "Error: Cannot create UnifiedController - views are NULL\n");
        return NULL;
    }
    
    UnifiedController *controller = g_new(UnifiedController, 1);
    if (!controller) {
        fprintf(stderr, "Error: Memory allocation failed for UnifiedController\n");
        return NULL;
    }
    
    controller->dashboard_view = dashboard_view;
    controller->simulator_view = simulator_view;
    controller->simulation_thread = NULL;
    controller->process_id_counter = 0;
    controller->running = FALSE;
    
    return controller;
}

void unified_controller_start(UnifiedController *controller) {
    if (!controller) {
        fprintf(stderr, "Error: Cannot start UnifiedController - controller is NULL\n");
        return;
    }
    
    controller->running = TRUE;
    controller->simulation_thread = g_thread_new("simulation-loop", simulation_loop, controller);
}

void unified_controller_create_process(GtkButton *button, gpointer user_data) {
    UnifiedController *controller = (UnifiedController *)user_data;
    if (!controller || !controller->simulator_view) {
        fprintf(stderr, "Error: Cannot create process - controller or view is NULL\n");
        return;
    }

    // Get input from the View
    char *file_path;
    int arrival_time;
    simulator_view_get_process_input(controller->simulator_view, &file_path, &arrival_time);
    
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
        simulator_view_append_dialog_text(controller->simulator_view, message);
        g_free(file_path);
        return;
    }
    
    // Create the process
    int pid = controller->process_id_counter + 1;
    fprintf(stderr, "Creating process with PID %d, file: %s, arrival: %d\n", pid, file_path, arrival_time);
    Process *process = createProcess(pid, file_path, arrival_time);
    if (!process) {
        snprintf(message, sizeof(message), "Error: Failed to create process p%d\n", pid);
        simulator_view_append_dialog_text(controller->simulator_view, message);
        g_free(file_path);
        return;
    }
        
    // Add to job pool and processes
    enqueue(job_pool, process);
    
    // Update the job pool display
    simulator_view_update_job_pool(controller->simulator_view);
    
    // Update the dialog text area with success message
    snprintf(message, sizeof(message), 
             "Added process p%d:\nInstructions: %s\nArrival Time: %d\n", 
             pid, process->instructions ? process->instructions : "None", arrival_time);
    simulator_view_append_dialog_text(controller->simulator_view, message);
    
    // Increment process ID counter
    controller->process_id_counter++;
    
    // Free allocated memory
    g_free(file_path);
}

void unified_controller_free(UnifiedController *controller) {
    controller->running = FALSE;
    if (controller->simulation_thread) {
        g_thread_join(controller->simulation_thread);
        controller->simulation_thread = NULL;
    }
    g_free(controller);
}