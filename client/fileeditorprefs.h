#pragma once

#include <gtk/gtk.h>
#include "fileeditorwin.h"

#define FILE_EDITOR_PREFS_TYPE (file_editor_prefs_get_type())
G_DECLARE_FINAL_TYPE (FileEditorPrefs, file_editor_prefs, FILE, EDITOR_PREFS, GtkDialog)

FileEditorPrefs *file_editor_prefs_new(FileEditorWindow *win);