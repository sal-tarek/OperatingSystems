#ifndef CLOCK_CONTROLLER_H
#define CLOCK_CONTROLLER_H

#include <gtk/gtk.h>

void clock_controller_init(void);
gboolean clock_controller_increment();
void clock_controller_reset(void);

#endif /* CLOCK_CONTROLLER_H */