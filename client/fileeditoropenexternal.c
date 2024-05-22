#include <gtk/gtk.h>
#include <ctype.h>
#include "fileeditor.h"
#include "fileeditorwin.h"
#include "fileeditoropenexternal.h"

struct _FileEditorOpenExternal {
	GtkDialog parent;

	GtkEntry *filename;
	GtkButton *cancelBtn;
	GtkButton *okBtn;
};

G_DEFINE_TYPE (FileEditorOpenExternal, file_editor_open_external, GTK_TYPE_DIALOG)

/*
 * FUNCTIONS
 */


/*
 * CALLBACKS
 */

void open_external_action(GtkButton *button, FileEditorOpenExternal *open_external) {
	FileEditorWindow *win = FILE_EDITOR_WINDOW(gtk_window_get_transient_for(GTK_WINDOW(open_external)));

	char *filename = (char *)gtk_editable_get_text(GTK_EDITABLE(open_external->filename));
	const char *server_ip = g_settings_get_string(win->settings, "serverip");

	File *file_struct = local_open_external_file(filename, server_ip);

	gtk_window_close(GTK_WINDOW(open_external));

	file_editor_window_open(FILE_EDITOR_WINDOW(win), NULL, file_struct);
}

/*
 * OVERRIDEN METHODS
 */

static void file_editor_open_external_init (FileEditorOpenExternal *open_external) {
	gtk_widget_init_template(GTK_WIDGET(open_external));

	g_signal_connect_swapped(open_external->cancelBtn, "clicked", G_CALLBACK(gtk_window_close), GTK_WINDOW(open_external));

	g_signal_connect(open_external->okBtn, "clicked", G_CALLBACK(open_external_action), open_external);
}

static void file_editor_open_external_dispose (GObject *object) {
	FileEditorOpenExternal *open_external;

	open_external = FILE_EDITOR_OPEN_EXTERNAL(object);

	G_OBJECT_CLASS(file_editor_open_external_parent_class)->dispose(object);

}

static void file_editor_open_external_class_init(FileEditorOpenExternalClass *class) {
	G_OBJECT_CLASS (class)->dispose = file_editor_open_external_dispose;

	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/com/psar/fileeditor/open_external.ui");
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorOpenExternal, filename);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorOpenExternal, cancelBtn);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorOpenExternal, okBtn);
}

FileEditorOpenExternal * file_editor_open_external_new(FileEditorWindow *win) {
	return g_object_new(FILE_EDITOR_OPEN_EXTERNAL_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}