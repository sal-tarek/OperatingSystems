#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "console_view.h"

// Single Console will be created and will be used globally
GtkWidget *console;

// A thread function that will handle console I/O
static void* console_io_thread(void* data) {
    GtkWidget *console = data;
    char buffer[256];
    int num1, num2;
    
    // Welcome message
    console_view_printf(console, "Welcome to the Console View Test\n");
    console_view_printf(console, "This demonstrates scanf and printf functionality\n\n");
    
    // Simple calculator example
    while (1) {
        console_view_printf(console, "Enter two numbers (or 'quit' to exit): ");
        console_view_scanf(console, buffer, sizeof(buffer));
        
        // Check for quit command
        if (strcmp(buffer, "quit") == 0) {
            console_view_printf(console, "\nExiting calculator. Thanks for using Console View!\n");
            break;
        }
        
        // Try to parse two numbers
        if (sscanf(buffer, "%d %d", &num1, &num2) == 2) {
            console_view_printf(console, "\nResults:\n");
            console_view_printf(console, "%d + %d = %d\n", num1, num2, num1 + num2);
            console_view_printf(console, "%d - %d = %d\n", num1, num2, num1 - num2);
            console_view_printf(console, "%d * %d = %d\n", num1, num2, num1 * num2);
            
            if (num2 != 0) {
                console_view_printf(console, "%d / %d = %.2f\n\n", num1, num2, (float)num1 / num2);
            } else {
                console_view_printf(console, "Division by zero not allowed\n\n");
            }
        } else {
            console_view_printf(console, "Invalid input. Please enter two numbers separated by space.\n\n");
        }
    }
    
    return NULL;
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Create the main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Console View Test");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    
    // Create a container for our content with padding
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(content_box, 10);
    gtk_widget_set_margin_end(content_box, 10);
    gtk_widget_set_margin_top(content_box, 10);
    gtk_widget_set_margin_bottom(content_box, 10);
    gtk_widget_set_vexpand(content_box, TRUE);
    gtk_widget_set_hexpand(content_box, TRUE);

    // Create console view
    GtkWidget *entry;
    console = console_view_new(&entry);
    gtk_widget_set_vexpand(console, TRUE);
    gtk_widget_set_hexpand(console, TRUE);
    
    // Add console to the content box
    gtk_box_append(GTK_BOX(content_box), console);
    
    // Set the content box as the window's child
    gtk_window_set_child(GTK_WINDOW(window), content_box);

    // Create a thread to handle I/O
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, console_io_thread, console);
    
    // Show the window
    gtk_widget_set_visible(window, TRUE);
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    GtkApplication *app = gtk_application_new("org.example.consoletest", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    // Run the app
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    
    // Clean up
    g_object_unref(app);
    
    return status;
}