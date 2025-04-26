#include <gtk/gtk.h>
#include "view.h"
#include <glib.h>
#include "process.h"
#include "Queue.h"
#include "MLFQ.h"
#include "memory.h"
#include "PCB.h"
#include "../include/instruction.h"
#include "memory_manager.h"

// Controller structure to hold references to View and Model data
typedef struct {
    GtkWidget *view_window;
    GtkWidget *running_process_label;
    GtkWidget *step_button;
    GtkWidget *automatic_button; // New button for automatic mode
    GtkWidget *pause_button;    // New button to pause automatic mode
    guint automatic_timer_id;   // Timer ID for automatic loop
} Controller;

static Controller *controller = NULL;

static void on_step_clicked(GtkWidget *button, gpointer user_data);
static void on_automatic_clicked(GtkWidget *button, gpointer user_data);
static void on_pause_clicked(GtkWidget *button, gpointer user_data);
static gboolean automatic_step(gpointer user_data);

void controller_init(GtkApplication *app) {
    controller = g_new0(Controller, 1);

    controller->view_window = view_init();
    controller->running_process_label = view_get_running_process_label();
    controller->step_button = view_get_step_button();
    controller->automatic_button = view_get_automatic_button();
    controller->pause_button = view_get_pause_button();
    controller->automatic_timer_id = 0; // Initialize timer ID

    gtk_window_set_application(GTK_WINDOW(controller->view_window), app);

    // Connect button signals
    g_signal_connect(controller->step_button, "clicked", G_CALLBACK(on_step_clicked), NULL);
    g_signal_connect(controller->automatic_button, "clicked", G_CALLBACK(on_automatic_clicked), NULL);
    g_signal_connect(controller->pause_button, "clicked", G_CALLBACK(on_pause_clicked), NULL);
}

// Update queue display for a specific queue
void controller_update_queue_display(int queue_index) {
    if (queue_index < 0 || queue_index >= numQueues) return;

    // Collect PIDs into a GList
    GList *pid_list = NULL;
    Process *curr = readyQueues[queue_index]->front;
    while (curr != NULL) {
        pid_list = g_list_append(pid_list, GINT_TO_POINTER(curr->pid));
        curr = curr->next;
    }

    // Determine running PID
    int running_pid = (runningProcess != NULL) ? runningProcess->pid : -1;

    // Update the view
    view_update_queue(queue_index, pid_list, running_pid);

    // Free the list (view_update_queue makes a copy)
    g_list_free(pid_list);
}

// Update running process display
void controller_update_running_process() {
    GString *process_str = g_string_new("");

    if (runningProcess != NULL) {
        // Fetch PCB for the running process
        char pcb_key[32];
        snprintf(pcb_key, sizeof(pcb_key), "P%d_PCB", runningProcess->pid);
        DataType type;
        void *data = fetchDataByIndex(pcb_key, &type);
        PCB *pcb = (type == TYPE_PCB && data) ? (PCB *)data : NULL;

        // Fetch current instruction for the running process
        char key[32];
        snprintf(key, sizeof(key), "P%d_Instruction_%d", runningProcess->pid, pcb ? pcb->programCounter + 1 : 1);
        char *instruction = fetchDataByIndex(key, &type);

        if (!instruction) {
            g_string_append_printf(process_str, "Running Process: PID=%d, Instruction=N/A", (int)runningProcess->pid);
            fprintf(stderr, "Failed to fetch instruction for key: %s\n", key);
        } else {
            g_string_append_printf(process_str, "Running Process: PID=%d, Instruction=%s", (int)runningProcess->pid, instruction);
        }
    } else {
        g_string_append(process_str, "Running Process: None");
    }

    gtk_label_set_text(GTK_LABEL(controller->running_process_label), process_str->str);
    g_string_free(process_str, TRUE);
}

// Update all displays (called per clock cycle)
void controller_update_all() {
    for (int i = 0; i < numQueues; i++) {
        controller_update_queue_display(i);
    }
    controller_update_running_process();
}

// Cleanup controller
void controller_cleanup() {
    if (controller) {
        // Remove automatic timer if active
        if (controller->automatic_timer_id != 0) {
            g_source_remove(controller->automatic_timer_id);
            controller->automatic_timer_id = 0;
        }
        g_free(controller);
        controller = NULL;
    }
}

// Step button callback
static void on_step_clicked(GtkWidget *button, gpointer user_data) {
    if (getProcessState(1) != TERMINATED || 
        getProcessState(2) != TERMINATED || 
        getProcessState(3) != TERMINATED) {
        populateMemory();
        runMLFQ();
        
        // Sleep
        g_usleep(50000);

        controller_update_all();
        clockCycle++;
    } else {
        gtk_widget_set_sensitive(button, FALSE); // Disable step button when done
        gtk_widget_set_sensitive(controller->automatic_button, FALSE); // Disable automatic button
    }
}

// Automatic button callback
static void on_automatic_clicked(GtkWidget *button, gpointer user_data) {
    if (controller->automatic_timer_id == 0) {
        // Start automatic mode
        controller->automatic_timer_id = g_timeout_add(1500, automatic_step, NULL); // 1.5 second interval
        gtk_widget_set_sensitive(controller->automatic_button, FALSE); // Disable automatic button
        gtk_widget_set_sensitive(controller->pause_button, TRUE);      // Enable pause button
        gtk_widget_set_sensitive(controller->step_button, FALSE);      // Disable step button
    }
}

// Pause button callback
static void on_pause_clicked(GtkWidget *button, gpointer user_data) {
    if (controller->automatic_timer_id != 0) {
        // Stop automatic mode
        g_source_remove(controller->automatic_timer_id);
        controller->automatic_timer_id = 0;
        gtk_widget_set_sensitive(controller->automatic_button, TRUE);  // Enable automatic button
        gtk_widget_set_sensitive(controller->pause_button, FALSE);     // Disable pause button
        gtk_widget_set_sensitive(controller->step_button, TRUE);       // Enable step button
    }
}

// Automatic step callback
static gboolean automatic_step(gpointer user_data) {
    if (getProcessState(1) != TERMINATED || 
        getProcessState(2) != TERMINATED || 
        getProcessState(3) != TERMINATED) {
        populateMemory();
        runMLFQ();
        
        // Sleep
        g_usleep(100000);

        controller_update_all();
        clockCycle++;
        return G_SOURCE_CONTINUE; // Continue the timer
    } else {
        // All processes terminated, stop automatic mode
        controller->automatic_timer_id = 0;
        gtk_widget_set_sensitive(controller->automatic_button, FALSE); // Disable automatic button
        gtk_widget_set_sensitive(controller->pause_button, FALSE);     // Disable pause button
        gtk_widget_set_sensitive(controller->step_button, FALSE);      // Disable step button
        return G_SOURCE_REMOVE; // Stop the timer
    }
}

// Application activate callback
static void on_activate(GtkApplication *app, gpointer user_data) {
    controller_init(app);
    controller_update_all();
    gtk_window_present(GTK_WINDOW(controller->view_window));
}

// Main entry point to start the UI
int controller_start(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.os.scheduler", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    controller_cleanup();
    return status;
}