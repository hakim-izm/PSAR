#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

void add_line(LineNode *lines, Line *line, int mode) {
	LineNode *curr = lines;
	
	// si mode = ADD_APPEND, on ajoute la ligne à la fin du fichier
	// si mode = ADD_INSERT, on ajoute la ligne après la ligne courante
	while(curr->next && mode == ADD_APPEND) {
		curr = curr->next;
	}

	LineNode *new_node = (LineNode *)malloc(sizeof(LineNode));
	if(!new_node) {
		fprintf(stderr, "CLIENT API: malloc error for new line node (add_line())\n");
		return;
	}

	new_node->line = line;
	new_node->next = NULL;
	curr->next = new_node;

	// TODO : envoi
}

File * open_local_file(char *filepath) {
	FILE *fp = fopen(filepath, "r"); // read only
	
	if(!fp) {
		fprintf(stderr, "CLIENT API: failed opening file \"%s\" (open_local_file())\n", filepath);
		return NULL;
	}

	// Init structs : LineNode et File
	LineNode *lines = (LineNode *)malloc(sizeof(LineNode));
	
	lines->line = NULL;
	lines->next = NULL;

	File *file_struct = (File *)malloc(sizeof(File));

	file_struct->filename = (char *)malloc(MAX_FILENAME_LENGTH);	
	file_struct->host_client = NULL; // TODO : récupérer ID client
	file_struct->lines = lines;
	file_struct->line_count = 0;
	file_struct->clients = NULL; // TODO : changer ce NULL
	file_struct->client_count = 0; // TODO : changer ce 0

	strncpy(file_struct->filename, basename(filepath), MAX_FILENAME_LENGTH);

	Line *curr_line = NULL;

	// Parsing du fichier (ligne par ligne)
	char line_buffer[MAX_LINE_LENGTH];
	int line_id = 0;
	while(fgets(line_buffer, MAX_LINE_LENGTH, fp)) {
		// Suppression retour à la ligne le cas échéant (remplacement '\n' par '\0')
		char *newline = strchr(line_buffer, '\n');
		if(newline)
			*newline = '\0';
		
		// Malloc pour nouvelle ligne
		Line *new_line = (Line *)malloc(sizeof(Line));
		if(!new_line){ // échec malloc -> libération ressources allouées
			fprintf(stderr, "CLIENT API: malloc error for new line, file=\"%s\" (open_local_file())\n", filepath);
			fclose(fp);
			
			LineNode *curr = lines;
			while(curr) {
				LineNode *next = curr->next;
				free(curr);
				curr = next;
			}

			return NULL;
		}

		// Init de la nouvelle ligne
		// new_line->next = NULL;
		new_line->text = strdup(line_buffer); // allocation mémoire !
		new_line->id = line_id;

		// Insertion de la nouvelle ligne
		if(!curr_line) {
			lines->line = new_line;
		} else {
			add_line(lines, new_line, ADD_APPEND);
		}

		curr_line = new_line;
		line_id++;
		file_struct->line_count++;
	}

	fclose(fp);

	// TODO : Envoi au serveur (nom fichier + id client)

	return file_struct;
}