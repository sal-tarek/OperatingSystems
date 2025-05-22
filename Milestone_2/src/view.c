#include <gtk/gtk.h>
#include "view.h"
#include "mutex.h"
#include "process.h"
#include "PCB.h"

View *view = NULL; // Global, non-static
static GtkCssProvider *css_provider = NULL;

QueueAnimation queue_animations[5];

static const char *css_data =
    ".scheduler-view window { background-color: #2E2F32; }"
    ".scheduler-view frame { background-color: #D9D9D9; border: 1px solid #bbb; color: #333; }"
    ".scheduler-view frame > label { color: white; font-weight: bold; font-size: 14px; background-color: #33A19A; padding: 5px; border-radius: 3px 3px 0 0; }"
    ".scheduler-view label { color: #333; font-size: 14px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; }"
    ".scheduler-view listbox { background-color: #D9D9D9; }"
    ".scheduler-view listbox row { padding: 5px; margin: 2px; }"
    ".scheduler-view listbox row:nth-child(even) { background-color: rgba(51, 161, 154, 0.1); }"
    ".scheduler-view listbox row:hover { background-color: rgba(51, 161, 154, 0.3); }"
    ".scheduler-view button { background-color: #33A19A; color: #D9D9D9; border-radius: 5px; padding: 5px; }"
    ".scheduler-view button:hover { background-color: #278f89; }"
    ".scheduler-view entry { background-color: white; color: #333; border: 1px solid #bbb; border-radius: 5px; padding: 5px; }"
    ".scheduler-view drop-down { background-color: white; color: #333; border: 1px solid #bbb; border-radius: 5px; padding: 5px; }"
    ".scheduler-view textview { background-color: white; color: #000; border: 1px solid #bbb; padding: 8px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; font-size: 13px; }"
    ".scheduler-view .memory-tag { background-color: #33A19A; color: white; border-radius: 3px 0 0 3px; padding: 5px; font-weight: bold; margin-left: -10px; box-shadow: 1px 1px 3px rgba(0,0,0,0.3); }"
    ".scheduler-view .memory-content { color: #333; padding: 5px; }"
    ".scheduler-view .memory-content:hover { color: #196761; }"
    ".scheduler-view .memory-empty { color: #777; font-style: italic; padding: 5px; }"
    ".scheduler-view .memory-pcb { background-color: #f5f5f5; border-radius: 0 0 5px 5px; padding: 0; border: 1px solid #33A19A; }"
    ".scheduler-view .memory-pcb-row { padding: 5px 10px; border-bottom: 1px solid rgba(51, 161, 154, 0.2); }"
    ".scheduler-view .memory-pcb-row:last-child { border-bottom: none; }"
    ".scheduler-view .memory-slot { border-bottom: 1px solid #ccc; background-color: #f5f5f5; }"
    ".scheduler-view .memory-slot:hover .memory-content { color: #196761; }"
    ".scheduler-view .frame-title { background-color: #33A19A; color: white; padding: 5px; border-radius: 3px 3px 0 0; }"
    ".scheduler-view .pcb-tab { background-color: #33A19A; color: white; padding: 6px 12px; border-radius: 5px 5px 0 0; font-weight: bold; margin-bottom: 0; }"
    ".scheduler-view .process-title { background-color: #196761; color: white; padding: 4px 10px; border-radius: 5px 5px 0 0; font-weight: bold; margin-bottom: 0; font-size: 13px; }"
    ".scheduler-view .queue-area { background-color: #D9D9D9; border: 1px solid #bbb; border-radius: 5px; }"
    ".scheduler-view .running-process { background-color: #196761; color: white; font-size: 14px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; padding: 8px; border-radius: 5px; margin-top: 10px; box-shadow: 0 2px 4px rgba(0,0,0,0.2); }"
    ".scheduler-view .blocked-queue-header { background-color: #FF0000; color: white; padding: 5px; border-radius: 3px 3px 0 0; }"
    ".simulator-label { color: #333; font-size: 14px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; }"
    ".simulator-frame { background-color: rgb(236, 236, 234); border: 1px solid #bbb; color: #333; }"
    ".resource-label { margin: 2px 0; padding: 3px 5px; border-radius: 3px; color: #333; }"    ".resource-held { background-color: #ffcccc; border-left: 3px solid #ff3333; color: #333; }"
    ".resource-available { background-color: #ccffcc; border-left: 3px solid #33cc33; color: #333; }"
    ".resource-frame-title { background-color: #33A19A; color: white; padding: 5px; border-radius: 3px 3px 0 0; font-weight: bold; font-size: 14px; }"    ".resource-queue-header { color: white; background-color: #FF0000; padding: 5px; border-radius: 3px 3px 0 0; font-size: 14px; font-weight: bold; width: 100%; }" 
    ".resource-queue-content { background-color: #D9D9D9; border: 1px solid #bbb; border-radius: 5px; }"
    ".resource-process-item { margin: 3px; padding: 5px 8px; background-color: #FF0000; color: white; border-radius: 5px; box-shadow: 0 2px 4px rgba(0,0,0,0.2); }"
    ".resource-queue-empty { font-style: italic; color: #777; padding: 10px; }"
    ".blocked-queue-frame { border: 1px solid #FF0000; margin-bottom: 8px; }";

static void draw_queue_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
{
    int queue_index = GPOINTER_TO_INT(data);
    GList *processes = view->queue_processes[queue_index];
    GList *anim_iter = queue_animations[queue_index].animations;

    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    if (queue_index == 4)
    {
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    }
    else
    {
        cairo_set_source_rgb(cr, 0.2, 0.631, 0.604);
    }
    cairo_rectangle(cr, 0, 0, width, 25);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Roboto", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);

    if (queue_index == 4)
    {
        cairo_move_to(cr, 10, 18);
        cairo_show_text(cr, "Blocked Queue");
    }
    else
    {
        cairo_move_to(cr, 10, 18);
        char label[20];
        snprintf(label, sizeof(label), "Ready Queue %d", queue_index);
        cairo_show_text(cr, label);
    }

    GList *proc_iter;
    int process_count = 0;

    for (proc_iter = processes; proc_iter != NULL; proc_iter = proc_iter->next)
    {
        int pid = GPOINTER_TO_INT(proc_iter->data);
        ProcessAnimation *anim = NULL;

        for (GList *a = anim_iter; a != NULL; a = a->next)
        {
            ProcessAnimation *pa = (ProcessAnimation *)a->data;
            if (pa->pid == pid)
            {
                anim = pa;
                break;
            }
        }

        float x, y;
        float alpha = 1.0;

        if (queue_index == 4)
        {
            x = width / 2;
            y = 70 + process_count * 60;
        }
        else
        {
            x = 120 + process_count * 80;
            y = height / 2 + 5;
        }

        if (anim && anim->animating)
        {
            float t = (float)anim->steps / anim->total_steps;
            x = anim->start_x + t * (anim->end_x - anim->start_x);
            y = anim->start_y + t * (anim->end_y - anim->start_y);
            alpha = anim->alpha;
        }

        cairo_pattern_t *box_gradient = cairo_pattern_create_linear(x - 25, y - 20, x + 25, y + 20);

        if (pid == view->running_pid)
        {
            cairo_pattern_add_color_stop_rgba(box_gradient, 0, 0.098, 0.529, 0.380, alpha);
            cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.059, 0.388, 0.278, alpha);
        }
        else if (queue_index == 4)
        {
            cairo_pattern_add_color_stop_rgba(box_gradient, 0, 1.0, 0.0, 0.0, alpha);
            cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.8, 0.0, 0.0, alpha);
        }
        else
        {
            cairo_pattern_add_color_stop_rgba(box_gradient, 0, 0.6, 0.3, 0.7, alpha);
            cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.4, 0.1, 0.5, alpha);
        }

        cairo_set_source(cr, box_gradient);

        double box_radius = 8.0;
        double degrees = M_PI / 180.0;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + 20 - box_radius, y - 15 + box_radius, box_radius, -90 * degrees, 0 * degrees);
        cairo_arc(cr, x + 20 - box_radius, y + 15 - box_radius, box_radius, 0 * degrees, 90 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y + 15 - box_radius, box_radius, 90 * degrees, 180 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y - 15 + box_radius, box_radius, 180 * degrees, 270 * degrees);
        cairo_close_path(cr);

        cairo_fill(cr);

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

        char pid_str[16];
        snprintf(pid_str, sizeof(pid_str), "P%d", pid);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha);
        cairo_select_font_face(cr, "Roboto", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14);

        cairo_text_extents_t extents;
        cairo_text_extents(cr, pid_str, &extents);
        cairo_move_to(cr, x - extents.width / 2, y + extents.height / 2);
        cairo_show_text(cr, pid_str);

        if (proc_iter->next != NULL)
        {
            if (queue_index == 4)
            {
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
            }
            else
            {
                x += 70;
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_move_to(cr, x - 40, y);
                cairo_line_to(cr, x - 20, y);
                cairo_stroke(cr);
                cairo_move_to(cr, x - 20, y);
                cairo_line_to(cr, x - 25, y - 5);
                cairo_move_to(cr, x - 20, y);
                cairo_line_to(cr, x - 25, y + 5);
                cairo_stroke(cr);
            }
        }        process_count++;
    }
}

static void draw_mutex_queue_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
{
    int mutex_index = GPOINTER_TO_INT(data);
    const char *mutex_names[] = {"userInput", "userOutput", "file"};
    
    if (!view || mutex_index < 0 || mutex_index >= 3) return;
    
    GList *processes = view->mutex_queue_processes[mutex_index];

    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    // Draw header with red background (same as blocked queue)
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_rectangle(cr, 0, 0, width, 25);
    cairo_fill(cr);

    // Draw header text
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Roboto", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_move_to(cr, 10, 18);
    char queue_label[32];
    snprintf(queue_label, sizeof(queue_label), "%s Queue", mutex_names[mutex_index]);
    cairo_show_text(cr, queue_label);    GList *proc_iter;
    int process_count = 0;

    for (proc_iter = processes; proc_iter != NULL; proc_iter = proc_iter->next)
    {
        int pid = GPOINTER_TO_INT(proc_iter->data);
        
        float x, y;
        float alpha = 1.0;

        // Position processes horizontally instead of vertically
        x = 50 + process_count * 60;  // Horizontal spacing
        y = height / 2 + 10;  // Center vertically with slight offset for header

        // Create gradient for blocked process (red like blocked queue)
        cairo_pattern_t *box_gradient = cairo_pattern_create_linear(x - 25, y - 20, x + 25, y + 20);
        cairo_pattern_add_color_stop_rgba(box_gradient, 0, 1.0, 0.0, 0.0, alpha);
        cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.8, 0.0, 0.0, alpha);

        cairo_set_source(cr, box_gradient);

        // Draw rounded rectangle (same as blocked queue)
        double box_radius = 8.0;
        double degrees = M_PI / 180.0;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + 20 - box_radius, y - 15 + box_radius, box_radius, -90 * degrees, 0 * degrees);
        cairo_arc(cr, x + 20 - box_radius, y + 15 - box_radius, box_radius, 0 * degrees, 90 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y + 15 - box_radius, box_radius, 90 * degrees, 180 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y - 15 + box_radius, box_radius, 180 * degrees, 270 * degrees);
        cairo_close_path(cr);

        cairo_fill(cr);

        // Draw border
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

        // Draw process ID text
        char pid_str[16];
        snprintf(pid_str, sizeof(pid_str), "P%d", pid);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha);
        cairo_select_font_face(cr, "Roboto", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14);

        cairo_text_extents_t extents;
        cairo_text_extents(cr, pid_str, &extents);
        cairo_move_to(cr, x - extents.width / 2, y + extents.height / 2);
        cairo_show_text(cr, pid_str);        // Draw arrow to next process (if there is one)
        if (proc_iter->next != NULL)
        {
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_move_to(cr, x + 25, y);
            cairo_line_to(cr, x + 35, y);
            cairo_stroke(cr);
            cairo_move_to(cr, x + 35, y);
            cairo_line_to(cr, x + 30, y - 5);
            cairo_move_to(cr, x + 35, y);
            cairo_line_to(cr, x + 30, y + 5);
            cairo_stroke(cr);
        }

        process_count++;
    }
}

static void draw_legend_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
{
    int x = 30;
    int y = height / 2;
    double circle_radius = 12;
    double degrees = M_PI / 180.0;

    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    cairo_pattern_t *ready_gradient = cairo_pattern_create_radial(x, y, 0, x, y, circle_radius);
    cairo_pattern_add_color_stop_rgba(ready_gradient, 0, 0.6, 0.3, 0.7, 1.0);
    cairo_pattern_add_color_stop_rgba(ready_gradient, 1, 0.4, 0.1, 0.5, 1.0);
    cairo_set_source(cr, ready_gradient);
    cairo_arc(cr, x, y, circle_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(ready_gradient);

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_select_font_face(cr, "Roboto", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "Ready");

    x += 100;

    cairo_pattern_t *running_gradient = cairo_pattern_create_radial(x, y, 0, x, y, circle_radius);
    cairo_pattern_add_color_stop_rgba(running_gradient, 0, 0.098, 0.529, 0.380, 1.0);
    cairo_pattern_add_color_stop_rgba(running_gradient, 1, 0.059, 0.388, 0.278, 1.0);
    cairo_set_source(cr, running_gradient);
    cairo_arc(cr, x, y, circle_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(running_gradient);

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "Running");

    x += 100;

    cairo_pattern_t *blocked_gradient = cairo_pattern_create_radial(x, y, 0, x, y, circle_radius);
    cairo_pattern_add_color_stop_rgba(blocked_gradient, 0, 1.0, 0.0, 0.0, 1.0);
    cairo_pattern_add_color_stop_rgba(blocked_gradient, 1, 0.8, 0.0, 0.0, 1.0);
    cairo_set_source(cr, blocked_gradient);
    cairo_arc(cr, x, y, circle_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(blocked_gradient);

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "Blocked");
}

static gboolean animate_process(gpointer user_data)
{
    int queue_index = GPOINTER_TO_INT(user_data);
    GList *anim_iter = queue_animations[queue_index].animations;
    gboolean any_animating = FALSE;

    for (GList *a = anim_iter; a != NULL; a = a->next)
    {
        ProcessAnimation *anim = (ProcessAnimation *)a->data;
        if (anim->animating)
        {
            anim->steps++;
            if (anim->steps >= anim->total_steps)
            {
                anim->animating = 0;
                anim->alpha = 1.0;
            }
            else
            {
                float progress = (float)anim->steps / anim->total_steps;
                anim->alpha = progress < 0.5 ? 4 * progress * progress * progress : 1 - pow(-2 * progress + 2, 3) / 2;
                any_animating = TRUE;
            }
        }
    }

    gtk_widget_queue_draw(view->drawing_areas[queue_index]);
    return any_animating ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

void view_init(GtkWidget *window, GtkWidget *main_box)
{
    view = g_new0(View, 1);
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css_provider, css_data);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    view->window = window;
    gtk_widget_add_css_class(window, "scheduler-view");

    // Use provided main_box instead of creating a new one
    g_print("view_init: using main_box: %p\n", main_box);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_box_append(GTK_BOX(main_box), grid);

    view->drawing_areas[4] = gtk_drawing_area_new();
    gtk_widget_set_size_request(view->drawing_areas[4], 130, 320);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(view->drawing_areas[4]),
                                   draw_queue_callback,
                                   GINT_TO_POINTER(4),
                                   NULL);
    gtk_widget_add_css_class(view->drawing_areas[4], "queue-area");
    gtk_grid_attach(GTK_GRID(grid), view->drawing_areas[4], 0, 0, 1, 4);

    for (int i = 0; i < 4; i++)
    {
        view->drawing_areas[i] = gtk_drawing_area_new();
        gtk_widget_set_size_request(view->drawing_areas[i], 720, 90);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(view->drawing_areas[i]),
                                       draw_queue_callback,
                                       GINT_TO_POINTER(i),
                                       NULL);
        gtk_widget_add_css_class(view->drawing_areas[i], "queue-area");
        gtk_grid_attach(GTK_GRID(grid), view->drawing_areas[i], 1, i, 1, 1);
    }

    GtkWidget *legend_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(legend_area, 720, 40);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(legend_area),
                                   draw_legend_callback,
                                   NULL,
                                   NULL);
    gtk_widget_add_css_class(legend_area, "queue-area");
    gtk_grid_attach(GTK_GRID(grid), legend_area, 0, 4, 2, 1);

    view->running_process_label = gtk_label_new("Process Info: None");
    gtk_widget_add_css_class(view->running_process_label, "running-process");
    gtk_label_set_xalign(GTK_LABEL(view->running_process_label), 0.0);
    gtk_widget_set_margin_top(view->running_process_label, 10);
    gtk_box_append(GTK_BOX(main_box), view->running_process_label);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_margin_top(button_box, 15);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    
    GtkWidget *scheduler_label = gtk_label_new("Scheduler:");
    gtk_widget_add_css_class(scheduler_label, "label");
    gtk_box_append(GTK_BOX(button_box), scheduler_label);

    GtkStringList *scheduler_list = gtk_string_list_new(NULL);
    gtk_string_list_append(scheduler_list, "MLFQ");
    gtk_string_list_append(scheduler_list, "FCFS");
    gtk_string_list_append(scheduler_list, "Round Robin");
    view->scheduler_combo = gtk_drop_down_new(G_LIST_MODEL(scheduler_list), NULL);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(view->scheduler_combo), 0);
    gtk_widget_add_css_class(view->scheduler_combo, "drop-down");
    gtk_box_append(GTK_BOX(button_box), view->scheduler_combo);

    view->quantum_label = gtk_label_new("Quantum:");
    gtk_widget_add_css_class(view->quantum_label, "label");
    view->quantum_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(view->quantum_entry), "2");
    gtk_entry_set_max_length(GTK_ENTRY(view->quantum_entry), 3);
    gtk_widget_set_size_request(view->quantum_entry, 50, -1);
    gtk_widget_add_css_class(view->quantum_entry, "entry");
    gtk_widget_set_visible(view->quantum_label, FALSE);
    gtk_widget_set_visible(view->quantum_entry, FALSE);
    gtk_box_append(GTK_BOX(button_box), view->quantum_label);
    gtk_box_append(GTK_BOX(button_box), view->quantum_entry);

    view->step_button = gtk_button_new_with_label("Step");
    gtk_widget_add_css_class(view->step_button, "button");
    view->automatic_button = gtk_button_new_with_label("Automatic");
    gtk_widget_add_css_class(view->automatic_button, "button");
    view->pause_button = gtk_button_new_with_label("Pause");
    gtk_widget_add_css_class(view->pause_button, "button");
    view->reset_button = gtk_button_new_with_label("Reset");
    gtk_widget_add_css_class(view->reset_button, "button");
    gtk_widget_set_sensitive(view->pause_button, FALSE);

    gtk_box_append(GTK_BOX(button_box), view->step_button);
    gtk_box_append(GTK_BOX(button_box), view->automatic_button);
    gtk_box_append(GTK_BOX(button_box), view->pause_button);
    gtk_box_append(GTK_BOX(button_box), view->reset_button);

    gtk_box_append(GTK_BOX(main_box), button_box);    for (int i = 0; i < 5; i++)
    {
        view->queue_processes[i] = NULL;
        queue_animations[i].animations = NULL;
    }
    
    // Initialize mutex queue processes
    for (int i = 0; i < 3; i++)
    {
        view->mutex_queue_processes[i] = NULL;
    }
    
    view->running_pid = -1;
    
    // Initialize the resource panel to NULL - it will be created separately if needed
    view->resource_panel = NULL;
}

// Create a separate container for the right panel (resource management)
void view_create_right_panel(GtkWidget *parent_container) {
    if (!parent_container || !view) return;
    
    // Create right panel container
    GtkWidget *right_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(right_container, TRUE);
    gtk_widget_set_halign(right_container, GTK_ALIGN_FILL);
    gtk_widget_set_size_request(right_container, 300, -1); // Adjust width to better fit with console
    gtk_box_append(GTK_BOX(parent_container), right_container);
    
    // Create resource panel within the right container
    view_create_resource_panel(view, right_container);
}

static int get_position_in_queue(GList *queue, int pid)
{
    int pos = 0;
    for (GList *p = queue; p != NULL; p = p->next)
    {
        if (GPOINTER_TO_INT(p->data) == pid)
        {
            return pos;
        }
        pos++;
    }
    return -1;
}

static void get_process_coords(int queue_index, int position, float *x, float *y)
{
    if (queue_index == 4)
    {
        *x = 65;
        *y = 70 + position * 60;
    }
    else
    {
        *x = 120 + position * 80;
        *y = 40 + queue_index * 75;
    }
}

void view_update_queue(int queue_index, GList *processes, int running_pid)
{
    if (queue_index < 0 || queue_index >= 5)
        return;

    g_list_free_full(queue_animations[queue_index].animations, g_free);
    queue_animations[queue_index].animations = NULL;

    GList *old_processes = view->queue_processes[queue_index];
    GList *new_processes = processes;

    int new_position = 0;
    for (GList *proc_iter = processes; proc_iter != NULL; proc_iter = proc_iter->next)
    {
        int pid = GPOINTER_TO_INT(proc_iter->data);
        ProcessAnimation *anim = g_new0(ProcessAnimation, 1);

        anim->pid = pid;
        anim->animating = 1;
        anim->steps = 0;
        anim->total_steps = 20;

        int old_queue = -1;
        int old_position = -1;

        for (int q = 0; q < 5; q++)
        {
            int pos = get_position_in_queue(view->queue_processes[q], pid);
            if (pos != -1)
            {
                old_queue = q;
                old_position = pos;
                break;
            }
        }

        get_process_coords(queue_index, new_position, &anim->end_x, &anim->end_y);

        if (old_queue == -1)
        {
            anim->start_x = anim->end_x;
            anim->start_y = anim->end_y;
            anim->alpha = 0.0;
        }
        else
        {
            get_process_coords(old_queue, old_position, &anim->start_x, &anim->start_y);
            anim->alpha = 1.0;
        }

        queue_animations[queue_index].animations = g_list_append(queue_animations[queue_index].animations, anim);
        new_position++;
    }

    g_list_free(view->queue_processes[queue_index]);
    view->queue_processes[queue_index] = g_list_copy(processes);

    view->running_pid = running_pid;

    g_timeout_add(16, animate_process, GINT_TO_POINTER(queue_index));
}

GtkWidget *view_get_running_process_label()
{
    return view->running_process_label;
}

GtkWidget *view_get_step_button()
{
    return view->step_button;
}

GtkWidget *view_get_automatic_button()
{
    return view->automatic_button;
}

GtkWidget *view_get_pause_button()
{
    return view->pause_button;
}

GtkWidget *view_get_reset_button()
{
    return view->reset_button;
}

GtkWidget *view_get_scheduler_combo()
{
    return view->scheduler_combo;
}

GtkWidget *view_get_quantum_entry()
{
    return view->quantum_entry;
}

GtkWidget *view_get_quantum_label()
{
    return view->quantum_label;
}

// Helper function to create a status label for resource panel
static GtkWidget* create_status_label(const char *text) {
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_add_css_class(label, "simulator-label");
    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    return label;
}

// Create the resource management panel
void view_create_resource_panel(View *view, GtkWidget *parent) {
    if (!view || !parent) return;
    
    view->resource_panel = g_new(ResourcePanel, 1);
    // We'll use the parent container directly since it's already properly setup
    GtkWidget *resource_container = parent;
    gtk_widget_set_vexpand(resource_container, TRUE);
    gtk_widget_set_margin_start(resource_container, 5);
    gtk_widget_set_margin_end(resource_container, 5);

    // Resource Status frame
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_add_css_class(frame, "simulator-frame");
    gtk_widget_add_css_class(frame, "scheduler-view");
    gtk_widget_set_margin_bottom(frame, 10);  // Add margin between resource status and queues
    gtk_box_append(GTK_BOX(resource_container), frame);
    
    // Add title with background
    GtkWidget *resource_title = gtk_label_new("Resource Status");
    gtk_widget_add_css_class(resource_title, "resource-frame-title");
    
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(content_box, 5);
    gtk_widget_set_margin_end(content_box, 5);
    gtk_widget_set_margin_top(content_box, 5);
    gtk_widget_set_margin_bottom(content_box, 5);
    
    GtkWidget *resource_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(resource_box), resource_title);
    gtk_box_append(GTK_BOX(resource_box), content_box);
    gtk_frame_set_child(GTK_FRAME(frame), resource_box);
    
    // Create labels for mutex status with enhanced styling
    const char *mutex_names[] = {"userInput", "userOutput", "file"};
    for (int i = 0; i < 3; i++) {
        GtkWidget *label = create_status_label("");
        gtk_widget_add_css_class(label, "resource-label");
        view->resource_panel->mutex_labels[i] = label;
        gtk_box_append(GTK_BOX(content_box), label);
    }
    
    // Create drawing areas for each mutex queue (styled like blocked queue)
    for (int i = 0; i < 3; i++) {
        view->resource_panel->mutex_drawing_areas[i] = gtk_drawing_area_new();
        gtk_widget_set_size_request(view->resource_panel->mutex_drawing_areas[i], 280, 70); // Reduced height
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(view->resource_panel->mutex_drawing_areas[i]),
                                       draw_mutex_queue_callback,
                                       GINT_TO_POINTER(i),
                                       NULL);
        gtk_widget_add_css_class(view->resource_panel->mutex_drawing_areas[i], "queue-area");
        gtk_widget_set_margin_bottom(view->resource_panel->mutex_drawing_areas[i], 8);
        gtk_box_append(GTK_BOX(resource_container), view->resource_panel->mutex_drawing_areas[i]);
    }

    view_update_resource_panel();
}

// Helper function to create a process box for the resource panel
static GtkWidget* create_process_box(int pid, int priority) {
    // Create a box for the process with rounded corners
    GtkWidget *process_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(process_box, 40, 40); // Make it square
    gtk_widget_set_halign(process_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(process_box, GTK_ALIGN_CENTER);
    
    // Create a label for the process ID
    char pid_str[20];
    snprintf(pid_str, sizeof(pid_str), "P%d", pid);
    GtkWidget *pid_label = gtk_label_new(pid_str);
    gtk_label_set_justify(GTK_LABEL(pid_label), GTK_JUSTIFY_CENTER);
    gtk_widget_add_css_class(pid_label, "blocked-process");
    
    gtk_box_append(GTK_BOX(process_box), pid_label);
    gtk_widget_add_css_class(process_box, "blocked-process");
    
    return process_box;
}

// Update the resource management panel
void view_update_resource_panel(void) {
    if (!view || !view->resource_panel) return;
    
    // Get the mutex status
    const char *mutex_names[] = {"userInput", "userOutput", "file"};
    int i;
    
    // Update mutex status labels
    for (i = 0; i < 3; i++) {
        mutex_t *mutex = get_mutex_by_name(mutex_names[i]);
        char text[100];
        
        if (mutex_is_available(mutex)) {
            snprintf(text, sizeof(text), "%s: Available", mutex_names[i]);
            gtk_widget_remove_css_class(view->resource_panel->mutex_labels[i], "resource-held");
            gtk_widget_add_css_class(view->resource_panel->mutex_labels[i], "resource-available");
        } else {
            Process *holder = mutex_get_holder(mutex);
            snprintf(text, sizeof(text), "%s: Held by Process %d", 
                    mutex_names[i], holder ? holder->pid : -1);
            gtk_widget_remove_css_class(view->resource_panel->mutex_labels[i], "resource-available");
            gtk_widget_add_css_class(view->resource_panel->mutex_labels[i], "resource-held");
        }
        gtk_label_set_text(GTK_LABEL(view->resource_panel->mutex_labels[i]), text);
    }    
    // Update mutex queues by rebuilding the process lists and redrawing
    for (i = 0; i < 3; i++) {
        mutex_t *mutex = get_mutex_by_name(mutex_names[i]);
        
        // Clear the existing process list
        g_list_free(view->mutex_queue_processes[i]);
        view->mutex_queue_processes[i] = NULL;
        
        // Rebuild the process list from the mutex blocked queue
        int blocked_count = mutex_get_blocked_count(mutex);
        for (int j = 0; j < blocked_count; j++) {
            Process *p = mutex_get_blocked_process(mutex, j);
            if (p) {
                view->mutex_queue_processes[i] = g_list_append(view->mutex_queue_processes[i], GINT_TO_POINTER(p->pid));
            }
        }
        
        // Trigger a redraw of the drawing area
        gtk_widget_queue_draw(view->resource_panel->mutex_drawing_areas[i]);
    }
}

// Reset the resource panel
void view_reset_resource_panel(void) {
    if (!view || !view->resource_panel) return;
    
    // Reset all mutexes in the backend
    reset_all_mutexes();
}