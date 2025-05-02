/*
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "console_view.h"
#include "memory_manager.h"
#include "process.h"
#include "Queue.h"
#include "memory.h"
#include "MLFQ.h"
#include "RoundRobin.h"
#include "FCFS.h"
#include "index.h"
#include "PCB.h"
#include "mutex.h"

#define numProcesses 3
#define numQueues 4

// Global variables
Queue *readyQueues[numQueues];  // Ready Queue holding processes waiting to run by the chosen Scheduler
Process *runningProcess = NULL; // currently running process (or NULL if none)
int clockCycle;                 // current clock cycle of the simulation
Queue *job_pool = NULL;
MemoryWord *memory = NULL;
IndexEntry *index_table = NULL;
Queue *global_blocked_queue = NULL;
int simulation_running = 1;  // Flag to control simulation loop
int selected_scheduler = -1; // 0=FCFS, 1=RR, 2=MLFQ
int quantum = 2;             // Default quantum for RR

FILE *logFile;

// GUI Global variables

GtkWidget *console = NULL;
GtkWidget *entry = NULL;
GAsyncQueue *input_queue = NULL;
GAsyncQueue *action_queue = NULL;

// Callback for key press events on the window
static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType state, gpointer user_data)
{
    if (gtk_widget_has_focus(entry))
    {
        return FALSE; // Let entry handle Enter
    }

    if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter)
    {
        char *action = g_strdup("enter_action");
        g_async_queue_push(action_queue, action);
        return TRUE;
    }
    return FALSE;
}

// Activate function
static void activate(GtkApplication *app, gpointer user_data)
{
    input_queue = g_async_queue_new();
    action_queue = g_async_queue_new();

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "OS Simulation");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkEventController *key_controller = gtk_event_controller_key_new();
    gtk_widget_add_controller(window, key_controller);
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), NULL);

    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(content_box, 10);
    gtk_widget_set_margin_end(content_box, 10);
    gtk_widget_set_margin_top(content_box, 10);
    gtk_widget_set_margin_bottom(content_box, 10);
    gtk_widget_set_hexpand(content_box, TRUE);
    gtk_widget_set_vexpand(content_box, TRUE);

    console = console_view_new(&entry);
    gtk_widget_set_hexpand(console, TRUE);
    gtk_widget_set_vexpand(console, TRUE);
    gtk_box_append(GTK_BOX(content_box), console);

    g_signal_connect(entry, "activate", G_CALLBACK(on_entry_activate), NULL);

    gtk_window_set_child(GTK_WINDOW(window), content_box);

    pthread_t ui_tid;
    pthread_create(&ui_tid, NULL, ui_thread, NULL);

    gtk_window_present(GTK_WINDOW(window));
}

// Main function
int main(int argc, char *argv[])
{
    logFile = fopen("G:/SEM6/CSEN_602/Project/OperatingSystems/Milestone_2/log/log.txt", "w");

    GtkApplication *app = gtk_application_new("org.example.ossimulation", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    console_view_cleanup();
    if (input_queue)
        g_async_queue_unref(input_queue);
    if (action_queue)
        g_async_queue_unref(action_queue);
    g_object_unref(app);

    console_view_cleanup();
    return status;
}
*/