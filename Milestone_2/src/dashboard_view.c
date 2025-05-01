#include "dashboard_view.h"
#include <string.h>

DashboardView* dashboard_view_new(void) {
    DashboardView *view = g_new0(DashboardView, 1);
    return view;
}

void dashboard_view_init(DashboardView *view, GtkApplication *app) {
    // Create the main window as an application window
    view->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(view->window), "Process Scheduler Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(view->window), 800, 600);

    // Create a main vertical box
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_vbox, 10);
    gtk_widget_set_margin_end(main_vbox, 10);
    gtk_widget_set_margin_top(main_vbox, 10);
    gtk_widget_set_margin_bottom(main_vbox, 10);
    gtk_window_set_child(GTK_WINDOW(view->window), main_vbox);

    // Create a horizontal box for the top section
    GtkWidget *top_section = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(main_vbox), top_section);

    // Create the overview frame
    view->overview_frame = gtk_frame_new("Overview");
    gtk_widget_set_halign(view->overview_frame, GTK_ALIGN_START);
    gtk_widget_set_hexpand(view->overview_frame, TRUE);
    gtk_box_append(GTK_BOX(top_section), view->overview_frame);

    // Create a grid for overview contents
    GtkWidget *overview_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(overview_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(overview_grid), 10);
    gtk_widget_set_margin_start(overview_grid, 10);
    gtk_widget_set_margin_end(overview_grid, 10);
    gtk_widget_set_margin_top(overview_grid, 10);
    gtk_widget_set_margin_bottom(overview_grid, 10);
    gtk_frame_set_child(GTK_FRAME(view->overview_frame), overview_grid);

    // Add labels for process count
    GtkWidget *process_label = gtk_label_new("Total Processes:");
    gtk_widget_set_halign(process_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), process_label, 0, 0, 1, 1);

    view->process_count_label = gtk_label_new("0");
    gtk_widget_set_halign(view->process_count_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), view->process_count_label, 1, 0, 1, 1);

    // Add labels for clock cycle
    GtkWidget *clock_label = gtk_label_new("Current Clock Cycle:");
    gtk_widget_set_halign(clock_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), clock_label, 0, 1, 1, 1);

    view->clock_cycle_label = gtk_label_new("0");
    gtk_widget_set_halign(view->clock_cycle_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), view->clock_cycle_label, 1, 1, 1, 1);

    // Add labels for algorithm
    GtkWidget *algorithm_text = gtk_label_new("Active Algorithm:");
    gtk_widget_set_halign(algorithm_text, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), algorithm_text, 0, 2, 1, 1);

    view->algorithm_label = gtk_label_new("None");
    gtk_widget_set_halign(view->algorithm_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), view->algorithm_label, 1, 2, 1, 1);

    // Apply CSS for styling
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "frame {"
        "   background-color: #33A19A;"
        "   padding: 10px;"
        "   border-radius: 5px;"
        "}"
        "label {"
        "   color: #196761;"
        "}"
        "frame > label {"
        "   font-weight: bold;"
        "   font-size: 16px;"
        "   color: #196761;"
        "}"
    );

    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(view->window),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

void dashboard_view_show(DashboardView *view) {
    gtk_window_present(GTK_WINDOW(view->window));
}

void dashboard_view_set_process_count(DashboardView *view, int count) {
    char count_str[32];
    snprintf(count_str, sizeof(count_str), "%d", count);
    gtk_label_set_text(GTK_LABEL(view->process_count_label), count_str);
}

void dashboard_view_set_clock_cycle(DashboardView *view, int cycle) {
    char cycle_str[32];
    snprintf(cycle_str, sizeof(cycle_str), "%d", cycle);
    gtk_label_set_text(GTK_LABEL(view->clock_cycle_label), cycle_str);
}

void dashboard_view_set_algorithm(DashboardView *view, const char *algorithm) {
    gtk_label_set_text(GTK_LABEL(view->algorithm_label), algorithm);
}

void dashboard_view_free(DashboardView *view) {
    if (view) {
        g_free(view);
    }
}