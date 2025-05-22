#ifndef VIEW_H
#define VIEW_H

#include <gtk/gtk.h>

// Resource panel structure for mutex management UI
typedef struct {
    GtkWidget *mutex_status_box;
    GtkWidget *blocked_queues_box;
    GtkWidget *mutex_labels[3];             // For userInput, userOutput, file
    GtkWidget *mutex_drawing_areas[3];      // Drawing areas for mutex queues (Cairo-based)
} ResourcePanel;

typedef struct
{
    int pid;
    float alpha;
    float start_x;
    float end_x;
    float start_y;
    float end_y;
    int animating;
    int steps;
    int total_steps;
} ProcessAnimation;

typedef struct
{
    GList *animations;
} QueueAnimation;

typedef struct
{
    GtkWidget *window;
    GtkWidget *drawing_areas[5];
    GtkWidget *running_process_label;
    GtkWidget *step_button;
    GtkWidget *automatic_button;
    GtkWidget *pause_button;
    GtkWidget *reset_button;
    GtkWidget *scheduler_combo;
    GtkWidget *quantum_entry;
    GtkWidget *quantum_label;
    GList *queue_processes[5];
    GList *mutex_queue_processes[3];    // Process lists for mutex queues
    int running_pid;
    ResourcePanel *resource_panel; // Resource panel for mutex management
} View;

extern View *view; // Declare view as extern
extern QueueAnimation queue_animations[5];

void view_init(GtkWidget *window, GtkWidget *main_box) ;
void view_update_queue(int queue_index, GList *processes, int running_pid);
GtkWidget *view_get_running_process_label();
GtkWidget *view_get_step_button();
GtkWidget *view_get_automatic_button();
GtkWidget *view_get_pause_button();
GtkWidget *view_get_reset_button();
GtkWidget *view_get_scheduler_combo();
GtkWidget *view_get_quantum_entry();
GtkWidget *view_get_quantum_label();

// Mutex resource panel functions
void view_update_resource_panel(void);
void view_create_resource_panel(View *view, GtkWidget *parent);
void view_reset_resource_panel(void);

#endif // VIEW_H