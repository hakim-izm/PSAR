#pragma once

#include <gtk/gtk.h>
#include "fileeditor.h"
#include "api/client.h"

#define FILE_EDITOR_WINDOW_TYPE (file_editor_window_get_type())
G_DECLARE_FINAL_TYPE (FileEditorWindow, file_editor_window, FILE, EDITOR_WINDOW, GtkApplicationWindow)

struct _FileEditorWindow
{
	GtkApplicationWindow parent;

	GSettings *settings;
	GtkWidget *appmenu;
	GtkWidget *stack;
	GtkWidget *grid;
};

File * get_file_struct_from_filename(const char *filename);

LineNode * get_line_node_from_text(File * file_struct, char *text);

void save_file_call(File * file, const char * filepath);

FileEditorWindow *file_editor_window_new (FileEditor *app);

void file_editor_window_open(FileEditorWindow *win, GFile *file);