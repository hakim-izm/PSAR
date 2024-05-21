#pragma once

#include <gtk/gtk.h>
#include "fileeditorwin.h"

#define FILE_EDITOR_OPEN_EXTERNAL_TYPE (file_editor_open_external_get_type())
G_DECLARE_FINAL_TYPE (FileEditorOpenExternal, file_editor_open_external, FILE, EDITOR_OPEN_EXTERNAL, GtkDialog)

FileEditorOpenExternal *file_editor_open_external_new(FileEditorWindow *win);