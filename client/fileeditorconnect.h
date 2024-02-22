#pragma once

#include <gtk/gtk.h>
#include "fileeditorwin.h"

#define FILE_EDITOR_CONNECT_TYPE (file_editor_connect_get_type())
G_DECLARE_FINAL_TYPE (FileEditorConnect, file_editor_connect, FILE, EDITOR_CONNECT, GtkDialog)

FileEditorConnect *file_editor_connect_new(FileEditorWindow *win);