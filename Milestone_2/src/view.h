#ifndef VIEW_H
#define VIEW_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *drawing_areas[5];
    GtkWidget *running_process_label;
    GtkWidget *step_button;
    GtkWidget *automatic_button;
    GtkWidget *pause_button;
    GtkWidget *scheduler_combo;
    GtkWidget *quantum_entry;
    GtkWidget *quantum_label;
    GList *queue_processes[5];
    int running_pid;
} View;

GtkWidget* view_init();
void view_update_queue(int queue_index, GList *processes, int running_pid);
GtkWidget* view_get_running_process_label();
GtkWidget* view_get_step_button();
GtkWidget* view_get_automatic_button();
GtkWidget* view_get_pause_button();
GtkWidget* view_get_scheduler_combo();
GtkWidget* view_get_quantum_entry();
GtkWidget* view_get_quantum_label();

#endif