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

    // Create the main container (vertical box)
    GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(main_container, 10);
    gtk_widget_set_margin_end(main_container, 10);
    gtk_widget_set_margin_bottom(main_container, 10);
    gtk_window_set_child(GTK_WINDOW(view->window), main_container);

    // Create the big container (horizontal box) with grey background
    GtkWidget *big_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_hexpand(big_container, TRUE);
    gtk_widget_set_vexpand(big_container, FALSE);
    gtk_box_append(GTK_BOX(main_container), big_container);

    // Overview Section (vertical box, on the left)
    GtkWidget *overview_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(overview_section, GTK_ALIGN_START);
    gtk_widget_set_valign(overview_section, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(big_container), overview_section);

    // Create the overview frame
    view->overview_frame = gtk_frame_new("Overview");
    gtk_widget_set_hexpand(view->overview_frame, FALSE);
    gtk_box_append(GTK_BOX(overview_section), view->overview_frame);

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

    // Process List Section (vertical box, on the right)
    GtkWidget *process_list_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(process_list_section, GTK_ALIGN_END);
    gtk_widget_set_hexpand(process_list_section, TRUE);
    gtk_box_append(GTK_BOX(big_container), process_list_section);

    // Create the title label for Process List
    GtkWidget *process_list_title = gtk_label_new("Process List");
    gtk_label_set_xalign(GTK_LABEL(process_list_title), 0.0);
    gtk_widget_set_margin_bottom(process_list_title, 5);
    gtk_box_append(GTK_BOX(process_list_section), process_list_title);

    // Create a horizontal box to hold process entries
    GtkWidget *process_list_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_hexpand(process_list_box, TRUE);

    // Create a scrolled window to contain the horizontal list
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), process_list_box);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, -1, 200);
    gtk_box_append(GTK_BOX(process_list_section), scrolled_window);

    // Store the process list box in the ProcessListWidgets structure
    view->process_list_widgets = g_new(ProcessListWidgets, 1);
    view->process_list_widgets->process_list_box = process_list_box;

    // Apply CSS for styling
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "frame {"
        "   background-color: #33A19A;"
        "   padding: 10px;"
        "   border-radius: 5px;"
        "}"
        "label {"
        "   color: rgb(18, 76, 71);"
        "}"
        "frame > label, box > label {"
        "   font-weight: bold;"
        "   font-size: 16px;"
        "   color: rgb(35, 124, 116);"
        "}"
        "box.big-container {"
        "   background-color: #D3D3D3;"
        "   padding: 10px;"
        "   border-radius: 5px;"
        "}"
        "box.process-entry {"
        "   background-color: #33A19A;"
        "   padding: 5px;"
        "   border-radius: 5px;"
        "}"
    );

    // Add a CSS class to big_container and process_list_section
    gtk_widget_add_css_class(big_container, "big-container");
    gtk_widget_add_css_class(process_list_box, "process-list");

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

void dashboard_view_add_process(DashboardView *view, int pid, const char *state, int priority, 
                               int mem_lower, int mem_upper, int program_counter) {
    // Create a horizontal box for this process entry
    GtkWidget *process_entry = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_start(process_entry, 5);
    gtk_widget_set_margin_end(process_entry, 5);
    gtk_widget_add_css_class(process_entry, "process-entry");

    // Create labels for each piece of process information
    char pid_str[32], priority_str[32], mem_lower_str[32], mem_upper_str[32], pc_str[32];
    snprintf(pid_str, sizeof(pid_str), "PID: %d", pid);
    snprintf(priority_str, sizeof(priority_str), "Priority: %d", priority);
    snprintf(mem_lower_str, sizeof(mem_lower_str), "Mem Lower: %d", mem_lower);
    snprintf(mem_upper_str, sizeof(mem_upper_str), "Mem Upper: %d", mem_upper);
    snprintf(pc_str, sizeof(pc_str), "PC: %d", program_counter);

    GtkWidget *pid_label = gtk_label_new(pid_str);
    gtk_widget_set_halign(pid_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(process_entry), pid_label);

    GtkWidget *state_label = gtk_label_new(state);
    gtk_widget_set_halign(state_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(process_entry), state_label);

    GtkWidget *priority_label = gtk_label_new(priority_str);
    gtk_widget_set_halign(priority_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(process_entry), priority_label);

    GtkWidget *mem_lower_label = gtk_label_new(mem_lower_str);
    gtk_widget_set_halign(mem_lower_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(process_entry), mem_lower_label);

    GtkWidget *mem_upper_label = gtk_label_new(mem_upper_str);
    gtk_widget_set_halign(mem_upper_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(process_entry), mem_upper_label);

    GtkWidget *pc_label = gtk_label_new(pc_str);
    gtk_widget_set_halign(pc_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(process_entry), pc_label);

    // Append the process entry to the process list box
    gtk_box_append(GTK_BOX(view->process_list_widgets->process_list_box), process_entry);
}

void dashboard_view_free(DashboardView *view) {
    if (view) {
        g_free(view->process_list_widgets);
        g_free(view);
    }
}