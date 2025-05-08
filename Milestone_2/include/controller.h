#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gtk/gtk.h>
#include "unified_controller.h"// Make schedulingAlgorithm accessible to other files
extern char *schedulingAlgorithm;

void controller_init(GtkApplication *app, GtkWidget *window, GtkWidget *main_box, UnifiedController* unified_controller);
void controller_cleanup(void);
void controller_update_all(void);
void controller_update_queue_display(int queue_index);
void controller_update_blocked_queue_display(void);
void run_selected_scheduler(void);
void controller_update_running_process(char* instruction);

#endif // CONTROLLER_H