#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *process_list_box;
} ProcessListWidgets;

typedef struct {
    GtkWidget *window;
    GtkWidget *main_container;
    GtkWidget *overview_frame;
    GtkWidget *process_count_label;
    GtkWidget *clock_cycle_label;
    GtkWidget *algorithm_label;
    ProcessListWidgets *process_list_widgets;
} DashboardView;

DashboardView *dashboard_view_new(void);
void dashboard_view_init(DashboardView *view, GtkApplication *app, GtkWidget *parent_container, GtkWindow *window);
void dashboard_view_show(DashboardView *view);
void dashboard_view_set_process_count(DashboardView *view, int count);
void dashboard_view_set_clock_cycle(DashboardView *view, int cycle);
void dashboard_view_set_algorithm(DashboardView *view, const char *algorithm);
void dashboard_view_add_process(DashboardView *view, int pid, const char *state, int priority, int mem_lower, int mem_upper, int program_counter);
void dashboard_view_free(DashboardView *view);

#endif // DASHBOARD_VIEW_H