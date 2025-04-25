#include <gtk/gtk.h>
#include "view.h"

static View *view = NULL;

// Drawing callback for each queue
static void draw_queue_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
    int queue_index = GPOINTER_TO_INT(data);
    GList *processes = view->queue_processes[queue_index];

    // Background
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9); 
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    int x = 20;
    GList *iter;
    for (iter = processes; iter != NULL; iter = iter->next) {
        int pid = GPOINTER_TO_INT(iter->data);

        // green if running, purple ready
        if (pid == view->running_pid) {
            cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); 
        } else {
            cairo_set_source_rgb(cr, 0.5, 0.0, 0.5); 
        }

        // Draw rectangle
        cairo_rectangle(cr, x, height / 2 - 15, 40, 30);
        cairo_fill(cr);

        //PID inside rectangle
        char pid_str[16];
        snprintf(pid_str, sizeof(pid_str), "P%d", pid);
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White text
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 12);
        cairo_move_to(cr, x + 10, height / 2 + 5);
        cairo_show_text(cr, pid_str);

        //arrow to the next process
        if (iter->next != NULL) {
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black arrow
            cairo_move_to(cr, x + 40, height / 2);
            cairo_line_to(cr, x + 60, height / 2);
            cairo_stroke(cr);
            // Arrowhead
            cairo_move_to(cr, x + 60, height / 2);
            cairo_line_to(cr, x + 55, height / 2 - 5);
            cairo_move_to(cr, x + 60, height / 2);
            cairo_line_to(cr, x + 55, height / 2 + 5);
            cairo_stroke(cr);
        }
        x += 70; // Space between rectangles
    }
}

static void draw_legend_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {

    int x = 20;
    int y = height / 2;

    // Purple circle for "ready"
    cairo_set_source_rgb(cr, 0.5, 0.0, 0.5); 
    cairo_arc(cr, x, y, 10, 0, 2 * G_PI);
    cairo_fill(cr);

    // "ready" label
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "ready");

    x += 70;

    // Green circle for "running"
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_arc(cr, x, y, 10, 0, 2 * G_PI);
    cairo_fill(cr);

    // "running" label
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); 
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "running");
}

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
    gtk_grid_attach(GTK_GRID(grid), header, 0, 0, 1, 1);

    // Drawing areas for each queue 
    for (int i = 0; i < 4; i++) {
        view->drawing_areas[i] = gtk_drawing_area_new();
        gtk_widget_set_size_request(view->drawing_areas[i], 600, 50);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(view->drawing_areas[i]),
                                       draw_queue_callback,
                                       GINT_TO_POINTER(i),
                                       NULL);
        gtk_grid_attach(GTK_GRID(grid), view->drawing_areas[i], 0, i + 1, 1, 1);

        // Initialize process list for this queue
        view->queue_processes[i] = NULL;
    }

    // Legend drawing area (below the queues)
    GtkWidget *legend_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(legend_area, 600, 30);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(legend_area),
                                   draw_legend_callback,
                                   NULL,
                                   NULL);
    gtk_grid_attach(GTK_GRID(grid), legend_area, 0, 5, 1, 1);

    // Create running process display
    view->running_process_label = gtk_label_new("Running Process: None");
    gtk_label_set_xalign(GTK_LABEL(view->running_process_label), 0.0);
    gtk_grid_attach(GTK_GRID(grid), view->running_process_label, 0, 6, 1, 1);

    // Create step button
    view->step_button = gtk_button_new_with_label("Step");
    gtk_grid_attach(GTK_GRID(grid), view->step_button, 0, 7, 1, 1);

    // Initialize running PID
    view->running_pid = -1;

    return view->window;
}

void view_update_queue(int queue_index, GList *processes, int running_pid) {
    if (queue_index < 0 || queue_index >= 4) return;

    // Free existing process list
    g_list_free(view->queue_processes[queue_index]);
    view->queue_processes[queue_index] = g_list_copy(processes);

    // Update running PID
    view->running_pid = running_pid;

    // Redraw the queue
    gtk_widget_queue_draw(view->drawing_areas[queue_index]);
}

GtkWidget* view_get_running_process_label() {
    return view->running_process_label;
}

GtkWidget* view_get_step_button() {
    return view->step_button;
}