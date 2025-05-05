#ifndef SIMULATOR_VIEW_H
#define SIMULATOR_VIEW_H

#include <gtk/gtk.h>

typedef struct {
    GtkWindow *window;
    GtkListBox *job_pool_display;
    GtkListBox *memory_list;
    GtkWindow *dialog;
    GtkEntry *file_entry;
    GtkEntry *arrival_entry;
    GtkTextView *dialog_text_view;
    GtkWidget *main_container;
    GtkWindow *main_window; // Added to store main application window
} SimulatorView;

SimulatorView *simulator_view_new(GtkApplication *app, GtkWidget *parent_container, GtkWindow *main_window);
void simulator_view_show(SimulatorView *view);
void simulator_view_connect_create_process(SimulatorView *view, GCallback callback, gpointer user_data);
void simulator_view_update_job_pool(SimulatorView *view);
void simulator_view_update_memory(SimulatorView *view);
void simulator_view_get_process_input(SimulatorView *view, char **file_path, int *arrival_time);
void simulator_view_append_dialog_text(SimulatorView *view, const char *text);
void simulator_view_free(SimulatorView *view);

#endif