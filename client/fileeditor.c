#include <gtk/gtk.h>

#include "fileeditor.h"
#include "fileeditorwin.h"
#include "fileeditorprefs.h"

struct _FileEditor {
	GtkApplication parent;
};

G_DEFINE_TYPE(FileEditor, file_editor, GTK_TYPE_APPLICATION);

/*
 * ACTIONS CALLBACKS
*/

static void connect_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
}

static void open_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
}

static void save_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
}

static void preferences_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
	FileEditorPrefs *prefs;
	GtkWindow *win;

	win = gtk_application_get_active_window(GTK_APPLICATION(app));
	prefs = file_editor_prefs_new(FILE_EDITOR_WINDOW(win));
	gtk_window_present(GTK_WINDOW(prefs));
}

static void quit_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
	g_application_quit (G_APPLICATION (app));
}

/*
 * ACTIONS ENTRIES
*/

static GActionEntry app_entries[] = {
	{"connect", connect_activated, NULL, NULL, NULL},
	{"open", open_activated, NULL, NULL, NULL},
	{"save", save_activated, NULL, NULL, NULL},
	{"preferences", preferences_activated, NULL, NULL, NULL},
	{"quit", quit_activated, NULL, NULL, NULL}
};

/*
 * OVERRIDEN METHODS
*/

static void file_editor_init(FileEditor *app) {

}

static void file_editor_activate(GApplication *app) {
	FileEditorWindow *win;

	win = file_editor_window_new(FILE_EDITOR(app));
	gtk_window_present(GTK_WINDOW(win));
}

static void file_editor_open(GApplication *app, GFile **files, int n_files, const char *hint) {
	GList *windows;
	FileEditorWindow *win;

	windows = gtk_application_get_windows(GTK_APPLICATION(app));
	if(windows)
		win = FILE_EDITOR_WINDOW (windows->data);
	else
		win = file_editor_window_new(FILE_EDITOR(app));

	for(int i=0; i<n_files; ++i)
		file_editor_window_open(win, files[i]);

	gtk_window_present(GTK_WINDOW(win));
}

static void file_editor_startup(GApplication *app) {
	const char *connect_accels[2] = {"<Ctrl>K", NULL};
	const char *open_accels[2] = {"<Ctrl>O", NULL};
	const char *save_accels[2] = {"<Ctrl>S", NULL};
	const char *preferences_accels[2] = {"<Ctrl>I", NULL};
	const char *quit_accels[2] = {"<Ctrl>Q", NULL};

	G_APPLICATION_CLASS (file_editor_parent_class)->startup(app);

	g_action_map_add_action_entries(G_ACTION_MAP(app),
					app_entries, G_N_ELEMENTS(app_entries),
					app);

	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
							      "app.connect",
							      connect_accels);

	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
							      "app.open",
							      open_accels);

	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
							      "app.save",
							      save_accels);

	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
							      "app.preferences",
							      preferences_accels);

	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
							      "app.quit",
							      quit_accels);
}

static void file_editor_class_init(FileEditorClass *class) {
	G_APPLICATION_CLASS (class)->startup = file_editor_startup;
	G_APPLICATION_CLASS (class)->activate = file_editor_activate;
	G_APPLICATION_CLASS (class)->open = file_editor_open;
}

FileEditor * file_editor_new(void) {
	return g_object_new (FILE_EDITOR_TYPE,
			     "application-id", "com.psar.fileeditor",
			     "flags", G_APPLICATION_HANDLES_OPEN,
			     NULL);
}