#ifndef VIEW_H
#define VIEW_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *drawing_areas[4]; // Drawing areas for each queue
    GtkWidget *running_process_label;
    GtkWidget *step_button;
    GtkWidget *automatic_button; //for automatic mode
    GtkWidget *pause_button;    //to pause automatic mode
    GList *queue_processes[4]; 
    int running_pid; // PID of the running process (-1 if none)
} View;

GtkWidget* view_init();
GtkWidget* view_get_running_process_label();
GtkWidget* view_get_step_button();
GtkWidget* view_get_automatic_button(); 
GtkWidget* view_get_pause_button();     
void view_update_queue(int queue_index, GList *processes, int running_pid);

#endif