#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <gtk/gtk.h>

typedef struct _DashboardView DashboardView;

// Structure to hold references to Process List Section widgets
typedef struct {
    GtkWidget *process_list_box; // Horizontal box containing process entries
} ProcessListWidgets;

// View structure
struct _DashboardView {
    GtkWidget *window;
    GtkWidget *overview_frame;
    GtkWidget *process_count_label;
    GtkWidget *clock_cycle_label;
    GtkWidget *algorithm_label;
    ProcessListWidgets *process_list_widgets;
};

// Functions to create, initialize, and show the view
DashboardView* dashboard_view_new(void);
void dashboard_view_init(DashboardView *view, GtkApplication *app);
void dashboard_view_show(DashboardView *view);

// Functions to update the view
void dashboard_view_set_process_count(DashboardView *view, int count);
void dashboard_view_set_clock_cycle(DashboardView *view, int cycle);
void dashboard_view_set_algorithm(DashboardView *view, const char *algorithm);

// Function to add a process to the Process List
void dashboard_view_add_process(DashboardView *view, int pid, const char *state, int priority, 
                               int mem_lower, int mem_upper, int program_counter);

// Function to free the view
void dashboard_view_free(DashboardView *view);

#endif // DASHBOARD_VIEW_H