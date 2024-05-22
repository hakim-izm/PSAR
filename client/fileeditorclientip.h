#pragma once

#include <gtk/gtk.h>
#include "fileeditorwin.h"

#define FILE_EDITOR_CLIENTIP_TYPE (file_editor_clientip_get_type())
G_DECLARE_FINAL_TYPE (FileEditorClientIP, file_editor_clientip, FILE, EDITOR_CLIENTIP, GtkDialog)

FileEditorClientIP *file_editor_clientip_new(FileEditorWindow *win);