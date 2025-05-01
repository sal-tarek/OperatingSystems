#include "dashboard_controller.h"
#include "memory_manager.h"
#include "Queue.h"
#include "PCB.h"
#include "process.h"
#include "memory.h"
#include <string.h>
#include <stdio.h>

extern int numberOfProcesses;
extern int clockCycle;
extern char* schedulingAlgorithm;

extern Queue *processes;

// Helper function to convert ProcessState to string
const char* process_state_to_string(ProcessState state) {
    switch (state) {
        case NEW: return "NEW";
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case BLOCKED: return "WAITING";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

// Backend functions (replace with your actual backend functions)
int get_number_of_processes(void) {
    return numberOfProcesses;
}

int get_current_clock_cycle(void) {
    return clockCycle;
}

const char* get_active_scheduling_algorithm(void) {
    return schedulingAlgorithm ? schedulingAlgorithm : "None";
}

DashboardController* dashboard_controller_new(GtkApplication *app) {
    DashboardController *controller = g_new0(DashboardController, 1);
    
    // Create the view
    controller->view = dashboard_view_new();
    dashboard_view_init(controller->view, app);
    
    return controller;
}

void dashboard_controller_show(DashboardController *controller) {
    // Initial update
    dashboard_controller_update(controller);
    
    // Show the view
    dashboard_view_show(controller->view);
    
    // Set up a timer to update the dashboard every second
    g_timeout_add(1000, (GSourceFunc)dashboard_controller_update, controller);
}

void dashboard_controller_update(DashboardController *controller) {
    // Fetch data from the backend
    int process_count = get_number_of_processes();
    int clock_cycle = get_current_clock_cycle();
    const char *algorithm = get_active_scheduling_algorithm();

    // Update the view (Overview Section)
    dashboard_view_set_process_count(controller->view, process_count);
    dashboard_view_set_clock_cycle(controller->view, clock_cycle);
    dashboard_view_set_algorithm(controller->view, algorithm);

    // Update the Process List
    dashboard_controller_update_process_list(controller);
}

void dashboard_controller_update_process_list(DashboardController *controller) {
    // Clear the existing process list in the view
    GtkWidget *process_list_box = controller->view->process_list_widgets->process_list_box;
    
    // GTK 4 approach to remove all children from the process_list_box
    GtkWidget *child = gtk_widget_get_first_child(process_list_box);
    while (child != NULL) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(process_list_box), child);
        child = next;
    }

    // Loop through the processes queue (PCB logic unchanged)
    int process_count = getQueueSize(processes);
    for (int i = 0; i < process_count; i++) {
        Process *curr = dequeue(processes);
        int pid = curr->pid;
        DataType type; // Define type as DataType, not a pointer
        char varKey[15];
        snprintf(varKey, 15, "P%d_PCB", pid);
        PCB *pcb = (PCB *)fetchDataByIndex(varKey, &type); // Pass &type
        if (type != TYPE_PCB) {
            perror("Erroneous fetch\n");
            enqueue(processes, curr);
            continue;
        }
        const char *state_str = process_state_to_string(pcb->state);
        dashboard_view_add_process(controller->view, pcb->id, state_str, pcb->priority, 
                                   pcb->memLowerBound, pcb->memUpperBound, pcb->programCounter);
        enqueue(processes, curr);
    }
}

void dashboard_controller_free(DashboardController *controller) {
    if (controller) {
        dashboard_view_free(controller->view);
    }
}