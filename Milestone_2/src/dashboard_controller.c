#include "dashboard_controller.h"
#include <string.h>
#include <stdio.h>

extern int numberOfProcesses;
extern int clockCycle;
extern char* schedulingAlgorithm;
extern GQueue *processes;

// Dummy fetchPCB function (returns linearly increasing numbers)
typedef struct {
    int pid;
    char state[32];
    int priority;
    int mem_lower;
    int mem_upper;
    int program_counter;
} PCB;

PCB fetchPCB(int pid) {
    static int counter = 0;
    PCB pcb;
    pcb.pid = pid;
    snprintf(pcb.state, sizeof(pcb.state), "%s", counter % 3 == 0 ? "Ready" : counter % 3 == 1 ? "Running" : "Blocked");
    pcb.priority = counter + 10;
    pcb.mem_lower = counter * 100;
    pcb.mem_upper = (counter + 1) * 100;
    pcb.program_counter = counter + 50;
    counter++;
    
    // Print the PCB data
    printf("PID: %d, State: %s, Priority: %d, Mem Lower: %d, Mem Upper: %d, PC: %d\n",
           pcb.pid, pcb.state, pcb.priority, pcb.mem_lower, pcb.mem_upper, pcb.program_counter);
    
    return pcb;
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
    // Loop through the processes queue
    GList *iter;
    for (iter = g_queue_peek_head_link(processes); iter; iter = iter->next) {
        int pid = GPOINTER_TO_INT(iter->data);
        PCB pcb = fetchPCB(pid);
        dashboard_view_add_process(controller->view, pcb.pid, pcb.state, pcb.priority, 
                                  pcb.mem_lower, pcb.mem_upper, pcb.program_counter);
    }
}

void dashboard_controller_free(DashboardController *controller) {
    if (controller) {
        dashboard_view_free(controller->view);
        g_free(controller);
    }
}