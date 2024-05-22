#include <gtk/gtk.h>
#include <ctype.h>
#include "fileeditor.h"
#include "fileeditorwin.h"
#include "fileeditorconnect.h"

struct _FileEditorConnect {
	GtkDialog parent;

	// GSettings *settings;
	GtkEntry *serverip;
	GtkButton *cancelBtn;
	GtkButton *okBtn;
};

G_DEFINE_TYPE (FileEditorConnect, file_editor_connect, GTK_TYPE_DIALOG)

/*
 * FUNCTIONS
 */
// Fonction pour vérifier la validité d'une adresse IP
static gboolean is_valid_ip(const char *ip) {
	if (ip == NULL) return FALSE;

	int num, dots = 0;
	char *ptr;
	char *ip_copy = g_strdup(ip);  // Créer une copie modifiable de l'adresse IP

	if (ip_copy == NULL) return FALSE;

	ptr = strtok(ip_copy, ".");
	if (ptr == NULL) {
		g_free(ip_copy);
		return FALSE;
	}

	while (ptr) {
		if (!isdigit(*ptr)) {
		g_free(ip_copy);
		return FALSE;
		}

		num = strtol(ptr, NULL, 10);
		if (num >= 0 && num <= 255) {
		ptr = strtok(NULL, ".");
		if (ptr != NULL) dots++;
		} else {
		g_free(ip_copy);
		return FALSE;
		}
	}

	g_free(ip_copy);
	return dots == 3;  // Une adresse IPv4 valide doit contenir 3 points
}

/*
 * CALLBACKS
 */

void connect_action(GtkButton *button, FileEditorConnect *connect) {
	
	const char *ip_addr = gtk_editable_get_text(GTK_EDITABLE(connect->serverip));

	printf("[DEBUG] IP ADDRESS: %s\n", ip_addr);

	if(!is_valid_ip(ip_addr)) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(connect),
							GTK_DIALOG_MODAL,
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_OK,
							"The IP address is invalid. Please enter a valid IP address.");
		gtk_window_set_title(GTK_WINDOW(dialog), "Invalid IP address");
		gtk_window_present(GTK_WINDOW(dialog));

		g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_close), NULL);
		
		return;
	}


	// Sauvegarde de l'adresse IP dans les paramètres
	// g_settings_set_string(connect->settings, "ip", ip_addr);
	FileEditorWindow *win = FILE_EDITOR_WINDOW(gtk_window_get_transient_for(GTK_WINDOW(connect)));
	g_settings_set_string(win->settings, "serverip", ip_addr);

	// DEBUG : affichage de l'adresse IP depuis les paramètres du parent
	const char *ip = g_settings_get_string(win->settings, "serverip");
	printf("[DEBUG] IP ADDRESS FROM PARENT: %s\n", ip);

	// récupération de l'adresse IP du client
	const char *client_ip = g_settings_get_string(win->settings, "clientip");
	

	// connexion au serveur
	if(connexion(ip_addr, client_ip)) {
		GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(connect),
							GTK_DIALOG_MODAL,
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_OK,
							"Connection failed. Please check the IP address and try again.");
		gtk_window_set_title(GTK_WINDOW(dialog), "Connection failed");
		gtk_window_present(GTK_WINDOW(dialog));

		g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_close), NULL);
		
		return;
	}

	gtk_window_close(GTK_WINDOW(connect));
}

/*
 * OVERRIDEN METHODS
 */

static void file_editor_connect_init (FileEditorConnect *connect) {
	gtk_widget_init_template(GTK_WIDGET(connect));

	g_signal_connect_swapped(connect->cancelBtn, "clicked", G_CALLBACK(gtk_window_close), GTK_WINDOW(connect));

	g_signal_connect(connect->okBtn, "clicked", G_CALLBACK(connect_action), connect);
}

static void file_editor_connect_dispose (GObject *object) {
	FileEditorConnect *connect;

	connect = FILE_EDITOR_CONNECT(object);

	// g_clear_object(&connect->settings);

	G_OBJECT_CLASS(file_editor_connect_parent_class)->dispose(object);

}

static void file_editor_connect_class_init(FileEditorConnectClass *class) {
	G_OBJECT_CLASS (class)->dispose = file_editor_connect_dispose;

	gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/com/psar/fileeditor/connect.ui");
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorConnect, serverip);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorConnect, cancelBtn);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), FileEditorConnect, okBtn);

	
}

FileEditorConnect * file_editor_connect_new(FileEditorWindow *win) {
	return g_object_new(FILE_EDITOR_CONNECT_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}