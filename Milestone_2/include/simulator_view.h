#ifndef SIMULATOR_VIEW_H
#define SIMULATOR_VIEW_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *mutex_status_box;
    GtkWidget *blocked_queues_box;
    GtkWidget *mutex_labels[3];  // Lama For userInput, userOutput, file
    GtkWidget *blocked_queue_labels[3];
} ResourcePanel;

typedef struct {
    GtkWindow *window;
    GtkListBox *job_pool_display;
    GtkListBox *memory_list;
    GtkWindow *dialog;
    GtkEntry *file_entry;
    GtkEntry *arrival_entry;
    GtkTextView *dialog_text_view;
    GtkWidget *main_container;
    GtkWindow *main_window;
    ResourcePanel *resource_panel;  //Lama Added for resource management
} SimulatorView;

SimulatorView *simulator_view_new(GtkApplication *app, GtkWidget *parent_container, GtkWindow *main_window);
void simulator_view_show(SimulatorView *view);
void simulator_view_connect_create_process(SimulatorView *view, GCallback callback, gpointer user_data);
void simulator_view_update_job_pool(SimulatorView *view);
void simulator_view_update_memory(SimulatorView *view);
void simulator_view_get_process_input(SimulatorView *view, char **file_path, int *arrival_time);
void simulator_view_append_dialog_text(SimulatorView *view, const char *text);
void simulator_view_free(SimulatorView *view);

// Lama New functions for resource management panel
void simulator_view_update_resource_panel(SimulatorView *view);
void simulator_view_create_resource_panel(SimulatorView *view, GtkWidget *parent);
void simulator_view_reset_resource_panel(SimulatorView *view);
#endif