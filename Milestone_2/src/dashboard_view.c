#include "dashboard_view.h"
#include <string.h>

DashboardView *dashboard_view_new(void) {
    DashboardView *view = g_new0(DashboardView, 1);
    return view;
}

void dashboard_view_init(DashboardView *view, GtkApplication *app, GtkWidget *parent_container, GtkWindow *window) {
    view->window = NULL;
    view->main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(view->main_container, 10);
    gtk_widget_set_margin_end(view->main_container, 10);
    gtk_widget_set_margin_bottom(view->main_container, 10);
    gtk_box_append(GTK_BOX(parent_container), view->main_container);

    GtkWidget *big_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_hexpand(big_container, TRUE);
    gtk_widget_set_vexpand(big_container, FALSE);
    gtk_box_append(GTK_BOX(view->main_container), big_container);

    GtkWidget *overview_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(overview_section, GTK_ALIGN_START);
    gtk_widget_set_valign(overview_section, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(overview_section, FALSE);
    gtk_widget_set_margin_top(overview_section, 20);
    gtk_box_append(GTK_BOX(big_container), overview_section);

    view->overview_frame = gtk_frame_new("Overview");
    gtk_widget_add_css_class(view->overview_frame, "dashboard-frame");
    gtk_widget_set_hexpand(view->overview_frame, FALSE);
    gtk_box_append(GTK_BOX(overview_section), view->overview_frame);

    GtkWidget *overview_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(overview_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(overview_grid), 10);
    gtk_widget_set_margin_start(overview_grid, 10);
    gtk_widget_set_margin_end(overview_grid, 10);
    gtk_widget_set_margin_top(overview_grid, 10);
    gtk_widget_set_margin_bottom(overview_grid, 10);
    gtk_frame_set_child(GTK_FRAME(view->overview_frame), overview_grid);

    GtkWidget *process_label = gtk_label_new("Total Processes:");
    gtk_widget_set_halign(process_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), process_label, 0, 0, 1, 1);

    view->process_count_label = gtk_label_new("0");
    gtk_widget_set_halign(view->process_count_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), view->process_count_label, 1, 0, 1, 1);

    GtkWidget *clock_label = gtk_label_new("Current Clock Cycle:");
    gtk_widget_set_halign(clock_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), clock_label, 0, 1, 1, 1);

    view->clock_cycle_label = gtk_label_new("0");
    gtk_widget_set_halign(view->clock_cycle_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), view->clock_cycle_label, 1, 1, 1, 1);

    GtkWidget *algorithm_text = gtk_label_new("Active Algorithm:");
    gtk_widget_set_halign(algorithm_text, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), algorithm_text, 0, 2, 1, 1);

    view->algorithm_label = gtk_label_new("None");
    gtk_widget_set_halign(view->algorithm_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(overview_grid), view->algorithm_label, 1, 2, 1, 1);

    GtkWidget *process_list_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(process_list_section, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(process_list_section, TRUE);
    gtk_box_append(GTK_BOX(big_container), process_list_section);

    GtkWidget *process_list_frame = gtk_frame_new("Process List");
    gtk_widget_add_css_class(process_list_frame, "dashboard-process-list-frame");
    gtk_box_append(GTK_BOX(process_list_section), process_list_frame);

    GtkWidget *process_list_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_frame_set_child(GTK_FRAME(process_list_frame), process_list_content);

    GtkWidget *process_list_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_hexpand(process_list_box, TRUE);

    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), process_list_box);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_size_request(scrolled_window, -1, 150);
    gtk_box_append(GTK_BOX(process_list_content), scrolled_window);

    view->process_list_widgets = g_new(ProcessListWidgets, 1);
    view->process_list_widgets->process_list_box = process_list_box;

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        ".dashboard-frame { background-color: rgb(236, 236, 234); border: 1px solid  #196761; color: #196761; padding:10px; }"
        ".dashboard-process-list-frame { background-color: #f5f5f5; color: #196761; font-weight:bold; letter-spacing: 0.5px; }"
        ".dashboard-label { color: #333; font-size: 14px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; }"
        ".dashboard-frame > label, .dashboard-box > label { font-weight: bold; font-size: 14px; color: white; background-color: #33A19A; padding: 5px; border-radius: 3px 3px 0 0; }"
        ".dashboard-big-container { background-color: #33A19A; padding: 10px; border-radius: 5px; }"
        ".dashboard-pcb-tab { background-color: #33A19A; color: white; padding: 6px 12px; border-radius: 5px 5px 0 0; font-weight: bold; margin-bottom: 0; margin-top: 5px; margin-right: 2px; }"
        ".dashboard-memory-pcb { background-color: #f5f5f5; border: 1px solid #33A19A; border-radius: 0 0 5px 5px; padding: 0; margin-bottom: 7px; margin-right: 2px; }"
        ".dashboard-memory-pcb-row { color: #333; padding: 5px 10px; border-bottom: 1px solid rgba(51, 161, 154, 0.2); }"
        ".dashboard-memory-pcb-row:last-child { border-bottom: none; }"
    );

    gtk_widget_add_css_class(big_container, "dashboard-big-container");
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(GTK_WIDGET(window)), // Use the passed window's display
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

void dashboard_view_show(DashboardView *view) {
    gtk_widget_set_visible(view->main_container, TRUE);
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

void dashboard_view_add_process(DashboardView *view, int pid, const char *state, int priority,
                               int mem_lower, int mem_upper, int program_counter) {
    GtkWidget *pcb_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(pcb_container, FALSE);

    char pcb_label[32];
    snprintf(pcb_label, sizeof(pcb_label), "Process %d", pid);
    GtkWidget *pcb_tab = gtk_label_new(pcb_label);
    gtk_widget_add_css_class(pcb_tab, "dashboard-pcb-tab");
    gtk_box_append(GTK_BOX(pcb_container), pcb_tab);

    GtkWidget *pcb_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(pcb_box, "dashboard-memory-pcb");
    gtk_widget_set_hexpand(pcb_box, FALSE);

    char state_info[64], priority_info[64], pc_info[64], mem_bounds[64];
    snprintf(state_info, sizeof(state_info), "State: %s", state ? state : "UNKNOWN");
    snprintf(priority_info, sizeof(priority_info), "Priority: %d", priority);
    snprintf(pc_info, sizeof(pc_info), "PC: %d", program_counter);
    snprintf(mem_bounds, sizeof(mem_bounds), "Mem Bounds: %d-%d", mem_lower, mem_upper);

    GtkWidget *state_row = gtk_label_new(state_info);
    gtk_label_set_xalign(GTK_LABEL(state_row), 0);
    gtk_widget_add_css_class(state_row, "dashboard-memory-pcb-row");
    gtk_box_append(GTK_BOX(pcb_box), state_row);

    GtkWidget *priority_row = gtk_label_new(priority_info);
    gtk_label_set_xalign(GTK_LABEL(priority_row), 0);
    gtk_widget_add_css_class(priority_row, "dashboard-memory-pcb-row");
    gtk_box_append(GTK_BOX(pcb_box), priority_row);

    GtkWidget *pc_row = gtk_label_new(pc_info);
    gtk_label_set_xalign(GTK_LABEL(pc_row), 0);
    gtk_widget_add_css_class(pc_row, "dashboard-memory-pcb-row");
    gtk_box_append(GTK_BOX(pcb_box), pc_row);

    GtkWidget *mem_row = gtk_label_new(mem_bounds);
    gtk_label_set_xalign(GTK_LABEL(mem_row), 0);
    gtk_widget_add_css_class(mem_row, "dashboard-memory-pcb-row");
    gtk_box_append(GTK_BOX(pcb_box), mem_row);

    gtk_box_append(GTK_BOX(pcb_container), pcb_box);
    gtk_box_append(GTK_BOX(view->process_list_widgets->process_list_box), pcb_container);
    gtk_widget_set_visible(pcb_container, TRUE);
}

void dashboard_view_free(DashboardView *view) {
    if (view) {
        g_free(view->process_list_widgets);
        g_free(view);
    }
}