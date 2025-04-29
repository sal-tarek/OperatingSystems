#ifndef SIMULATOR_VIEW_H
#define SIMULATOR_VIEW_H

#include <gtk/gtk.h>

typedef struct {
    GtkWindow *window;
    GtkListBox *job_pool_display;
    GtkListBox *memory_list;
    GtkWindow *dialog;
    GtkTextView *dialog_text_view;
    GtkEntry *file_entry;
    GtkEntry *arrival_entry;
} SimulatorView;

SimulatorView *simulator_view_new(GtkApplication *app);
void simulator_view_update_job_pool(SimulatorView *view);
void simulator_view_update_memory(SimulatorView *view);
void simulator_view_show_process_dialog(SimulatorView *view, GCallback callback, gpointer user_data);
void simulator_view_get_process_input(SimulatorView *view, char **file_path, int *arrival_time);
void simulator_view_append_dialog_text(SimulatorView *view, const char *text);
void simulator_view_free(SimulatorView *view);

#endif // SIMULATOR_VIEW_H