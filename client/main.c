#include <gtk/gtk.h>

#include <pthread.h>
#include "fileeditor.h"
#include "api/client.h"

int main(int argc, char **argv){
        /*
         * Environment used to load gschema "com.psar.fileeditor.gschema.xml"
         * Doesn't work without setting the environment.
         */
        g_setenv ("GSETTINGS_SCHEMA_DIR", ".", FALSE);

	// initialisation de la souche client
	pthread_t thread;
	if (pthread_create(&thread, NULL, initialize_client, NULL) != 0) {
		perror("Thread creation failed");
		EXIT_FAILURE;
	}
        
	return g_application_run(G_APPLICATION(file_editor_new()), argc, argv);
}