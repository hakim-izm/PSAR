#include <gtk/gtk.h>

#include "fileeditor.h"
#include "fileeditorwin.h"
#include "fileeditorprefs.h"
#include "fileeditorconnect.h"
#include "api/client.h"

struct _FileEditor {
	GtkApplication parent;
};

G_DEFINE_TYPE(FileEditor, file_editor, GTK_TYPE_APPLICATION);

/*
 * ACTIONS CALLBACKS
 */

static void connect_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
	FileEditorConnect *connect;
	GtkWindow *win;

	win = gtk_application_get_active_window(GTK_APPLICATION(app));
	connect = file_editor_connect_new(FILE_EDITOR_WINDOW(win));
	gtk_window_present(GTK_WINDOW(connect));
}

static void open_response (GtkDialog *dialog, int response) {
	if(response == GTK_RESPONSE_ACCEPT) {
		GFile *file;
		GtkFileChooser *chooser;
		GtkWindow *win;

		chooser = GTK_FILE_CHOOSER(dialog);
		file = gtk_file_chooser_get_file(chooser);
		win = gtk_window_get_transient_for(GTK_WINDOW(dialog));

		file_editor_window_open(FILE_EDITOR_WINDOW(win), file);
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}

static void open_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
	GtkWidget *chooser;
	GtkWindow *win;

	win = gtk_application_get_active_window(GTK_APPLICATION(app));

	chooser = gtk_file_chooser_dialog_new("Open File",
					     GTK_WINDOW(win),
					     GTK_FILE_CHOOSER_ACTION_OPEN,
					     "_Cancel", GTK_RESPONSE_CANCEL,
					     "_Open", GTK_RESPONSE_ACCEPT,
					     NULL);

	gtk_window_present(GTK_WINDOW(chooser));

	g_signal_connect(chooser, "response", G_CALLBACK(open_response), win);

}

static void save_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
	FileEditorWindow *win;

	win = FILE_EDITOR_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(app)));

	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file-struct");

	save_file_call(file_struct, NULL);
}

static void save_as_response (GtkDialog *dialog, int response) {
	if(response == GTK_RESPONSE_ACCEPT) {
		printf("save_as_response - response accept\n");
		GFile *file;
		GtkFileChooser *chooser;
		GtkWindow *win;

		chooser = GTK_FILE_CHOOSER(dialog);
		file = gtk_file_chooser_get_file(chooser);
		win = gtk_window_get_transient_for(GTK_WINDOW(dialog));

		FileEditorWindow *file_editor_win = FILE_EDITOR_WINDOW(win);
		GtkWidget *stack = file_editor_win->stack;
		GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
		File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");

		char *path = g_file_get_path(file);
		printf("before save_file_call\n");
		save_file_call(file_struct, path);
		printf("after save_file_call\n");
		g_free(path);
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}

static void save_as_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
	GtkFileChooser *chooser;

	GtkWindow *win;

	win = gtk_application_get_active_window(GTK_APPLICATION(app));

	chooser = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new("Save File As",
							       win,
							       GTK_FILE_CHOOSER_ACTION_SAVE,
							       "_Cancel", GTK_RESPONSE_CANCEL,
							       "_Save", GTK_RESPONSE_ACCEPT,
							       NULL));

	gtk_window_present(GTK_WINDOW(chooser));

	g_signal_connect(chooser, "response", G_CALLBACK(save_as_response), win);

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
	{"save_as", save_as_activated, NULL, NULL, NULL},
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
	const char *save_as_accels[2] = {"<Ctrl><Alt>S", NULL};
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
							      "app.save_as",
							      save_as_accels);

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