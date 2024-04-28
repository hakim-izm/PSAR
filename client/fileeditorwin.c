#include <gtk/gtk.h>

#include "fileeditor.h"
#include "fileeditorwin.h"

struct _FileEditorWindow
{
	GtkApplicationWindow parent;

	GSettings *settings;
	GtkWidget *appmenu;
	GtkWidget *stack;
	GtkWidget *grid;
};

G_DEFINE_TYPE(FileEditorWindow, file_editor_window, GTK_TYPE_APPLICATION_WINDOW);

/*
 * CALLBACKS
 */
void row_selected(GtkListBox *listbox, GtkListBoxRow *row, FileEditorWindow *win)
{
	GtkLabel *lineSelectedLabel;

	lineSelectedLabel = GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(win->grid), 0, 0));

	if(row){
		int index = gtk_list_box_row_get_index(row);
		char *label_text = g_strdup_printf("Line selected: %d", index);
		gtk_label_set_text(lineSelectedLabel, label_text);
		g_free(label_text);
	}
	else{
		lineSelectedLabel = GTK_LABEL(gtk_label_new("No line selected"));
	}
}

/*
 * OVERRIDEN METHODS
 */

static void file_editor_window_init(FileEditorWindow *win)
{
	GtkBuilder *builder;
	GMenuModel *menu;

	gtk_widget_init_template(GTK_WIDGET(win));

	builder = gtk_builder_new_from_resource("/com/psar/fileeditor/menu.ui");
	menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(win->appmenu), menu);
	g_object_unref(builder);

	win->settings = g_settings_new("com.psar.fileeditor");
}

static void file_editor_window_class_init(FileEditorWindowClass *class)
{
	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/com/psar/fileeditor/window.ui");
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorWindow, stack);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorWindow, appmenu);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorWindow, grid);
}

FileEditorWindow *file_editor_window_new(FileEditor *app)
{
	return g_object_new(FILE_EDITOR_WINDOW_TYPE, "application", app, NULL);
}

void file_editor_window_open(FileEditorWindow *win, GFile *file)
{
	char *basename;
	GtkWidget *scrolled;
	GtkTextBuffer *buffer;
	char *contents;
	gsize length;
	GtkTextTag *tag;

	basename = g_file_get_basename(file);

	scrolled = gtk_scrolled_window_new();
	gtk_widget_set_hexpand(scrolled, TRUE);
	gtk_widget_set_vexpand(scrolled, TRUE);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget *listbox = gtk_list_box_new();
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_SINGLE);
	g_signal_connect(listbox, "row-selected", G_CALLBACK(row_selected), win);

	if (g_file_load_contents(file, NULL, &contents, &length, NULL, NULL))
	{
		gchar **lines = g_strsplit(contents, "\n", -1);
		gint num_lines = g_strv_length(lines);
		for (int i = 0; i < num_lines; i++)
		{
			buffer = gtk_text_buffer_new(NULL);
			gtk_text_buffer_set_text(buffer, lines[i], -1);

			tag = gtk_text_buffer_create_tag(buffer, NULL, NULL);
			g_settings_bind(win->settings, "font", tag, "font", G_SETTINGS_BIND_DEFAULT);

			GtkWidget *view = gtk_text_view_new_with_buffer(buffer);
			gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
			gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);

			GtkWidget *row = gtk_list_box_row_new();
			gtk_widget_set_hexpand(view, TRUE);
			gtk_widget_set_vexpand(view, FALSE); // Only expand horizontally for each line
			gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), view);
			gtk_list_box_append(GTK_LIST_BOX(listbox), row);

			g_object_unref(buffer);
		}
		g_strfreev(lines);
		g_free(contents);
	}

	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), listbox);
	gtk_stack_add_titled(GTK_STACK(win->stack), scrolled, basename, basename);

	g_free(basename);
}
