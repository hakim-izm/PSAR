#include <gtk/gtk.h>

#include "fileeditor.h"
#include "fileeditorwin.h"
#include "api/client.h"

#define MAX_FILES 8



G_DEFINE_TYPE(FileEditorWindow, file_editor_window, GTK_TYPE_APPLICATION_WINDOW);

/*
 * GLOBAL VARIABLES
 */

Line *selected_line;

/*
 * FUNCTIONS
 */

void save_file_call(File * file, const char * filepath, FileEditorWindow *win) {
	printf("save_file_call\n");
	if(!file) {
		fprintf(stderr, "CLIENT API: No save to file (save_file_call())\n");
		return;
	}

	// copie locale dans le répertoire courant si filepath est NULL
	const char *path = filepath ? filepath : file->filename;

	local_save_file(file, path);

	// win->dirty = FALSE;
}

GtkListBoxRow * get_list_box_row_from_line_node(GtkListBox *listbox, LineNode *line_node)
{
	GtkWidget *child;

	child = gtk_widget_get_first_child(GTK_WIDGET(listbox));

	while(child) {
		GtkListBoxRow *row = GTK_LIST_BOX_ROW(child);
		if(row){
			LineNode *row_line_node = g_object_get_data(G_OBJECT(row), "line_node");
			if(row_line_node == line_node) {
				return row;
			}
		}

		child = gtk_widget_get_next_sibling(child);
	}

	return NULL;
}

LineNode * get_line_node_from_line(Line *line, File *file_struct)
{
	LineNode *curr = file_struct->lines;

	while(curr) {
		if(curr->line == line) {
			return curr;
		}

		curr = curr->next;
	}

	return NULL;
}

void empty_listbox(GtkListBox *listbox)
{
	GtkWidget *child;

	while ((child = gtk_widget_get_first_child(GTK_WIDGET (listbox))))
		gtk_list_box_remove(listbox, child);
}

void update_listbox_row_text(GtkListBoxRow *row, const gchar *text) {
	GtkWidget *textview = gtk_widget_get_first_child(GTK_WIDGET(row));
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(buffer, text, -1);
}

void insert_row_to_listbox(GtkListBox *listbox, LineNode *prev_line_node, LineNode *new_line_node) {
	GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, new_line_node->line->text, -1);

	GtkWidget *view = gtk_text_view_new_with_buffer(buffer);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);

	GtkWidget *row = gtk_list_box_row_new();
	gtk_widget_set_hexpand(view, TRUE);
	gtk_widget_set_vexpand(view, FALSE);
	gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), view);

	g_object_set_data(G_OBJECT(row), "line_node", new_line_node);

	if (prev_line_node == NULL) {
		// Insérer en début de liste
		gtk_list_box_insert(GTK_LIST_BOX(listbox), row, 0);
	} else {
		// Trouver le row correspondant au prev_line_node
		GtkListBoxRow *prev_row = get_list_box_row_from_line_node(listbox, prev_line_node);
		// Insérer après le row précédent
		int index = gtk_list_box_row_get_index(prev_row);
		gtk_list_box_insert(GTK_LIST_BOX(listbox), row, index + 1);
	}
}

void remove_row_from_listbox(GtkListBox *listbox, GtkListBoxRow *row) {
	gtk_list_box_remove(listbox, GTK_WIDGET(row));
}

void update_row_in_listbox(GtkListBoxRow *row, const gchar *text) {
	update_listbox_row_text(row, text);
}

void fill_listbox(GtkListBox *listbox, File *file_struct) {
	LineNode *curr = file_struct->lines;
	LineNode *prev = NULL;

	while (curr) {
		insert_row_to_listbox(listbox, prev, curr);
		prev = curr;
		curr = curr->next;
	}
}

/*
 * CALLBACKS
 */
void on_cancel_edit_clicked(GtkButton *button, FileEditorWindow *win)
{
	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");
	GtkWidget *scrolled = gtk_widget_get_ancestor(GTK_WIDGET(visible_child), GTK_TYPE_SCROLLED_WINDOW);
	GtkListBox *listbox = GTK_LIST_BOX(gtk_widget_get_first_child(gtk_widget_get_first_child(scrolled)));

	gtk_widget_set_visible(GTK_WIDGET(win->main_mode_grid), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(win->edition_grid), FALSE);

	gtk_list_box_set_selection_mode(listbox, GTK_SELECTION_SINGLE);
}

void on_confirm_edit_clicked(GtkButton *button, FileEditorWindow *win)
{
	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");
	GtkWidget *scrolled = gtk_widget_get_ancestor(GTK_WIDGET(visible_child), GTK_TYPE_SCROLLED_WINDOW);
	GtkListBox *listbox = GTK_LIST_BOX(gtk_widget_get_first_child(gtk_widget_get_first_child(scrolled)));
	GtkWidget *edition_grid = win->edition_grid;

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_widget_get_first_child(GTK_WIDGET(edition_grid))));
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);

	char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

	local_edit_line(selected_line, text);

	win->dirty = TRUE;
	
	GtkListBoxRow *selected_row = get_list_box_row_from_line_node(listbox, get_line_node_from_line(selected_line, file_struct));
	if(selected_row)
		update_row_in_listbox(selected_row, text);

	g_free(text);

	gtk_widget_set_visible(GTK_WIDGET(win->main_mode_grid), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(win->edition_grid), FALSE);

	gtk_list_box_set_selection_mode(listbox, GTK_SELECTION_SINGLE);

	gtk_list_box_select_row(listbox, selected_row);
	gtk_widget_grab_focus(GTK_WIDGET(selected_row));

	// empty_listbox(listbox);
	// fill_listbox(listbox, file_struct);
}

void on_row_selected(GtkListBox *listbox, GtkListBoxRow *row, FileEditorWindow *win)
{
	GtkLabel *lineSelectedLabel;
	GtkWidget *textView;

	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");


	lineSelectedLabel = GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(win->main_mode_grid), 0, 0));

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

void on_add_clicked(GtkButton *button, FileEditorWindow *win)
{
	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");
	GtkWidget *scrolled = gtk_widget_get_ancestor(GTK_WIDGET(visible_child), GTK_TYPE_SCROLLED_WINDOW);
	GtkListBox *listbox = GTK_LIST_BOX(gtk_widget_get_first_child(gtk_widget_get_first_child(scrolled)));
	
	if(!selected_line) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "No line selected");
		gtk_window_present(GTK_WINDOW(dialog));
		return;
	}

	LineNode *selected_line_node = get_line_node_from_line(selected_line, file_struct);

	local_add_line(file_struct, selected_line_node, "New line", ADD_INSERT);

	win->dirty = TRUE;

	insert_row_to_listbox(listbox, selected_line_node, selected_line_node->next);

	GtkListBoxRow *new_row = get_list_box_row_from_line_node(listbox, selected_line_node->next);
	if(new_row){
		gtk_list_box_select_row(listbox, new_row);
		gtk_widget_grab_focus(GTK_WIDGET(new_row));
	}

/*
	empty_listbox(listbox);
	fill_listbox(listbox, file_struct);

	// Sélection de la nouvelle ligne
	GtkListBoxRow *selected_row = get_list_box_row_from_line_node(listbox, selected_line_node->next);
	if(selected_row)
		gtk_list_box_select_row(listbox, selected_row);
*/

}

void on_edit_clicked(GtkButton *button, FileEditorWindow *win)
{
	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");
	GtkWidget *edition_grid = win->edition_grid;
	GtkButton *cancel_edit_btn, *confirm_edit_btn;
	GtkWidget *scrolled = gtk_widget_get_ancestor(GTK_WIDGET(visible_child), GTK_TYPE_SCROLLED_WINDOW);
	GtkListBox *listbox = GTK_LIST_BOX(gtk_widget_get_first_child(gtk_widget_get_first_child(scrolled)));

	if(!selected_line) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "No line selected");
		gtk_window_present(GTK_WINDOW(dialog));
		return;
	}

	gtk_list_box_set_selection_mode(listbox, GTK_SELECTION_NONE);

	gtk_widget_set_visible(GTK_WIDGET(win->main_mode_grid), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(win->edition_grid), TRUE);

	// TEXTVIEW

	GtkTextBuffer *buffer;

	buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, selected_line->text, -1);

	GtkTextTag *tag = gtk_text_buffer_create_tag(buffer, NULL, NULL);
	g_settings_bind(win->settings, "font", tag, "font", G_SETTINGS_BIND_DEFAULT);

	GtkWidget *textview = gtk_widget_get_first_child(GTK_WIDGET(edition_grid));

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(textview), buffer);

	g_object_unref(buffer);

	// BUTTONS
	cancel_edit_btn = GTK_BUTTON(gtk_grid_get_child_at(GTK_GRID(edition_grid), 1, 0));
	confirm_edit_btn = GTK_BUTTON(gtk_grid_get_child_at(GTK_GRID(edition_grid), 2, 0));

	g_signal_connect(cancel_edit_btn, "clicked", G_CALLBACK(on_cancel_edit_clicked), win);
	g_signal_connect(confirm_edit_btn, "clicked", G_CALLBACK(on_confirm_edit_clicked), win);


}

void on_delete_clicked(GtkButton *button, FileEditorWindow *win)
{
	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");
	GtkWidget *scrolled = gtk_widget_get_ancestor(GTK_WIDGET(visible_child), GTK_TYPE_SCROLLED_WINDOW);
	GtkListBox *listbox = GTK_LIST_BOX(gtk_widget_get_first_child(gtk_widget_get_first_child(scrolled)));

	if(!selected_line) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "No line selected");
		gtk_window_present(GTK_WINDOW(dialog));
		return;
	}

	LineNode * following_line_node = get_line_node_from_line(selected_line, file_struct)->next;

	GtkListBoxRow *selected_row = get_list_box_row_from_line_node(listbox, get_line_node_from_line(selected_line, file_struct));
	if(selected_row)
		remove_row_from_listbox(listbox, selected_row);

	local_remove_line(file_struct, selected_line);
	
	win->dirty = TRUE;

	// Select the following line
	if(following_line_node) {
		GtkListBoxRow *following_row = get_list_box_row_from_line_node(listbox, following_line_node);
		if(following_row) {
			gtk_list_box_select_row(listbox, following_row);
			gtk_widget_grab_focus(GTK_WIDGET(following_row));
		}
	}



	// if(following_line_node)
	// 	selected_line = file_struct->lines->line;

	// empty_listbox(listbox);
	// fill_listbox(listbox, file_struct);

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
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorWindow, main_mode_grid);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorWindow, edition_grid);
}

FileEditorWindow *file_editor_window_new(FileEditor *app)
{
	return g_object_new(FILE_EDITOR_WINDOW_TYPE, "application", app, NULL);
}

void file_editor_window_close(FileEditorWindow *win)
{
	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct;
	GtkListBox *listbox;

	while(visible_child) {
		file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");
		listbox = GTK_LIST_BOX(gtk_widget_get_first_child(gtk_widget_get_first_child(visible_child)));

		empty_listbox(listbox);
		local_close_file(file_struct);

		visible_child = gtk_widget_get_next_sibling(visible_child);
	}
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

	// FILEEDITORWINDOW
	g_signal_connect(win, "destroy", G_CALLBACK(file_editor_window_close), NULL);
	win->dirty = FALSE;

	// BUTTONS

	add_btn = GTK_BUTTON(gtk_grid_get_child_at(GTK_GRID(win->main_mode_grid), 1, 0));
	edit_btn = GTK_BUTTON(gtk_grid_get_child_at(GTK_GRID(win->main_mode_grid), 2, 0));
	delete_btn = GTK_BUTTON(gtk_grid_get_child_at(GTK_GRID(win->main_mode_grid), 3, 0));

	gtk_widget_set_visible(GTK_WIDGET(add_btn), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(edit_btn), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(delete_btn), TRUE);

	g_signal_connect(add_btn, "clicked", G_CALLBACK(on_add_clicked), win);
	g_signal_connect(edit_btn, "clicked", G_CALLBACK(on_edit_clicked), win);
	g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_clicked), win);

	// FILE

	filepath = g_file_get_path(file);

	File *file_struct = local_open_local_file(filepath);

	// LISTBOX STUFF

	scrolled = gtk_scrolled_window_new();
	gtk_widget_set_hexpand(scrolled, TRUE);
	gtk_widget_set_vexpand(scrolled, TRUE);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget *listbox = gtk_list_box_new();
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_SINGLE);
	g_signal_connect(listbox, "row-selected", G_CALLBACK(on_row_selected), win);
	
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), listbox);
	gtk_stack_add_titled(GTK_STACK(win->stack), scrolled, file_struct->filename, file_struct->filename);


	// associate file_struct with stack child

	fill_listbox(GTK_LIST_BOX(listbox), file_struct);
	g_object_set_data(G_OBJECT(scrolled), "file_struct", file_struct);
	
	g_free(filepath);
}
