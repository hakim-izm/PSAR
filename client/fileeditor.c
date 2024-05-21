#include <gtk/gtk.h>

#include "api/client.h"
#include "fileeditor.h"
#include "fileeditorwin.h"
#include "fileeditorconnect.h"
#include "fileeditoropenexternal.h"


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

		file_editor_window_open(FILE_EDITOR_WINDOW(win), file, NULL);
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

static void open_external_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
	FileEditorOpenExternal *open_external;
	GtkWindow *win;

	win = gtk_application_get_active_window(GTK_APPLICATION(app));
	open_external = file_editor_open_external_new(FILE_EDITOR_WINDOW(win));
	gtk_window_present(GTK_WINDOW(open_external));

}

static void save_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
	FileEditorWindow *win;

	win = FILE_EDITOR_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(app)));

	GtkWidget *stack = win->stack;
	GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
	File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");

	save_file_call(file_struct, NULL, win);
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
		save_file_call(file_struct, path, file_editor_win);
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

static void quit_save_response (GtkDialog *dialog, int response, FileEditorWindow *win) {
	if(response == GTK_RESPONSE_YES) {
		GtkWidget *stack = win->stack;
		GtkWidget *visible_child = gtk_stack_get_visible_child(GTK_STACK(stack));
		File *file_struct = g_object_get_data(G_OBJECT(visible_child), "file_struct");

		save_file_call(file_struct, NULL, win);
	}
	gtk_window_close(GTK_WINDOW(win));
	gtk_window_destroy(GTK_WINDOW(dialog));
}

static void quit_activated (GSimpleAction *action, GVariant *parameter, gpointer app) {
	// Close all FileEditorWindow
	GList *windows;
	windows = gtk_application_get_windows(GTK_APPLICATION(app));
	FileEditorWindow *win = FILE_EDITOR_WINDOW(windows->data);
	while(windows) {
		if(win->dirty){
			GtkWidget *dialog;
			dialog = gtk_message_dialog_new(GTK_WINDOW(windows->data),
						       GTK_DIALOG_MODAL,
						       GTK_MESSAGE_QUESTION,
						       GTK_BUTTONS_YES_NO,
						       "Do you want to save the changes?");
			gtk_window_set_title(GTK_WINDOW(dialog), "Save Changes");
			gtk_window_present(GTK_WINDOW(dialog));

			g_signal_connect(dialog, "response", G_CALLBACK(quit_save_response), win);

			return;
		}

		gtk_window_close(GTK_WINDOW(windows->data));
		windows = windows->next;
	}

	// Déconnexion du serveur

	const char *server_ip =	g_settings_get_string(win->settings, "ip");
	if(deconnexion(server_ip)) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(win),
							GTK_DIALOG_MODAL,
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_OK,
							"Disconnection failed. The editor will be closed.");
		gtk_window_set_title(GTK_WINDOW(dialog), "Disconnection failed");
		gtk_window_present(GTK_WINDOW(dialog));

		g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_close), NULL);
	}

	printf("bye\n");

	g_application_quit (G_APPLICATION (app));
}

/*
 * ACTIONS ENTRIES
 */

static GActionEntry app_entries[] = {
	{"connect", connect_activated, NULL, NULL, NULL},
	{"open", open_activated, NULL, NULL, NULL},
	{"open_external", open_external_activated, NULL, NULL, NULL},
	{"save", save_activated, NULL, NULL, NULL},
	{"save_as", save_as_activated, NULL, NULL, NULL},
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
		file_editor_window_open(win, files[i], NULL);

	gtk_window_present(GTK_WINDOW(win));
}

static void file_editor_startup(GApplication *app) {
	const char *connect_accels[2] = {"<Ctrl>K", NULL};
	const char *open_accels[2] = {"<Ctrl>O", NULL};
	const char *open_external_accels[2] = {"<Ctrl>I", NULL};
	const char *save_accels[2] = {"<Ctrl>S", NULL};
	const char *save_as_accels[2] = {"<Ctrl><Alt>S", NULL};
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
							      "app.open_external",
							      open_external_accels);

	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
							      "app.save",
							      save_accels);
	
	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
							      "app.save_as",
							      save_as_accels);

	gtk_application_set_accels_for_action(GTK_APPLICATION(app),
							      "app.quit",
							      quit_accels);
	

	g_signal_connect(GTK_APPLICATION(app), "delete-event", G_CALLBACK(quit_activated), NULL);
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