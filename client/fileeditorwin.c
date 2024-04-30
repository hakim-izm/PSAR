#include <gtk/gtk.h>

#include "fileeditor.h"
#include "fileeditorwin.h"
#include "api/client.h"

#define MAX_FILES 8

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
 * GLOBAL VARIABLES
 */

File *open_files[MAX_FILES];

/*
 * FUNCTIONS
 */
File * get_file_struct_from_filename(const char *filename)
{
	for(int i = 0; i < MAX_FILES; i++) {
		if(open_files[i] && strcmp(open_files[i]->filename, filename) == 0) {
			return open_files[i];
		}
	}
	return NULL;
}

LineNode * get_line_node_from_text(File * file_struct, char *text)
{
	LineNode *curr = file_struct->lines;
	while(curr) {
		if(strcmp(curr->line->text, text) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

/*
 * CALLBACKS
 */
void row_selected(GtkListBox *listbox, GtkListBoxRow *row, FileEditorWindow *win)
{
	GtkLabel *lineSelectedLabel;
	GtkWidget *textView;

	File *file_struct = get_file_struct_from_filename(gtk_stack_get_visible_child_name(GTK_STACK(win->stack)));

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

	// test : get LineNode from text
	textView = gtk_widget_get_first_child(GTK_WIDGET(row));

	// un seul fils par row
	if(GTK_IS_TEXT_VIEW(textView)) {
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
		GtkTextIter start, end;
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		gchar *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
		LineNode *lineNode = get_line_node_from_text(file_struct, text);
		if(lineNode) {
			g_print("Line found: %s\n", lineNode->line->text);
		}
		else {
			g_print("Line not found\n");
		}
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
	char *filepath;
	GtkWidget *scrolled;
	GtkTextBuffer *buffer;
	char *contents;
	gsize length;
	GtkTextTag *tag;

	filepath = g_file_get_path(file);

	File *file_struct;

	for(int i = 0; i < MAX_FILES; i++) {
		if(!open_files[i]) {
			file_struct = open_files[i] = open_local_file(filepath);
			break;
		}

		if(i == MAX_FILES - 1) {
			// dialog error
			GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Too many files opened");
			
			gtk_window_present(GTK_WINDOW(dialog));

			g_free(filepath);
			return;
		}
	}

	scrolled = gtk_scrolled_window_new();
	gtk_widget_set_hexpand(scrolled, TRUE);
	gtk_widget_set_vexpand(scrolled, TRUE);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget *listbox = gtk_list_box_new();
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_SINGLE);
	// g_signal_connect(listbox, "row-selected", G_CALLBACK(row_selected), win);
	g_signal_connect(listbox, "row-selected", G_CALLBACK(row_selected), win);

	LineNode *curr = file_struct->lines;

	while(curr) {
		Line *line = curr->line;

		buffer = gtk_text_buffer_new(NULL);
		gtk_text_buffer_set_text(buffer, line->text, -1);

		tag = gtk_text_buffer_create_tag(buffer, NULL, NULL);
		g_settings_bind(win->settings, "font", tag, "font", G_SETTINGS_BIND_DEFAULT);

		GtkWidget *view = gtk_text_view_new_with_buffer(buffer);
		gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
		gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);

		GtkWidget *row = gtk_list_box_row_new();
		gtk_widget_set_hexpand(view, TRUE);
		gtk_widget_set_vexpand(view, FALSE);
		gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), view);
		gtk_list_box_append(GTK_LIST_BOX(listbox), row);

		g_object_unref(buffer);

		curr = curr->next;
	}

	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), listbox);
	gtk_stack_add_titled(GTK_STACK(win->stack), scrolled, file_struct->filename, file_struct->filename);

	g_free(filepath);
}