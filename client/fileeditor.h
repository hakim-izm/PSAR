#pragma once

#include <gtk/gtk.h>

#define FILE_EDITOR_TYPE (file_editor_get_type())
G_DECLARE_FINAL_TYPE (FileEditor, file_editor, FILE, EDITOR, GtkApplication)

struct _FileEditor {
	GtkApplication parent;
};

FileEditor *file_editor_new(void);