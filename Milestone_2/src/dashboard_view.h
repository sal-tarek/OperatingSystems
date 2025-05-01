#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <gtk/gtk.h>

typedef struct _DashboardView DashboardView;

// View structure
struct _DashboardView {
    GtkWidget *window;
    GtkWidget *overview_frame;
    GtkWidget *process_count_label;
    GtkWidget *clock_cycle_label;
    GtkWidget *algorithm_label;
};

// Functions to create, initialize, and show the view
DashboardView* dashboard_view_new(void);
void dashboard_view_init(DashboardView *view, GtkApplication *app);
void dashboard_view_show(DashboardView *view);

// Functions to update the view
void dashboard_view_set_process_count(DashboardView *view, int count);
void dashboard_view_set_clock_cycle(DashboardView *view, int cycle);
void dashboard_view_set_algorithm(DashboardView *view, const char *algorithm);

// Function to free the view
void dashboard_view_free(DashboardView *view);

#endif // DASHBOARD_VIEW_H