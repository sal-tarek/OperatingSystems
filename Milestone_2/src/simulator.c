#include <gtk/gtk.h>
#include "simulator_view.h"
#include "simulator_controller.h"
#include "Queue.h"
#include "process.h"
#include "memory.h"
#include "FCFS.h"
#include "index.h"
#include "PCB.h"

#define numProcesses 3
#define numQueues 4
#define QUEUE_CAPACITY 10 // Define a capacity for the queues

// Global variables
Queue *readyQueues[numQueues];              // Ready Queue holding processes waiting to run
Process *runningProcess = NULL;             // Currently running process (or NULL if none)
int clockCycle = 0;                         // Current clock cycle of the simulation
Queue *job_pool = NULL;                     // Job pool queue
MemoryWord *memory = NULL;                  // Memory hashmap
IndexEntry *index_table = NULL;             // Index table for memory management
Queue *global_blocked_queue = NULL;         // Global blocked queue

static void activate(GtkApplication *app, gpointer user_data) {
    // Initialize the View
    SimulatorView *view = simulator_view_new(app);
    if (!view) {
        fprintf(stderr, "Error: Failed to create SimulatorView\n");
        return;
    }

    // Initialize the Controller
    SimulatorController *controller = simulator_controller_new(view);
    if (!controller) {
        fprintf(stderr, "Error: Failed to create SimulatorController\n");
        simulator_view_free(view);
        return;
    }

    // Connect the "Done" button in the process creation dialog to the Controller's callback
    simulator_view_connect_create_process(view, G_CALLBACK(simulator_controller_create_process), controller);

    // Start the simulation loop (memory updates every clock cycle)
    simulator_controller_start(controller);

    // Clean up on window destroy
    // Note: Controller must be freed before View because Controller holds a reference to View
    g_signal_connect(view->window, "destroy", G_CALLBACK(simulator_controller_free), controller);
    g_signal_connect(view->window, "destroy", G_CALLBACK(simulator_view_free), view);
}

int main(int argc, char *argv[]) {
    // Initialize global variables
    clockCycle = 0;

    // Initialize memory hashmap (empty for now)
    memory = NULL;

      // Create job_pool queue
      job_pool = createQueue();
      if (!job_pool) {
          fprintf(stderr, "Failed to create job_pool\n");
          return 1;
      }  

    // Create ready queues
    for (int i = 0; i < numQueues; i++) {
      readyQueues[i] = createQueue();
        if (!readyQueues[i]) {
            fprintf(stderr, "Failed to create readyQueues[%d]\n", i);
            // Clean up already allocated queues
            for (int j = 0; j < i; j++) {
                freeQueue(readyQueues[j]);
            }
            freeQueue(job_pool);
            return 1;
        }
    }

    // Create global blocked queue
    global_blocked_queue = createQueue();
    if (!global_blocked_queue) {
        fprintf(stderr, "Failed to create global_blocked_queue\n");
        for (int i = 0; i < numQueues; i++) {
            freeQueue(readyQueues[i]);
        }
        freeQueue(job_pool);
        return 1;
    }

    // Initialize index table (assuming it's a hashmap or array, set to NULL for now)
    index_table = NULL;

    // Create and run the GTK application
    GtkApplication *app = gtk_application_new("org.example.ossimulator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    // Clean up job_pool
    while (!isEmpty(job_pool)) {
        Process *process = dequeue(job_pool);
        if (process) {
            freeProcess(process);
        }
    }
    freeQueue(job_pool);
    job_pool = NULL;

    // Clean up ready queues
    for (int i = 0; i < numQueues; i++) {
        while (!isEmpty(readyQueues[i])) {
            Process *process = dequeue(readyQueues[i]);
            if (process) {
                freeProcess(process);
            }
        }
        freeQueue(readyQueues[i]);
        readyQueues[i] = NULL;
    }

    // Clean up global blocked queue
    while (!isEmpty(global_blocked_queue)) {
        Process *process = dequeue(global_blocked_queue);
        if (process) {
            freeProcess(process);
        }
    }
    freeQueue(global_blocked_queue);
    global_blocked_queue = NULL;

    // Clean up memory (assuming memory.h provides a function to free the hashmap)
    // Note: This depends on your back-end implementation
    freeMemoryWord();
 

    // Clean up index table (assuming index.h provides a function to free it)
    // Note: This depends on your back-end implementation
    // For now, we'll assume it's a simple pointer and set to NULL
    index_table = NULL;

    return status;
}