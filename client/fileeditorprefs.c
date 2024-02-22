#include <gtk/gtk.h>

#include "fileeditor.h"
#include "fileeditorwin.h"
#include "fileeditorprefs.h"

struct _FileEditorPrefs {
	GtkDialog parent;

	GSettings *settings;
	GtkWidget *font;
};

G_DEFINE_TYPE (FileEditorPrefs, file_editor_prefs, GTK_TYPE_DIALOG)

/*
 * OVERRIDEN METHODS
 */

static void file_editor_prefs_init (FileEditorPrefs *prefs) {
	gtk_widget_init_template(GTK_WIDGET(prefs));
	prefs->settings = g_settings_new("com.psar.fileeditor");

	g_settings_bind(prefs->settings, "font",
			prefs->font, "font",
			G_SETTINGS_BIND_DEFAULT);
}

static void file_editor_prefs_dispose (GObject *object) {
	FileEditorPrefs *prefs;

	prefs = FILE_EDITOR_PREFS(object);

	g_clear_object(&prefs->settings);

	G_OBJECT_CLASS(file_editor_prefs_parent_class)->dispose(object);
}

static void file_editor_prefs_class_init(FileEditorPrefsClass *class) {
	G_OBJECT_CLASS (class)->dispose = file_editor_prefs_dispose;

	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/com/psar/fileeditor/prefs.ui");
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorPrefs, font);
}

FileEditorPrefs * file_editor_prefs_new(FileEditorWindow *win) {
	return g_object_new(FILE_EDITOR_PREFS_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}