#include <gtk/gtk.h>

#include "fileeditor.h"
#include "fileeditorwin.h"
#include "api/client.h"

#define MAX_FILES 8



G_DEFINE_TYPE(FileEditorWindow, file_editor_window, GTK_TYPE_APPLICATION_WINDOW);

/*
 * GLOBAL VARIABLES
 */

File *open_files[MAX_FILES];
GtkListBox *listboxes[MAX_FILES];
Line *selected_line;

/*
 * FUNCTIONS
 */

void save_file_call(File * file, const char * filepath) {
	printf("save_file_call\n");
	if(!file) {
		fprintf(stderr, "CLIENT API: No save to file (save_file_call())\n");
		return;
	}

	const char *path = filepath ? filepath : file->filename;

	save_file(file, path);
}

void empty_listbox(GtkListBox *listbox)
{
	GtkWidget *child;

	while ((child = gtk_widget_get_first_child(GTK_WIDGET (listbox))))
		gtk_list_box_remove(listbox, child);
}

void fill_listbox(GtkListBox *listbox, File *file_struct)
{
	GtkTextBuffer *buffer;
	GtkTextTag *tag;
	FileEditorWindow *win = FILE_EDITOR_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(listbox), FILE_EDITOR_WINDOW_TYPE));

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

		g_object_unref(buffer);

		// Associate line_node with row

		gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(row), TRUE);
		gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(row), TRUE);
		g_object_set_data(G_OBJECT(row), "line_node", curr);

		gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);


		curr = curr->next;
	}
}

/*
 * CALLBACKS
 */
void on_row_selected(GtkListBox *listbox, GtkListBoxRow *row, FileEditorWindow *win)
{
	GtkLabel *lineSelectedLabel;
	GtkWidget *textView;

	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");


	lineSelectedLabel = GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(win->grid), 0, 0));

	if(row) {
		LineNode *lineNode = g_object_get_data(G_OBJECT(row), "line_node");
		if(lineNode) {
			g_print("Line found: %s\n", lineNode->line->text);
			selected_line = lineNode->line;
			char *label_text = g_strdup_printf("Line selected: %d", lineNode->line->id);
			gtk_label_set_text(lineSelectedLabel, label_text);
			g_free(label_text);
		} else {
			lineSelectedLabel = GTK_LABEL(gtk_label_new("No line selected"));
			g_print("Line not found\n");
		}
	
	} else {
		lineSelectedLabel = GTK_LABEL(gtk_label_new("No line selected"));
	}
}

void on_delete_clicked(GtkButton *button, FileEditorWindow *win)
{
	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");

	if(!selected_line) {
		// dialog error
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "No line selected");
		
		gtk_window_present(GTK_WINDOW(dialog));

		return;
	}

	remove_line(file_struct, selected_line);

	int i;
	for(i = 0; i < MAX_FILES; i++) {
		if(open_files[i] == file_struct) {
			break;
		}
	}

	if(i == MAX_FILES) {
		// dialog error
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "File not found");
		
		gtk_window_present(GTK_WINDOW(dialog));

		return;
	}

	GtkListBox *listbox = listboxes[i];

	empty_listbox(listbox);
	fill_listbox(listbox, file_struct);

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
	GtkButton *add_btn, *edit_btn, *delete_btn;

	// BUTTONS

	add_btn = GTK_BUTTON(gtk_grid_get_child_at(GTK_GRID(win->grid), 1, 0));
	edit_btn = GTK_BUTTON(gtk_grid_get_child_at(GTK_GRID(win->grid), 2, 0));
	delete_btn = GTK_BUTTON(gtk_grid_get_child_at(GTK_GRID(win->grid), 3, 0));

	gtk_widget_set_visible(GTK_WIDGET(add_btn), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(edit_btn), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(delete_btn), TRUE);

	// g_signal_connect(add_btn, "clicked", G_CALLBACK(on_add_clicked), win);
	// g_signal_connect(edit_btn, "clicked", G_CALLBACK(on_edit_clicked), win);
	g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_clicked), win);

	// FILE

	filepath = g_file_get_path(file);

	File *file_struct;

	int i;

	for(i = 0; i < MAX_FILES; i++) {
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

	// LISTBOX STUFF

	scrolled = gtk_scrolled_window_new();
	gtk_widget_set_hexpand(scrolled, TRUE);
	gtk_widget_set_vexpand(scrolled, TRUE);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget *listbox = gtk_list_box_new();
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_SINGLE);
	g_signal_connect(listbox, "row-selected", G_CALLBACK(on_row_selected), win);
	listboxes[i] = GTK_LIST_BOX(listbox);

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

		g_object_unref(buffer);

		// Associate line_node with row

		gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(row), TRUE);
		gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(row), TRUE);
		g_object_set_data(G_OBJECT(row), "line_node", curr);

		gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);


		curr = curr->next;
	}

	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), listbox);
	gtk_stack_add_titled(GTK_STACK(win->stack), scrolled, file_struct->filename, file_struct->filename);

	// associate file_struct with stack child
	g_object_set_data(G_OBJECT(scrolled), "file_struct", file_struct);

	g_free(filepath);
}