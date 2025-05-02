#ifndef CONTROLLER_H
#define CONTROLLER_H

int controller_start(int argc, char *argv[]);
void controller_update_all(void);
void controller_update_queue_display(int queue_index);
void controller_update_blocked_queue_display(void); // New function

#endif // CONTROLLER_H