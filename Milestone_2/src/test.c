#include <gtk/gtk.h>
#include "dashboard_view.h"
#include "dashboard_controller.h"

// Callback to update the dashboard periodically
gboolean update_dashboard(gpointer user_data) {
    DashboardController *controller = (DashboardController*)user_data;
    dashboard_controller_update_overview(controller);
    return G_SOURCE_CONTINUE; // Keep the timer running
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Create the main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "System Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create the Overview Section and get the widgets
    OverviewWidgets *overview_widgets;
    GtkWidget *overview_section = create_overview_section(&overview_widgets);

    // Create the controller
    DashboardController *controller = dashboard_controller_new(overview_widgets);

    // Update the Overview Section with initial data
    dashboard_controller_update_overview(controller);

    // Add the Overview Section to the window
    // In GTK4, we use gtk_window_set_child instead of gtk_container_add
    gtk_window_set_child(GTK_WINDOW(window), overview_section);

    // Set up a timer to update the dashboard every second
    g_timeout_add(1000, update_dashboard, controller);

    // Show the window
    gtk_widget_show(window);
    
    // Store the controller and widgets in the window data for cleanup
    g_object_set_data_full(G_OBJECT(window), "controller", controller, 
                          (GDestroyNotify)dashboard_controller_free);
    g_object_set_data_full(G_OBJECT(window), "overview_widgets", overview_widgets,
                          (GDestroyNotify)g_free);
}

int main(int argc, char *argv[]) {
    // Initialize GTK application
    GtkApplication *app = gtk_application_new("org.example.dashboard", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}