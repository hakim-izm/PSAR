#include <gtk/gtk.h>

#include "fileeditor.h"
#include "fileeditorwin.h"

struct _FileEditorWindow {
	GtkApplicationWindow parent;

	GSettings *settings;
	GtkWidget *appmenu;
	GtkWidget *stack;
};

G_DEFINE_TYPE(FileEditorWindow, file_editor_window, GTK_TYPE_APPLICATION_WINDOW);

/*
 * OVERRIDEN METHODS
*/

static void file_editor_window_init(FileEditorWindow *win) {
	GtkBuilder *builder;
	GMenuModel *menu;

	gtk_widget_init_template(GTK_WIDGET(win));

	builder = gtk_builder_new_from_resource("/com/psar/fileeditor/menu.ui");
	menu = G_MENU_MODEL (gtk_builder_get_object(builder, "menu"));
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(win->appmenu), menu);
	g_object_unref(builder);

	win->settings = g_settings_new("com.psar.fileeditor");
}

static void file_editor_window_class_init(FileEditorWindowClass *class) {
	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS (class), "/com/psar/fileeditor/window.ui");
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorWindow, stack);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorWindow, appmenu);
}

FileEditorWindow * file_editor_window_new (FileEditor *app) {
	return g_object_new(FILE_EDITOR_WINDOW_TYPE, "application", app, NULL);
}

void file_editor_window_open(FileEditorWindow *win, GFile *file) {
	char *basename;
	GtkWidget *scrolled, *view;
	char *contents;
	gsize length;
	GtkTextBuffer *buffer;
	GtkTextTag *tag;
	GtkTextIter start_iter, end_iter;

	basename = g_file_get_basename(file);

	scrolled = gtk_scrolled_window_new();
	gtk_widget_set_hexpand(scrolled, TRUE);
	gtk_widget_set_vexpand(scrolled, TRUE);
	
	view = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), TRUE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), TRUE);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
	gtk_stack_add_titled(GTK_STACK (win->stack), scrolled, basename, basename);
		
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

	if(g_file_load_contents (file, NULL, &contents, &length, NULL, NULL)) {
		gtk_text_buffer_set_text(buffer, contents, length);
		g_free(contents);
	}

	tag = gtk_text_buffer_create_tag(buffer, NULL, NULL);
	g_settings_bind(win->settings, "font", tag, "font", G_SETTINGS_BIND_DEFAULT);

	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
	gtk_text_buffer_apply_tag(buffer, tag, &start_iter, &end_iter);

	g_free(basename);

}