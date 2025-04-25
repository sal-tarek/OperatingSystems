#include <gtk/gtk.h>
#include "view.h"

static View *view = NULL;

// Structure to hold animation state for each process
typedef struct {
    int pid;              // Process ID
    float alpha;          // Opacity for fade-in (0.0 to 1.0)
    float start_x;        // Starting x position for movement
    float end_x;          // Ending x position for movement
    float start_y;        // Starting y position for movement
    float end_y;          // Ending y position for movement
    int animating;        // 1 if animating, 0 if done
    int steps;            // Animation step counter
    int total_steps;      // Total steps for animation
} ProcessAnimation;

// Structure to hold animation data for each queue
typedef struct {
    GList *animations;    // List of ProcessAnimation for this queue
} QueueAnimation;

static QueueAnimation queue_animations[4];

// Drawing callback for each queue
static void draw_queue_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
    int queue_index = GPOINTER_TO_INT(data);
    GList *processes = view->queue_processes[queue_index];
    GList *anim_iter = queue_animations[queue_index].animations;

    // Background
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    // Calculate the starting x position 
    int num_processes = g_list_length(processes);
    int base_x = 20 + (num_processes - 1) * 70; 

    GList *proc_iter;
    for (proc_iter = g_list_last(processes); proc_iter != NULL; proc_iter = proc_iter->prev) {
        int pid = GPOINTER_TO_INT(proc_iter->data);
        ProcessAnimation *anim = NULL;

        // Find the animation for this process
        for (GList *a = anim_iter; a != NULL; a = a->next) {
            ProcessAnimation *pa = (ProcessAnimation *)a->data;
            if (pa->pid == pid) {
                anim = pa;
                break;
            }
        }

        float x = base_x;
        float y = height / 2;
        float alpha = 1.0;

        if (anim && anim->animating) {
            // Interpolate position and alpha
            float t = (float)anim->steps / anim->total_steps;
            x = anim->start_x + t * (anim->end_x - anim->start_x);
            y = anim->start_y + t * (anim->end_y - anim->start_y);
            alpha = anim->alpha;
        }

        // Set color: green if running, purple if ready
        if (pid == view->running_pid) {
            cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, alpha);
        } else {
            cairo_set_source_rgba(cr, 0.5, 0.0, 0.5, alpha);
        }

        // Draw rectangle
        cairo_rectangle(cr, x - 20, y - 15, 40, 30); // Adjust x by base offset
        cairo_fill(cr);

        // Draw PID inside rectangle
        char pid_str[16];
        snprintf(pid_str, sizeof(pid_str), "P%d", pid);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 12);
        cairo_move_to(cr, x - 10, y + 5);
        cairo_show_text(cr, pid_str);

        // Draw arrow to the next process 
        if (proc_iter->prev != NULL) {
            float next_x = base_x - 70; 
            if (anim && anim->animating) {
                float t = (float)anim->steps / anim->total_steps;
                next_x = (anim->start_x + t * (anim->end_x - anim->start_x)) - 50;
            }
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_move_to(cr, x - 20, y);
            cairo_line_to(cr, x - 40, y);
            cairo_stroke(cr);
            cairo_move_to(cr, x - 40, y);
            cairo_line_to(cr, x - 35, y - 5);
            cairo_move_to(cr, x - 40, y);
            cairo_line_to(cr, x - 35, y + 5);
            cairo_stroke(cr);
        }

        base_x -= 70; 
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

// Animation callback
static gboolean animate_process(gpointer user_data) {
    int queue_index = GPOINTER_TO_INT(user_data);

    GList *anim_iter = queue_animations[queue_index].animations;
    gboolean any_animating = FALSE;

    for (GList *a = anim_iter; a != NULL; a = a->next) {
        ProcessAnimation *anim = (ProcessAnimation *)a->data;
        if (anim->animating) {
            anim->steps++;
            if (anim->steps >= anim->total_steps) {
                anim->animating = 0;
                anim->alpha = 1.0;
            } else {
                // Update alpha for fade-in
                anim->alpha = (float)anim->steps / anim->total_steps;
                any_animating = TRUE;
            }
        }
    }

    gtk_widget_queue_draw(view->drawing_areas[queue_index]);

    return any_animating ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
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

        // Initialize process list and animations for this queue
        view->queue_processes[i] = NULL;
        queue_animations[i].animations = NULL;
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

    g_list_free_full(queue_animations[queue_index].animations, g_free);
    queue_animations[queue_index].animations = NULL;

    // Compute positions for processes
    GList *old_processes = view->queue_processes[queue_index];
    GList *new_processes = processes;

    const int drawing_area_height = 50;
    const int row_spacing = 10;
    const int header_height = 30; 

    // Calculate the position for each process
    int num_processes = g_list_length(new_processes);
    int base_x = 20 + (num_processes - 1) * 70; 

    
    GList *proc_iter;
    for (proc_iter = g_list_last(new_processes); proc_iter != NULL; proc_iter = proc_iter->prev) {
        int pid = GPOINTER_TO_INT(proc_iter->data);
        ProcessAnimation *anim = g_new0(ProcessAnimation, 1);
        anim->pid = pid;
        anim->alpha = 0.0;
        anim->animating = 1;
        anim->steps = 0;
        anim->total_steps = 14; // 14 steps for fade-in (~0.2 seconds per step)

        // Check if this process existed in the old list
        int old_pos = -1;
        int old_queue = -1;
        for (int q = 0; q < 4; q++) {
            int pos = 0;
            for (GList *p = view->queue_processes[q]; p != NULL; p = p->next) {
                if (GPOINTER_TO_INT(p->data) == pid) {
                    old_pos = pos;
                    old_queue = q;
                    break;
                }
                pos++;
            }
            if (old_pos != -1) break;
        }

        // Calculate the old and new positions 
        int new_pos = g_list_position(g_list_last(new_processes), proc_iter);
        int final_x = 20 + new_pos * 70; 

        int old_x = 20 + (g_list_length(old_processes) - 1 - old_pos) * 70;
        if (old_pos == -1) old_x = final_x; 

        if (old_pos == -1) {
            // New process: fade in 
            anim->start_x = final_x;
            anim->end_x = final_x;
            anim->start_y = drawing_area_height / 2;
            anim->end_y = drawing_area_height / 2;
        } else if (old_queue != queue_index) {
            // Moving between queues: slide vertically
            anim->start_x = old_x;
            anim->end_x = final_x;
            // Adjust y position based on queue indices, accounting for grid layout
            int start_row = old_queue + 1; 
            int end_row = queue_index + 1;
            anim->start_y = header_height + start_row * (drawing_area_height + row_spacing) + drawing_area_height / 2;
            anim->end_y = header_height + end_row * (drawing_area_height + row_spacing) + drawing_area_height / 2;
            anim->start_y -= (header_height + end_row * (drawing_area_height + row_spacing));
            anim->end_y -= (header_height + end_row * (drawing_area_height + row_spacing));
        } else {
            // Moving within the same queue: slide horizontally
            anim->start_x = old_x;
            anim->end_x = final_x;
            anim->start_y = drawing_area_height / 2;
            anim->end_y = drawing_area_height / 2;
        }

        queue_animations[queue_index].animations = g_list_append(queue_animations[queue_index].animations, anim);
        base_x -= 70; 
    }

    // Update process list
    g_list_free(view->queue_processes[queue_index]);
    view->queue_processes[queue_index] = g_list_copy(processes);

    // Update running PID
    view->running_pid = running_pid;

    // Start animation
    g_timeout_add(20, animate_process, GINT_TO_POINTER(queue_index));
}

GtkWidget* view_get_running_process_label() {
    return view->running_process_label;
}

GtkWidget* view_get_step_button() {
    return view->step_button;
}