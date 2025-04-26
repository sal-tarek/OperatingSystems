#include <gtk/gtk.h>
#include "console_view.h"

static void on_prompt_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *console = GTK_WIDGET(user_data);
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    console_view_prompt_input(console, parent);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "OS Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), box);

    // Create and add console
    GtkWidget *console = console_view_new();
    gtk_box_append(GTK_BOX(box), console);
    gtk_widget_set_vexpand(console, TRUE);

    // Create a button to prompt input
    GtkWidget *prompt_button = gtk_button_new_with_label("Enter Command");
    gtk_box_append(GTK_BOX(box), prompt_button);

    g_signal_connect(prompt_button, "clicked", G_CALLBACK(on_prompt_button_clicked), console);

    gtk_window_present(GTK_WINDOW(window));

    // Example of appending output manually
    console_view_append_text(console, "System initialized...");
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.example.consoleapp", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
