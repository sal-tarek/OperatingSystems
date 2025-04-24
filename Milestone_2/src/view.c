#include <gtk/gtk.h>
#include "view.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *queue_labels[4];
    GtkWidget *running_process_label;
    GtkWidget *step_button;
} View;

static View *view = NULL;

GtkWidget* view_init() {
    view = g_new0(View, 1);
    
    // Create main window
    view->window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(view->window), "OS Scheduler Simulation");
    gtk_window_set_default_size(GTK_WINDOW(view->window), 800, 600);
    
    // Create main grid
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_window_set_child(GTK_WINDOW(view->window), grid);
    
    // Create header label
    GtkWidget *header = gtk_label_new("Operating System Scheduler Dashboard");
    gtk_grid_attach(GTK_GRID(grid), header, 0, 0, 2, 1);
    
    // Create queue displays
    for (int i = 0; i < 4; i++) {
        char label_text[32];
        snprintf(label_text, sizeof(label_text), "Queue %d: Empty", i + 1);
        view->queue_labels[i] = gtk_label_new(label_text);
        gtk_label_set_xalign(GTK_LABEL(view->queue_labels[i]), 0.0);
        gtk_grid_attach(GTK_GRID(grid), view->queue_labels[i], 0, i + 1, 2, 1);
    }
    
    // Create running process display
    view->running_process_label = gtk_label_new("Running Process: None");
    gtk_label_set_xalign(GTK_LABEL(view->running_process_label), 0.0);
    gtk_grid_attach(GTK_GRID(grid), view->running_process_label, 0, 5, 2, 1);
    
    // Create step button
    view->step_button = gtk_button_new_with_label("Step");
    gtk_grid_attach(GTK_GRID(grid), view->step_button, 0, 6, 1, 1);
    
    return view->window;
}

GtkWidget* view_get_queue_label(int queue_index) {
    if (queue_index >= 0 && queue_index < 4) {
        return view->queue_labels[queue_index];
    }
    return NULL;
}

GtkWidget* view_get_running_process_label() {
    return view->running_process_label;
}

GtkWidget* view_get_step_button() {
    return view->step_button;
}