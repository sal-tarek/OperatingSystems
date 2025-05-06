#include <gtk/gtk.h>
#include <glib.h>
#include "view.h"
#include "process.h"
#include "Queue.h"
#include "MLFQ.h"
#include "RoundRobin.h"
#include "FCFS.h"
#include "memory.h"
#include "PCB.h"
#include "instruction.h"
#include "memory_manager.h"
#include "controller.h"
#include "clock_controller.h"
#include "console_model.h"
#include "console_controller.h"

extern int numberOfProcesses;
extern Process *runningProcess;
extern Queue *readyQueues[MAX_NUM_QUEUES];
extern Queue *global_blocked_queue;
extern int clockCycle;

// Define schedulingAlgorithm globally so it can be accessed from other files
char *schedulingAlgorithm = NULL;

typedef struct
{
    GtkWidget *view_window;
    GtkWidget *running_process_label;
    GtkWidget *step_button;
    GtkWidget *automatic_button;
    GtkWidget *pause_button;
    GtkWidget *reset_button;
    GtkWidget *scheduler_combo;
    GtkWidget *quantum_entry;
    GtkWidget *quantum_label;
    guint automatic_timer_id;
    gboolean is_running;
    int quantum;
} Controller;

static Controller *controller = NULL;

static void on_step_clicked(GtkWidget *button, gpointer user_data);
static void on_automatic_clicked(GtkWidget *button, gpointer user_data);
static void on_pause_clicked(GtkWidget *button, gpointer user_data);
static void on_reset_clicked(GtkWidget *button, gpointer user_data);
static void on_scheduler_changed(GtkWidget *combo, GParamSpec *pspec, gpointer user_data);
static gboolean automatic_step(gpointer user_data);

void controller_update_queue_display(int queue_index)
{
    if (queue_index < 0 || queue_index >= MAX_NUM_QUEUES)
        return;

    GList *pid_list = NULL;
    Process *curr = readyQueues[queue_index]->front;
    while (curr != NULL)
    {
        pid_list = g_list_append(pid_list, GINT_TO_POINTER(curr->pid));
        curr = curr->next;
    }

    // Log queue changes
    if (g_list_length(pid_list) > 0)
    {
        GString *queue_str = g_string_new("");
        g_string_append_printf(queue_str, "Queue %d: ", queue_index);
        
        GList *iter = pid_list;
        while (iter)
        {
            g_string_append_printf(queue_str, "P%d", GPOINTER_TO_INT(iter->data));
            iter = iter->next;
            if (iter)
                g_string_append(queue_str, " -> ");
        }
        
        console_model_log_output("[QUEUE] %s\n", queue_str->str);
        g_string_free(queue_str, TRUE);
    }

    int running_pid = (runningProcess != NULL) ? runningProcess->pid : -1;
    view_update_queue(queue_index, pid_list, running_pid);
    g_list_free(pid_list);
}

void controller_update_blocked_queue_display(void)
{
    GList *pid_list = NULL;
    Process *curr = global_blocked_queue->front;
    while (curr != NULL)
    {
        pid_list = g_list_append(pid_list, GINT_TO_POINTER(curr->pid));
        curr = curr->next;
    }

    // Log blocked queue changes
    if (g_list_length(pid_list) > 0)
    {
        GString *blocked_str = g_string_new("");
        g_string_append(blocked_str, "Blocked Queue: ");
        
        GList *iter = pid_list;
        while (iter)
        {
            g_string_append_printf(blocked_str, "P%d", GPOINTER_TO_INT(iter->data));
            iter = iter->next;
            if (iter)
                g_string_append(blocked_str, " -> ");
        }
        
        console_model_log_output("[BLOCKED] %s\n", blocked_str->str);
        g_string_free(blocked_str, TRUE);
    }

    int running_pid = (runningProcess != NULL) ? runningProcess->pid : -1;
    view_update_queue(4, pid_list, running_pid);
    g_list_free(pid_list);
}

void controller_update_running_process()
{
    GString *process_str = g_string_new("");

    if (runningProcess != NULL)
    {
        char pcb_key[32];
        snprintf(pcb_key, sizeof(pcb_key), "P%d_PCB", runningProcess->pid);
        DataType type;
        void *data = fetchDataByIndex(pcb_key, &type);
        PCB *pcb = (type == TYPE_PCB && data) ? (PCB *)data : NULL;

        char key[32];
        snprintf(key, sizeof(key), "P%d_Instruction_%d", runningProcess->pid, pcb ? pcb->programCounter + 1 : 1);
        char *instruction = fetchDataByIndex(key, &type);

        if (!instruction)
        {
            g_string_append_printf(process_str, "Running Process: PID=%d, Instruction=N/A, Time in Queue=%d", (int)runningProcess->pid, runningProcess->timeInQueue);
            fprintf(stderr, "Failed to fetch instruction for key: %s\n", key);
        }
        else
        {
            g_string_append_printf(process_str, "Running Process: PID=%d, Instruction=%s, Time in Queue=%d", (int)runningProcess->pid, instruction, runningProcess->timeInQueue);
        }
    }
    else
    {
        g_string_append(process_str, "Running Process: None");
    }

    gtk_label_set_text(GTK_LABEL(controller->running_process_label), process_str->str);
    g_string_free(process_str, TRUE);
}

void controller_update_all()
{
    // Log significant state changes
    if (runningProcess != NULL)
    {
        char pcb_key[32];
        snprintf(pcb_key, sizeof(pcb_key), "P%d_PCB", runningProcess->pid);
        DataType type;
        void *data = fetchDataByIndex(pcb_key, &type);
        PCB *pcb = (type == TYPE_PCB && data) ? (PCB *)data : NULL;

        char key[32];
        snprintf(key, sizeof(key), "P%d_Instruction_%d", runningProcess->pid, pcb ? pcb->programCounter + 1 : 1);
        char *instruction = fetchDataByIndex(key, &type);

        if (instruction)
        {
            console_model_log_output("[STATE] Process %d running instruction: %s\n", runningProcess->pid, instruction);
        }
    }

    // Log blocked processes
    if (global_blocked_queue->front)
    {
        GString *blocked_str = g_string_new("Blocked processes: ");
        Process *curr = global_blocked_queue->front;
        while (curr)
        {
            g_string_append_printf(blocked_str, "P%d", curr->pid);
            curr = curr->next;
            if (curr)
                g_string_append(blocked_str, ", ");
        }
        console_model_log_output("[STATE] %s\n", blocked_str->str);
        g_string_free(blocked_str, TRUE);
    }

    for (int i = 0; i < MAX_NUM_QUEUES; i++)
    {
        controller_update_queue_display(i);
    }

    controller_update_blocked_queue_display();
    controller_update_running_process();
}

// Run the selected scheduling algorithm
void run_selected_scheduler()
{
    if (strcmp(schedulingAlgorithm, "FCFS") == 0)
    {
        console_model_log_output("[SCHEDULER] Running FCFS algorithm\n");
        runFCFS();
    }
    else if (strcmp(schedulingAlgorithm, "Round Robin") == 0)
    {
        console_model_log_output("[SCHEDULER] Running Round Robin algorithm with quantum %d\n", controller->quantum);
        runRR(controller->quantum);
    }
    else
    {
        // Default case
        console_model_log_output("[SCHEDULER] Running MLFQ algorithm\n");
        runMLFQ();
    }
}

void controller_init(GtkApplication *app, GtkWidget *window, GtkWidget *main_box)
{
    controller = g_new0(Controller, 1);
    controller->view_window = window;
    view_init(window, main_box);
    controller->running_process_label = view_get_running_process_label();
    controller->step_button = view_get_step_button();
    controller->automatic_button = view_get_automatic_button();
    controller->pause_button = view_get_pause_button();
    controller->reset_button = view_get_reset_button();
    controller->scheduler_combo = view_get_scheduler_combo();
    controller->quantum_entry = view_get_quantum_entry();
    controller->quantum_label = view_get_quantum_label();
    controller->automatic_timer_id = 0;
    controller->is_running = FALSE;
    schedulingAlgorithm = g_strdup("MLFQ"); // Initialize with default algorithm
    controller->quantum = 2;

    // Initialize the clock controller
    clock_controller_init();

    gtk_window_set_application(GTK_WINDOW(controller->view_window), app);

    g_signal_connect(controller->scheduler_combo, "notify::selected", G_CALLBACK(on_scheduler_changed), NULL);
    g_signal_connect(controller->step_button, "clicked", G_CALLBACK(on_step_clicked), NULL);
    g_signal_connect(controller->automatic_button, "clicked", G_CALLBACK(on_automatic_clicked), NULL);
    g_signal_connect(controller->pause_button, "clicked", G_CALLBACK(on_pause_clicked), NULL);
    g_signal_connect(controller->reset_button, "clicked", G_CALLBACK(on_reset_clicked), NULL);

    populateMemory();
    controller_update_all();
}

static void on_scheduler_changed(GtkWidget *combo, GParamSpec *pspec, gpointer user_data)
{
    if (controller->is_running)
    {
        // Don't allow changing scheduler while running
        return;
    }

    guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(combo));

    // Set new algorithm based on selection
    if (selected == 0)
    {
        schedulingAlgorithm = g_strdup("MLFQ");
        gtk_widget_set_visible(controller->quantum_label, FALSE);
        gtk_widget_set_visible(controller->quantum_entry, FALSE);
        console_model_log_output("[CONFIG] Scheduler set to MLFQ\n");
    }
    else if (selected == 1)
    {
        schedulingAlgorithm = g_strdup("FCFS");
        gtk_widget_set_visible(controller->quantum_label, FALSE);
        gtk_widget_set_visible(controller->quantum_entry, FALSE);
        console_model_log_output("[CONFIG] Scheduler set to FCFS\n");
    }
    else if (selected == 2)
    {
        schedulingAlgorithm = g_strdup("Round Robin");
        gtk_widget_set_visible(controller->quantum_label, TRUE);
        gtk_widget_set_visible(controller->quantum_entry, TRUE);
        console_model_log_output("[CONFIG] Scheduler set to Round Robin\n");
    }
}

// Handle step button click
static void on_step_clicked(GtkWidget *button, gpointer user_data)
{

    populateMemory();
    if (numberOfProcesses <= 0)
    {
        clockCycle++;
        return;
    }
    printf("\nTime %d: \n \n", clockCycle);

    // Check if any processes are still running
    int any_running = 0;

    for (int i = 1; i <= numberOfProcesses; i++)
    {
        if (getProcessState(i) != TERMINATED)
        {
            any_running = 1;
            break;
        }
    }

    if (!isEmpty(job_pool))
        any_running = 1;

    if (any_running)
    {
        controller->is_running = TRUE;
        gtk_widget_set_sensitive(controller->scheduler_combo, FALSE);
        gtk_widget_set_sensitive(controller->quantum_entry, FALSE);

        // Get quantum value if using Round Robin
        if (schedulingAlgorithm && strcmp(schedulingAlgorithm, "Round Robin") == 0)
        {
            const char *quantum_text = gtk_editable_get_text(GTK_EDITABLE(controller->quantum_entry));
            controller->quantum = atoi(quantum_text);
            if (controller->quantum <= 0)
                controller->quantum = 2;
        }

        console_model_log_output("[STEP] Executing single step at cycle %d\n", clockCycle);

        // Update clock cycle
        if (!clock_controller_increment())
        {
            // All processes terminated
            controller->is_running = FALSE;
            gtk_widget_set_sensitive(controller->scheduler_combo, TRUE);
            gtk_widget_set_sensitive(controller->quantum_entry, TRUE);
            gtk_widget_set_sensitive(button, FALSE);
            gtk_widget_set_sensitive(controller->automatic_button, FALSE);
            console_model_log_output("[STEP] All processes terminated\n");
        }
    }
    else
    {
        controller->is_running = FALSE;
        gtk_widget_set_sensitive(controller->scheduler_combo, TRUE);
        gtk_widget_set_sensitive(controller->quantum_entry, TRUE);
        gtk_widget_set_sensitive(button, FALSE);
        gtk_widget_set_sensitive(controller->automatic_button, FALSE);
        console_model_log_output("[STEP] No processes to run\n");
    }
}

static void on_automatic_clicked(GtkWidget *button, gpointer user_data)
{
    populateMemory();
    if (numberOfProcesses <= 0)
    {
        clockCycle++;
        return;
    }
    if (controller->automatic_timer_id == 0)
    {
        controller->is_running = TRUE;
        gtk_widget_set_sensitive(controller->scheduler_combo, FALSE);
        gtk_widget_set_sensitive(controller->quantum_entry, FALSE);

        // Get quantum value if using Round Robin
        if (schedulingAlgorithm && strcmp(schedulingAlgorithm, "Round Robin") == 0)
        {
            const char *quantum_text = gtk_editable_get_text(GTK_EDITABLE(controller->quantum_entry));
            controller->quantum = atoi(quantum_text);
            if (controller->quantum <= 0)
                controller->quantum = 2;
        }

        console_model_log_output("[AUTO] Starting automatic execution\n");
        controller->automatic_timer_id = g_timeout_add(1500, automatic_step, NULL);
        gtk_widget_set_sensitive(controller->automatic_button, FALSE);
        gtk_widget_set_sensitive(controller->pause_button, TRUE);
        gtk_widget_set_sensitive(controller->step_button, FALSE);
    }
}

static void on_pause_clicked(GtkWidget *button, gpointer user_data)
{
    if (controller->automatic_timer_id != 0)
    {
        g_source_remove(controller->automatic_timer_id);
        controller->automatic_timer_id = 0;
        controller->is_running = FALSE;
        gtk_widget_set_sensitive(controller->scheduler_combo, TRUE);
        gtk_widget_set_sensitive(controller->quantum_entry, TRUE);
        gtk_widget_set_sensitive(controller->automatic_button, TRUE);
        gtk_widget_set_sensitive(controller->pause_button, FALSE);
        gtk_widget_set_sensitive(controller->step_button, TRUE);
        console_model_log_output("[AUTO] Paused automatic execution at cycle %d\n", clockCycle);
    }
}

static void on_reset_clicked(GtkWidget *button, gpointer user_data)
{
    if (controller->automatic_timer_id != 0)
    {
        g_source_remove(controller->automatic_timer_id);
        controller->automatic_timer_id = 0;
    }

    controller->is_running = FALSE;
    controller->quantum = 2;

    // Free previous algorithm and set default
    if (schedulingAlgorithm)
    {
        g_free(schedulingAlgorithm);
    }
    schedulingAlgorithm = g_strdup("MLFQ");

    // Reset clock to 0
    clock_controller_reset();

    runningProcess = NULL;

    freeMemoryWord();

    for (int i = 0; i < MAX_NUM_QUEUES; i++)
    {
        while (readyQueues[i]->front != NULL)
        {
            Process *p = readyQueues[i]->front;
            readyQueues[i]->front = p->next;
            free(p);
        }
        readyQueues[i]->rear = NULL;
    }

    while (global_blocked_queue->front != NULL)
    {
        Process *p = global_blocked_queue->front;
        global_blocked_queue->front = p->next;
        free(p);
    }
    global_blocked_queue->rear = NULL;

    for (int i = 0; i < 5; i++)
    {
        g_list_free(view->queue_processes[i]);
        view->queue_processes[i] = NULL;
        g_list_free_full(queue_animations[i].animations, g_free);
        queue_animations[i].animations = NULL;
        gtk_widget_queue_draw(view->drawing_areas[i]);
    }

    view->running_pid = -1;
    gtk_label_set_text(GTK_LABEL(controller->running_process_label), "Running Process: None");
    gtk_drop_down_set_selected(GTK_DROP_DOWN(controller->scheduler_combo), 0);
    gtk_editable_set_text(GTK_EDITABLE(controller->quantum_entry), "2");
    gtk_widget_set_visible(controller->quantum_label, FALSE);
    gtk_widget_set_visible(controller->quantum_entry, FALSE);
    gtk_widget_set_sensitive(controller->scheduler_combo, TRUE);
    gtk_widget_set_sensitive(controller->quantum_entry, TRUE);
    gtk_widget_set_sensitive(controller->step_button, TRUE);
    gtk_widget_set_sensitive(controller->automatic_button, TRUE);
    gtk_widget_set_sensitive(controller->pause_button, FALSE);

    // Reset console view
    console_controller_reset_view();

    console_model_log_output("[RESET] Simulation reset to initial state\n");

    // Update all displays
    controller_update_all();
}

// Timer callback for automatic execution called periodically to execute steps automatically
static gboolean automatic_step(gpointer user_data)
{
    // Check if any processes are still running
    int any_running = 0;
    for (int i = 1; i <= numberOfProcesses; i++)
        for (int i = 1; i <= numberOfProcesses; i++)
        {
            if (getProcessState(i) != TERMINATED)
            {
                any_running = 1;
                break;
            }
        }

    if (any_running)
    {
        console_model_log_output("[AUTO] Executing automatic step at cycle %d\n", clockCycle);

        run_selected_scheduler();

        // Update clock cycle which also updates all UI components
        if (!clock_controller_increment())
        {
            // All processes terminated - stop automatic execution
            controller->automatic_timer_id = 0;
            controller->is_running = FALSE;
            gtk_widget_set_sensitive(controller->scheduler_combo, TRUE);
            gtk_widget_set_sensitive(controller->quantum_entry, TRUE);
            gtk_widget_set_sensitive(controller->automatic_button, FALSE);
            gtk_widget_set_sensitive(controller->pause_button, FALSE);
            gtk_widget_set_sensitive(controller->step_button, FALSE);
            console_model_log_output("[AUTO] All processes terminated, automatic execution stopped\n");
            return G_SOURCE_REMOVE;
        }

        return G_SOURCE_CONTINUE;
    }
    else
    {
        controller->automatic_timer_id = 0;
        controller->is_running = FALSE;
        gtk_widget_set_sensitive(controller->scheduler_combo, TRUE);
        gtk_widget_set_sensitive(controller->quantum_entry, TRUE);
        gtk_widget_set_sensitive(controller->automatic_button, FALSE);
        gtk_widget_set_sensitive(controller->pause_button, FALSE);
        gtk_widget_set_sensitive(controller->step_button, FALSE);
        console_model_log_output("[AUTO] No processes to run, automatic execution stopped\n");
        return G_SOURCE_REMOVE;
    }
}

void controller_cleanup()
{
    if (controller)
    {
        if (controller->automatic_timer_id != 0)
        {
            g_source_remove(controller->automatic_timer_id);
            controller->automatic_timer_id = 0;
        }

        g_free(controller);
        controller = NULL;
    }
}