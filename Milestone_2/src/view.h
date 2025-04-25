#ifndef VIEW_H
#define VIEW_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *drawing_areas[4]; // Drawing areas for each queue
    GtkWidget *running_process_label;
    GtkWidget *step_button;
    GList *queue_processes[4]; 
    int running_pid; // PID of the running process (-1 if none)
} View;

GtkWidget* view_init();
GtkWidget* view_get_running_process_label();
GtkWidget* view_get_step_button();
void view_update_queue(int queue_index, GList *processes, int running_pid);

#endif