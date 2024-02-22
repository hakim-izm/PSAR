#include <gtk/gtk.h>

#include "fileeditor.h"

int main(int argc, char **argv){
        /*
         * Environment used to load gschema "com.psar.fileeditor.gschema.xml"
         * Doesn't work without setting the environment.
         */
        g_setenv ("GSETTINGS_SCHEMA_DIR", ".", FALSE);
        
	return g_application_run(G_APPLICATION(file_editor_new()), argc, argv);
}