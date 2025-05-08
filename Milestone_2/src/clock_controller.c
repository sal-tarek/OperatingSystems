#include "clock_controller.h"
#include "memory.h"
#include "process.h"
#include "console_model.h"
#include "controller.h"
#include "unified_controller.h"
#include "Queue.h"
#include <stdio.h>

// Forward declarations for external components
extern void controller_update_all(void);
extern void populateMemory(void);
extern ProcessState getProcessState(int pid);
extern int numberOfProcesses;
extern Queue *job_pool;

// Global clock cycle counter
extern int clockCycle;

void clock_controller_init(void)
{
    clockCycle = 0;
    console_model_log_output("[CLOCK] Clock controller initialized\n");
}

gboolean clock_controller_increment()
{
    // Step 1: Check if any processes are still running
    // continue if there are processes in the job pool or there are non-terminated processes
    int any_running = 0;
    if (!isEmpty(job_pool))
        any_running = 1;
    
    // Check if any processes are still running
    for (int i = 1; i <= numberOfProcesses; i++)
    {
        if (getProcessState(i) != TERMINATED)
        {
            any_running = 1;
            break;
        }
    }

    if (!any_running)
    {
        console_model_log_output("[CLOCK] All processes terminated at cycle %d\n", clockCycle);
        return FALSE;
    }

    // Step 2: Update memory representation
    populateMemory();

    // Step 3: Increment the clock cycle
    clockCycle++;

    // Step 4: Run the selected scheduler
    run_selected_scheduler();

    // Step 5: Update all UI components
    controller_update_all();
    
    return TRUE;
}

void clock_controller_reset(void)
{
    clockCycle = 0;
    console_model_log_output("[CLOCK] Clock reset to -1\n");
}