#pragma once

#include <gtk/gtk.h>
#include "fileeditor.h"

#define FILE_EDITOR_WINDOW_TYPE (file_editor_window_get_type())
G_DECLARE_FINAL_TYPE (FileEditorWindow, file_editor_window, FILE, EDITOR_WINDOW, GtkApplicationWindow)

FileEditorWindow *file_editor_window_new (FileEditor *app);

void file_editor_window_open(FileEditorWindow *win, GFile *file);