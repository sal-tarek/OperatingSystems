#include <gtk/gtk.h>
#include "dashboard_controller.h"

static void activate(GtkApplication *app, gpointer user_data) {
    DashboardController *controller = dashboard_controller_new(app);
    
    // Show the dashboard
    dashboard_controller_show(controller);
    
    // Store the controller in the application data for cleanup
    g_object_set_data_full(G_OBJECT(app), "controller", controller, 
                          (GDestroyNotify)dashboard_controller_free);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.dashboard", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}