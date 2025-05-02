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

extern int numProcesses;
extern Process *runningProcess;
extern Queue *readyQueues[MAX_NUM_QUEUES];
extern Queue *global_blocked_queue;
extern int clockCycle;

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
    int selected_scheduler;
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
    for (int i = 0; i < MAX_NUM_QUEUES; i++)
    {
        controller_update_queue_display(i);
    }
    controller_update_blocked_queue_display();
    controller_update_running_process();
}

static void run_selected_scheduler()
{
    switch (controller->selected_scheduler)
    {
    case 0:
        runMLFQ();
        break;
    case 1:
        runFCFS();
        break;
    case 2:
        runRR(controller->quantum);
        break;
    }
}

void controller_init(GtkApplication *app)
{
    controller = g_new0(Controller, 1);

    controller->view_window = view_init();
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
    controller->selected_scheduler = 0;
    controller->quantum = 2;

    gtk_window_set_application(GTK_WINDOW(controller->view_window), app);

    g_signal_connect(controller->scheduler_combo, "notify::selected", G_CALLBACK(on_scheduler_changed), NULL);
    g_signal_connect(controller->step_button, "clicked", G_CALLBACK(on_step_clicked), NULL);
    g_signal_connect(controller->automatic_button, "clicked", G_CALLBACK(on_automatic_clicked), NULL);
    g_signal_connect(controller->pause_button, "clicked", G_CALLBACK(on_pause_clicked), NULL);
    g_signal_connect(controller->reset_button, "clicked", G_CALLBACK(on_reset_clicked), NULL);

    populateMemory();
    controller_update_all();
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

static void on_scheduler_changed(GtkWidget *combo, GParamSpec *pspec, gpointer user_data)
{
    if (controller->is_running)
    {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(combo), controller->selected_scheduler);
        return;
    }

    controller->selected_scheduler = gtk_drop_down_get_selected(GTK_DROP_DOWN(combo));
    if (controller->selected_scheduler == 2)
    {
        gtk_widget_set_visible(controller->quantum_label, TRUE);
        gtk_widget_set_visible(controller->quantum_entry, TRUE);
    }
    else
    {
        gtk_widget_set_visible(controller->quantum_label, FALSE);
        gtk_widget_set_visible(controller->quantum_entry, FALSE);
    }
}

static void on_step_clicked(GtkWidget *button, gpointer user_data)
{
    int any_running = 0;
    for (int i = 1; i <= numProcesses; i++)
    {
        if (getProcessState(i) != TERMINATED)
        {
            any_running = 1;
            break;
        }
    }
    if (any_running)
    {
        controller->is_running = TRUE;
        gtk_widget_set_sensitive(controller->scheduler_combo, FALSE);
        gtk_widget_set_sensitive(controller->quantum_entry, FALSE);

        if (controller->selected_scheduler == 2)
        {
            const char *quantum_text = gtk_editable_get_text(GTK_EDITABLE(controller->quantum_entry));
            controller->quantum = atoi(quantum_text);
            if (controller->quantum <= 0)
                controller->quantum = 2;
        }

        populateMemory();
        controller_update_all();
        run_selected_scheduler();
        g_usleep(50000);
        controller_update_all();
        clockCycle++;
    }
    else
    {
        controller->is_running = FALSE;
        gtk_widget_set_sensitive(controller->scheduler_combo, TRUE);
        gtk_widget_set_sensitive(controller->quantum_entry, TRUE);
        gtk_widget_set_sensitive(button, FALSE);
        gtk_widget_set_sensitive(controller->automatic_button, FALSE);
    }
}

static void on_automatic_clicked(GtkWidget *button, gpointer user_data)
{
    if (controller->automatic_timer_id == 0)
    {
        controller->is_running = TRUE;
        gtk_widget_set_sensitive(controller->scheduler_combo, FALSE);
        gtk_widget_set_sensitive(controller->quantum_entry, FALSE);

        if (controller->selected_scheduler == 2)
        {
            const char *quantum_text = gtk_editable_get_text(GTK_EDITABLE(controller->quantum_entry));
            controller->quantum = atoi(quantum_text);
            if (controller->quantum <= 0)
                controller->quantum = 2;
        }

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
    controller->selected_scheduler = 0;

    numProcesses = 0;
    clockCycle = 0;
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
}

static gboolean automatic_step(gpointer user_data)
{
    int any_running = 0;
    for (int i = 1; i <= numProcesses; i++)
    {
        if (getProcessState(i) != TERMINATED)
        {
            any_running = 1;
            break;
        }
    }
    if (any_running)
    {
        populateMemory();
        run_selected_scheduler();
        g_usleep(100000);
        controller_update_all();
        clockCycle++;
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
        return G_SOURCE_REMOVE;
    }
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
    controller_init(app);
    gtk_window_present(GTK_WINDOW(controller->view_window));
}

int controller_start(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("org.os.scheduler", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    controller_cleanup();
    return status;
}