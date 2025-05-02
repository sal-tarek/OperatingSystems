#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gtk/gtk.h>

void controller_init(GtkApplication *app, GtkWidget *window, GtkWidget *main_box);
void controller_cleanup(void);
void controller_update_all(void);
void controller_update_queue_display(int queue_index);
void controller_update_blocked_queue_display(void);

#endif // CONTROLLER_H