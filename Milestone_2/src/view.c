#include <gtk/gtk.h>
#include "view.h"

static View *view = NULL;
static GtkCssProvider *css_provider = NULL;

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

static QueueAnimation queue_animations[5]; // Updated for 5 queues

// CSS for styling the application
static const char *css_data = 
"window {"
"    background-color: #f5f5f7;"
"}"
"button {"
"    background: linear-gradient(to bottom, #4a86e8, #3a76d8);"
"    color: white;"
"    border-radius: 6px;"
"    border: none;"
"    padding: 8px 16px;"
"    font-weight: bold;"
"    box-shadow: 0 1px 3px rgba(0,0,0,0.2);"
"    transition: all 200ms ease;"
"}"
"button:hover {"
"    background: linear-gradient(to bottom, #5a96f8, #4a86e8);"
"    box-shadow: 0 2px 5px rgba(0,0,0,0.3);"
"}"
"button:active {"
"    background: linear-gradient(to bottom, #3a76d8, #2a66c8);"
"    box-shadow: inset 0 1px 2px rgba(0,0,0,0.3);"
"}"
"label.header {"
"    font-size: 20px;"
"    font-weight: bold;"
"    color: #333333;"
"    margin: 10px 0;"
"}"
"label.running-process {"
"    font-size: 14px;"
"    font-weight: bold;"
"    margin-top: 5px;"
"    color: #555555;"
"}"
"box {"
"    background-color: #f5f5f7;"
"    border-radius: 8px;"
"    padding: 5px;"
"}"
".queue-area {"
"    background-color: #ffffff;"
"    border-radius: 10px;"
"    box-shadow: 0 2px 10px rgba(0,0,0,0.1);"
"    margin: 5px;"
"}";

// Drawing callback for each queue
static void draw_queue_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
    int queue_index = GPOINTER_TO_INT(data);
    GList *processes = view->queue_processes[queue_index];
    GList *anim_iter = queue_animations[queue_index].animations;

    // Background with rounded corners
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);  // White background
    double radius = 10.0;
    double degrees = M_PI / 180.0;
    
    // Draw rounded rectangle for background
    cairo_new_sub_path(cr);
    cairo_arc(cr, width - radius, radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, width - radius, height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, radius, height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, radius, radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);
    
    cairo_fill(cr);
    
    // Add a subtle inner shadow
    cairo_set_source_rgba(cr, 0.9, 0.9, 0.9, 0.5);
    cairo_set_line_width(cr, 1);
    cairo_new_sub_path(cr);
    cairo_arc(cr, width - radius, radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, width - radius, height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, radius, height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, radius, radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);
    cairo_stroke(cr);
    
    // Queue Label with gradient
    cairo_pattern_t *gradient = cairo_pattern_create_linear(0, 0, width, 0);
    if (queue_index == 4) { // Blocked queue
        // Red gradient for blocked queue
        cairo_pattern_add_color_stop_rgba(gradient, 0, 0.8, 0.2, 0.2, 1.0);
        cairo_pattern_add_color_stop_rgba(gradient, 1, 0.6, 0.1, 0.1, 1.0);
    } else {
        // Blue gradient for ready queues
        cairo_pattern_add_color_stop_rgba(gradient, 0, 0.2, 0.4, 0.8, 1.0);
        cairo_pattern_add_color_stop_rgba(gradient, 1, 0.1, 0.3, 0.6, 1.0);
    }
    
    cairo_set_source(cr, gradient);
    cairo_rectangle(cr, 0, 0, width, 25);
    cairo_fill(cr);
    cairo_pattern_destroy(gradient);
    
    // Queue label text
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);  // White text
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    
    if (queue_index == 4) { // Blocked queue
        cairo_move_to(cr, 10, 18);
        cairo_show_text(cr, "Blocked Queue");
    } else {
        cairo_move_to(cr, 10, 18);
        char label[20];
        snprintf(label, sizeof(label), "Ready Queue %d", queue_index);
        cairo_show_text(cr, label);
    }

    // Draw processes in original order (from head to tail)
    GList *proc_iter;
    int process_count = 0;
    
    // Iterate through processes in original order (from head to tail)
    for (proc_iter = processes; proc_iter != NULL; proc_iter = proc_iter->next) {
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

        float x, y;
        float alpha = 1.0;

        // Default positions if no animation
        if (queue_index == 4) { // Blocked queue (vertical)
            x = width / 2;
            y = 70 + process_count * 60;  // Increased spacing for better visibility
        } else { // Ready queues (horizontal)
            x = 120 + process_count * 80;  // Increased spacing
            y = height / 2 + 5;  // Adjusted for gradient header
        }

        if (anim && anim->animating) {
            // Interpolate position and alpha
            float t = (float)anim->steps / anim->total_steps;
            x = anim->start_x + t * (anim->end_x - anim->start_x);
            y = anim->start_y + t * (anim->end_y - anim->start_y);
            alpha = anim->alpha;
        }

        // Create gradient patterns for process boxes
        cairo_pattern_t *box_gradient = cairo_pattern_create_linear(x - 25, y - 20, x + 25, y + 20);
        
        if (pid == view->running_pid) {
            // Green gradient for running
            cairo_pattern_add_color_stop_rgba(box_gradient, 0, 0.0, 0.8, 0.0, alpha);
            cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.0, 0.6, 0.0, alpha);
        } else if (queue_index == 4) {
            // Red gradient for blocked
            cairo_pattern_add_color_stop_rgba(box_gradient, 0, 0.9, 0.2, 0.2, alpha);
            cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.7, 0.1, 0.1, alpha);
        } else {
            // Purple gradient for ready
            cairo_pattern_add_color_stop_rgba(box_gradient, 0, 0.6, 0.3, 0.7, alpha);
            cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.4, 0.1, 0.5, alpha);
        }
        
        cairo_set_source(cr, box_gradient);
        
        // Draw process box with rounded corners
        double box_radius = 8.0;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + 20 - box_radius, y - 15 + box_radius, box_radius, -90 * degrees, 0 * degrees);
        cairo_arc(cr, x + 20 - box_radius, y + 15 - box_radius, box_radius, 0 * degrees, 90 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y + 15 - box_radius, box_radius, 90 * degrees, 180 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y - 15 + box_radius, box_radius, 180 * degrees, 270 * degrees);
        cairo_close_path(cr);
        
        cairo_fill(cr);
        
        // Add shadow effect
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.2);
        cairo_set_line_width(cr, 1);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + 20 - box_radius, y - 15 + box_radius, box_radius, -90 * degrees, 0 * degrees);
        cairo_arc(cr, x + 20 - box_radius, y + 15 - box_radius, box_radius, 0 * degrees, 90 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y + 15 - box_radius, box_radius, 90 * degrees, 180 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y - 15 + box_radius, box_radius, 180 * degrees, 270 * degrees);
        cairo_close_path(cr);
        cairo_stroke(cr);
        
        cairo_pattern_destroy(box_gradient);

        // Draw PID inside rectangle with better font
        char pid_str[16];
        snprintf(pid_str, sizeof(pid_str), "P%d", pid);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14);  // Larger font
        
        // Measure text for centering
        cairo_text_extents_t extents;
        cairo_text_extents(cr, pid_str, &extents);
        cairo_move_to(cr, x - extents.width/2, y + extents.height/2);
        cairo_show_text(cr, pid_str);

        if (proc_iter->next != NULL) {  // Not the last process in original list order
            if (queue_index == 4) { // Vertical arrows for blocked queue
                float next_y = 50 + (process_count + 1) * 50;
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_move_to(cr, x, y + 15);
                cairo_line_to(cr, x, y + 30);
                cairo_stroke(cr);
                cairo_move_to(cr, x, y + 30);
                cairo_line_to(cr, x - 5, y + 25);
                cairo_move_to(cr, x, y + 30);
                cairo_line_to(cr, x + 5, y + 25);
                cairo_stroke(cr);
            } else { // Horizontal arrows for ready queues - leftward arrows
                x+=70;
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_move_to(cr, x - 20, y);  // Start from left side of current process
                cairo_line_to(cr, x - 40, y);  // Draw line to the left
                cairo_stroke(cr);
                cairo_move_to(cr, x - 40, y);  // Position at end of line
                cairo_line_to(cr, x - 35, y - 5);  // Draw arrow head top
                cairo_move_to(cr, x - 40, y);  // Position at end of line again
                cairo_line_to(cr, x - 35, y + 5);  // Draw arrow head bottom
                cairo_stroke(cr);
            }
        }
        
        process_count++;
    }
}

static void draw_legend_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
    int x = 30;
    int y = height / 2;
    double circle_radius = 12;
    double degrees = M_PI / 180.0;

    // White rounded rectangle for background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    double bg_radius = 10.0;
    cairo_new_sub_path(cr);
    cairo_arc(cr, width - bg_radius, bg_radius, bg_radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, width - bg_radius, height - bg_radius, bg_radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, bg_radius, height - bg_radius, bg_radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, bg_radius, bg_radius, bg_radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);
    cairo_fill(cr);

    // Add subtle shadow
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.1);
    cairo_set_line_width(cr, 1);
    cairo_new_sub_path(cr);
    cairo_arc(cr, width - bg_radius, bg_radius, bg_radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, width - bg_radius, height - bg_radius, bg_radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, bg_radius, height - bg_radius, bg_radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, bg_radius, bg_radius, bg_radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);
    cairo_stroke(cr);

    // Purple circle with gradient for "ready"
    cairo_pattern_t *ready_gradient = cairo_pattern_create_radial(x, y, 0, x, y, circle_radius);
    cairo_pattern_add_color_stop_rgba(ready_gradient, 0, 0.6, 0.3, 0.7, 1.0);
    cairo_pattern_add_color_stop_rgba(ready_gradient, 1, 0.4, 0.1, 0.5, 1.0);
    cairo_set_source(cr, ready_gradient);
    cairo_arc(cr, x, y, circle_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(ready_gradient);

    // Add highlight to circle
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);
    cairo_arc(cr, x - 3, y - 3, circle_radius/2, 0, 2 * M_PI);
    cairo_fill(cr);

    // "ready" label with better font
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "Ready");

    x += 100;

    // Green circle with gradient for "running"
    cairo_pattern_t *running_gradient = cairo_pattern_create_radial(x, y, 0, x, y, circle_radius);
    cairo_pattern_add_color_stop_rgba(running_gradient, 0, 0.1, 0.9, 0.1, 1.0);
    cairo_pattern_add_color_stop_rgba(running_gradient, 1, 0.0, 0.7, 0.0, 1.0);
    cairo_set_source(cr, running_gradient);
    cairo_arc(cr, x, y, circle_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(running_gradient);

    // Add highlight to circle
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);
    cairo_arc(cr, x - 3, y - 3, circle_radius/2, 0, 2 * M_PI);
    cairo_fill(cr);

    // "running" label
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "Running");

    x += 100;

    // Red circle with gradient for "blocked"
    cairo_pattern_t *blocked_gradient = cairo_pattern_create_radial(x, y, 0, x, y, circle_radius);
    cairo_pattern_add_color_stop_rgba(blocked_gradient, 0, 1.0, 0.3, 0.3, 1.0);
    cairo_pattern_add_color_stop_rgba(blocked_gradient, 1, 0.8, 0.1, 0.1, 1.0);
    cairo_set_source(cr, blocked_gradient);
    cairo_arc(cr, x, y, circle_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(blocked_gradient);

    // Add highlight to circle
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);
    cairo_arc(cr, x - 3, y - 3, circle_radius/2, 0, 2 * M_PI);
    cairo_fill(cr);

    // "blocked" label
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "Blocked");
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
                // Use easing function for smoother animation
                float progress = (float)anim->steps / anim->total_steps;
                // Cubic easing: smooth acceleration and deceleration
                anim->alpha = progress < 0.5 ? 4 * progress * progress * progress : 1 - pow(-2 * progress + 2, 3) / 2;
                any_animating = TRUE;
            }
        }
    }

    gtk_widget_queue_draw(view->drawing_areas[queue_index]);

    return any_animating ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

GtkWidget* view_init() {
    view = g_new0(View, 1);

    // Create CSS provider
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider, css_data, -1);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // Create main window
    view->window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(view->window), "OS Scheduler Simulation");
    gtk_window_set_default_size(GTK_WINDOW(view->window), 950, 650);
    

    // Create main box with padding
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_start(main_box, 15);
    gtk_widget_set_margin_end(main_box, 15);
    gtk_widget_set_margin_top(main_box, 15);
    gtk_widget_set_margin_bottom(main_box, 15);
    gtk_window_set_child(GTK_WINDOW(view->window), main_box);

    // Create main grid
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_box_append(GTK_BOX(main_box), grid);

    // Create blocked queue area (vertical, on the left)
    view->drawing_areas[4] = gtk_drawing_area_new();
    gtk_widget_set_size_request(view->drawing_areas[4], 130, 320);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(view->drawing_areas[4]),
                                   draw_queue_callback,
                                   GINT_TO_POINTER(4),
                                   NULL);
    gtk_widget_add_css_class(view->drawing_areas[4], "queue-area");
    gtk_grid_attach(GTK_GRID(grid), view->drawing_areas[4], 0, 0, 1, 4); // Takes 1 column, spans 4 rows

    // Create horizontal ready queues (to the right of blocked queue)
    for (int i = 0; i < 4; i++) {
        view->drawing_areas[i] = gtk_drawing_area_new();
        gtk_widget_set_size_request(view->drawing_areas[i], 720, 90);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(view->drawing_areas[i]),
                                        draw_queue_callback,
                                        GINT_TO_POINTER(i),
                                        NULL);
        gtk_widget_add_css_class(view->drawing_areas[i], "queue-area");
        gtk_grid_attach(GTK_GRID(grid), view->drawing_areas[i], 1, i, 1, 1);
    }

    // Legend drawing area (below the queues)
    GtkWidget *legend_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(legend_area, 720, 40);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(legend_area),
                                   draw_legend_callback,
                                   NULL,
                                   NULL);
    gtk_widget_add_css_class(legend_area, "queue-area");
    gtk_grid_attach(GTK_GRID(grid), legend_area, 0, 4, 2, 1); // Span 2 columns

    // Create running process display
    view->running_process_label = gtk_label_new("Running Process: None");
    gtk_widget_add_css_class(view->running_process_label, "running-process");
    gtk_label_set_xalign(GTK_LABEL(view->running_process_label), 0.0);
    gtk_widget_set_margin_top(view->running_process_label, 10);
    gtk_box_append(GTK_BOX(main_box), view->running_process_label);

    // Create button box for control buttons with nice styling
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_margin_top(button_box, 15);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    
    // Create buttons
    view->step_button = gtk_button_new_with_label("Step");
    view->automatic_button = gtk_button_new_with_label("Automatic");
    view->pause_button = gtk_button_new_with_label("Pause");
    gtk_widget_set_sensitive(view->pause_button, FALSE); // Disabled by default
    
    // Add buttons to button box
    gtk_box_append(GTK_BOX(button_box), view->step_button);
    gtk_box_append(GTK_BOX(button_box), view->automatic_button);
    gtk_box_append(GTK_BOX(button_box), view->pause_button);
    
    // Add button box to main box
    gtk_box_append(GTK_BOX(main_box), button_box);

    // Initialize process lists and animations for all queues
    for (int i = 0; i < 5; i++) {
        view->queue_processes[i] = NULL;
        queue_animations[i].animations = NULL;
    }

    // Initialize running PID
    view->running_pid = -1;

    return view->window;
}

// Get position of a process in a specific queue
static int get_position_in_queue(GList *queue, int pid) {
    int pos = 0;
    for (GList *p = queue; p != NULL; p = p->next) {
        if (GPOINTER_TO_INT(p->data) == pid) {
            return pos;
        }
        pos++;
    }
    return -1;
}

// Get absolute coordinates for a process in a queue
static void get_process_coords(int queue_index, int position, float *x, float *y) {
    if (queue_index == 4) { // Blocked queue (vertical)
        *x = 65;  // Center of vertical queue
        *y = 70 + position * 60; // Vertical position with increased spacing
    } else { // Ready queues (horizontal)
        *x = 120 + position * 80; // Horizontal position with increased spacing
        *y = 40 + queue_index * 75; // Vertical position based on queue index with increased spacing
    }
}
void view_update_queue(int queue_index, GList *processes, int running_pid) {
    if (queue_index < 0 || queue_index >= 5) return;

    g_list_free_full(queue_animations[queue_index].animations, g_free);
    queue_animations[queue_index].animations = NULL;

    // Get old and new processes
    GList *old_processes = view->queue_processes[queue_index];
    GList *new_processes = processes;

    // Create animations for each process
    int new_position = 0;
    for (GList *proc_iter = processes; proc_iter != NULL; proc_iter = proc_iter->next) {
        int pid = GPOINTER_TO_INT(proc_iter->data);
        ProcessAnimation *anim = g_new0(ProcessAnimation, 1);
        
        anim->pid = pid;
        anim->animating = 1;
        anim->steps = 0;
        anim->total_steps = 20; // 20 steps for smoother animation
        
        // Find this process in any old queue
        int old_queue = -1;
        int old_position = -1;
        
        for (int q = 0; q < 5; q++) {
            int pos = get_position_in_queue(view->queue_processes[q], pid);
            if (pos != -1) {
                old_queue = q;
                old_position = pos;
                break;
            }
        }
        
        // Set end coordinates for this process in its new position
        get_process_coords(queue_index, new_position, &anim->end_x, &anim->end_y);
        
        if (old_queue == -1) {
            // New process - start at same position but fade in
            anim->start_x = anim->end_x;
            anim->start_y = anim->end_y;
            anim->alpha = 0.0;
        } else {
            // Get coordinates in old position
            get_process_coords(old_queue, old_position, &anim->start_x, &anim->start_y);
            anim->alpha = 1.0;
        }
        
        queue_animations[queue_index].animations = g_list_append(queue_animations[queue_index].animations, anim);
        new_position++;
    }

    // Update process list
    g_list_free(view->queue_processes[queue_index]);
    view->queue_processes[queue_index] = g_list_copy(processes);

    // Update running PID
    view->running_pid = running_pid;

    // Start animation timer
    g_timeout_add(16, animate_process, GINT_TO_POINTER(queue_index));
}

GtkWidget* view_get_running_process_label() {
    return view->running_process_label;
}

GtkWidget* view_get_step_button() {
    return view->step_button;
}

GtkWidget* view_get_automatic_button() {
    return view->automatic_button;
}

GtkWidget* view_get_pause_button() {
    return view->pause_button;
}