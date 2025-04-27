#ifndef VIEW_H
#define VIEW_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *drawing_areas[5]; // Drawing areas for 4 ready queues + 1 blocked queue
    GtkWidget *running_process_label;
    GtkWidget *step_button;
    GtkWidget *automatic_button; // Button for automatic mode
    GtkWidget *pause_button;    // Button for pause
    GList *queue_processes[5];  // Process lists for 4 ready queues + 1 blocked queue
    int running_pid; // PID of the running process (-1 if none)
} View;

GtkWidget* view_init();
GtkWidget* view_get_running_process_label();
GtkWidget* view_get_step_button();
GtkWidget* view_get_automatic_button();
GtkWidget* view_get_pause_button();
void view_update_queue(int queue_index, GList *processes, int running_pid);

#endif