#include <gtk/gtk.h>
#include "dashboard_controller.h"
#include "Queue.h"
#include "process.h"
#include "memory_manager.h"

// gcc -o dashboard test.c dashboard_view.c dashboard_controller.c Queue.c process.c PCB.c memory.c memory_manager.c index.c $(pkg-config --cflags --libs gtk4)

#define numProcesses 3
#define numQueues 4
#define QUEUE_CAPACITY 10 // Define a capacity for the queues


// Global variables
Queue *readyQueues[numQueues];              // Ready Queue holding processes waiting to run by the chosen Scheduler
Process *runningProcess = NULL;             // currently running process (or NULL if none)
int clockCycle = 0;                             // current clock cycle of the simulation
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
Queue *global_blocked_queue = NULL;
int numberOfProcesses = 0;
char* schedulingAlgorithm = NULL;
Queue * processes = NULL;
Queue * job_pool = NULL;


// Main application code
static void activate(GtkApplication *app, gpointer user_data) {

    processes = createQueue();
    Process *p1 = createProcess(1, "../programs/Program_1.txt", 0);
    Process *p2 = createProcess(2, "../programs/Program_2.txt", 0);
    Process *p3 = createProcess(3, "../programs/Program_3.txt", 0);
    if (!p1 || !p2 || !p3) {
        fprintf(stderr, "Failed to create processes\n");
        freeQueue(processes);
        return;
    }

    // Enqueue processes
    enqueue(job_pool, p1);
    enqueue(job_pool, p2);
    enqueue(job_pool, p3);
    populateMemory();


    DashboardController *controller = dashboard_controller_new(app);
    
    // Show the dashboard
    dashboard_controller_show(controller);
    
    // Store the controller in the application data for cleanup
    g_object_set_data_full(G_OBJECT(app), "controller", controller, (GDestroyNotify)dashboard_controller_free);
}

int main(int argc, char **argv) {
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

    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.dashboard", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
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