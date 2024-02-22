#include <gtk/gtk.h>

#include "fileeditor.h"
#include "fileeditorwin.h"
#include "fileeditorconnect.h"

struct _FileEditorConnect {
	GtkDialog parent;

	GSettings *settings;
	GtkEntry *ip;
	GtkEntry *port;
};

G_DEFINE_TYPE (FileEditorConnect, file_editor_connect, GTK_TYPE_DIALOG)

/*
 * OVERRIDEN METHODS
*/

static void file_editor_connect_init (FileEditorConnect *connect) {
	gtk_widget_init_template(GTK_WIDGET(connect));
	connect->settings = g_settings_new("com.psar.fileeditor");

	g_settings_bind(connect->settings, "ip",
			connect->ip, "text",
			G_SETTINGS_BIND_DEFAULT);

	g_settings_bind(connect->settings, "port",
			connect->port, "text",
			G_SETTINGS_BIND_DEFAULT);
}

static void file_editor_connect_dispose (GObject *object) {
	FileEditorConnect *connect;

        const char *ip_addr = gtk_editable_get_text(GTK_EDITABLE(GTK_ENTRY(connect->ip)));
        const char *port = gtk_editable_get_text(GTK_EDITABLE(GTK_ENTRY(connect->port)));

        printf("[DEBUG] IP ADDRESS: %s:%s\n", ip_addr, port);

	connect = FILE_EDITOR_CONNECT(object);

	g_clear_object(&connect->settings);

	G_OBJECT_CLASS(file_editor_connect_parent_class)->dispose(object);

}

static void file_editor_connect_class_init(FileEditorConnectClass *class) {
	G_OBJECT_CLASS (class)->dispose = file_editor_connect_dispose;

	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/com/psar/fileeditor/connect.ui");
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorConnect, ip);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorConnect, port);
}

FileEditorConnect * file_editor_connect_new(FileEditorWindow *win) {
	return g_object_new(FILE_EDITOR_CONNECT_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}