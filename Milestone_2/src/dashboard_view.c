#include "dashboard_view.h"
#include "dashboard_controller.h"

// Function to create the Overview Section
GtkWidget* create_overview_section(OverviewWidgets **overview_widgets) {
    // Create the main vertical box for the Overview Section
    GtkWidget *overview_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(overview_box, 10);
    gtk_widget_set_margin_end(overview_box, 10);
    gtk_widget_set_margin_top(overview_box, 10);
    gtk_widget_set_margin_bottom(overview_box, 10);

    // Create the title label
    GtkWidget *title_label = gtk_label_new("Overview");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.0); // Left align
    gtk_widget_set_margin_bottom(title_label, 5);

    // Style the title label (green text: #196761)
    GtkStyleContext *title_context = gtk_widget_get_style_context(title_label);
    gtk_style_context_add_class(title_context, "overview-title");

    // Create a grid to hold the overview information
    GtkWidget *overview_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(overview_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(overview_grid), 10);

    // Labels for the data (placeholders for now, to be updated by controller)
    GtkWidget *processes_label = gtk_label_new("no./ of processes:");
    GtkWidget *processes_value = gtk_label_new("0");
    GtkWidget *clock_label = gtk_label_new("current clock cycle:");
    GtkWidget *clock_value = gtk_label_new("0");
    GtkWidget *algorithm_label = gtk_label_new("active scheduling algorithm:");
    GtkWidget *algorithm_value = gtk_label_new("None");

    // Store the value labels in the OverviewWidgets structure
    *overview_widgets = g_new(OverviewWidgets, 1);
    (*overview_widgets)->processes_value_label = processes_value;
    (*overview_widgets)->clock_value_label = clock_value;
    (*overview_widgets)->algorithm_value_label = algorithm_value;

    // Style the labels (green text: #196761)
    const char *label_class = "overview-label";
    gtk_style_context_add_class(gtk_widget_get_style_context(processes_label), label_class);
    gtk_style_context_add_class(gtk_widget_get_style_context(processes_value), label_class);
    gtk_style_context_add_class(gtk_widget_get_style_context(clock_label), label_class);
    gtk_style_context_add_class(gtk_widget_get_style_context(clock_value), label_class);
    gtk_style_context_add_class(gtk_widget_get_style_context(algorithm_label), label_class);
    gtk_style_context_add_class(gtk_widget_get_style_context(algorithm_value), label_class);

    // Add labels to the grid
    gtk_grid_attach(GTK_GRID(overview_grid), processes_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(overview_grid), processes_value, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(overview_grid), clock_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(overview_grid), clock_value, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(overview_grid), algorithm_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(overview_grid), algorithm_value, 1, 2, 1, 1);

    // Style the grid background (teal: #33A19A)
    GtkStyleContext *grid_context = gtk_widget_get_style_context(overview_grid);
    gtk_style_context_add_class(grid_context, "overview-grid");

    // Pack everything into the main box
    // In GTK4, use gtk_box_append instead of gtk_box_pack_start
    gtk_box_append(GTK_BOX(overview_box), title_label);
    gtk_box_append(GTK_BOX(overview_box), overview_grid);

    // Apply CSS for styling
    GtkCssProvider *provider = gtk_css_provider_new();
    // In GTK4, load_from_data has a different signature
    gtk_css_provider_load_from_data(provider,
        ".overview-title {"
        "   font-weight: bold;"
        "   font-size: 16px;"
        "   color: #196761;"
        "}"
        ".overview-label {"
        "   color: #196761;"
        "}"
        ".overview-grid {"
        "   background-color: #33A19A;"
        "   padding: 10px;"
        "   border-radius: 5px;"
        "}", -1);

    // In GTK4, we use gtk_style_context_add_provider_for_display
    GdkDisplay *display = gtk_widget_get_display(overview_box);
    gtk_style_context_add_provider_for_display(display,
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    return overview_box;
}