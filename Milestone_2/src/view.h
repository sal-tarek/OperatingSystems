#ifndef VIEW_H
#define VIEW_H
#include <gtk/gtk.h>

GtkWidget* view_init();
GtkWidget* view_get_queue_label(int queue_index);
GtkWidget* view_get_running_process_label();
GtkWidget* view_get_step_button();

#endif // VIEW_H