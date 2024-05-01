#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

void add_line(File *file, LineNode *lines, const char *text, int mode) {
	// si mode = ADD_APPEND, on ajoute la ligne à la fin du fichier
	// si mode = ADD_INSERT, on ajoute la ligne après la ligne courante (curr)
	// si lines = NULL et mode = ADD_INSERT, on ajoute la ligne au début du fichier
	LineNode *curr = mode == ADD_INSERT ? lines : file->lines;
	while(mode == ADD_APPEND && curr->next) {
		curr = curr->next;
	}

	LineNode *new_node = (LineNode *)malloc(sizeof(LineNode));
	if(!new_node) {
		fprintf(stderr, "CLIENT API: malloc error for new line node (add_line())\n");
		return;
	}

	Line *line = (Line *)malloc(sizeof(Line));
	if(!line) {
		fprintf(stderr, "CLIENT API: malloc error for new line (add_line())\n");
		free(new_node);
		return;
	}

	line->text = strdup(text);
	line->id = file->line_count;
	file->line_count++;

	new_node->line = line;

	if(!curr) {
		LineNode *prev_first = file->lines;
		file->lines = new_node;
		new_node->next = prev_first;		
	} else if(mode == ADD_INSERT) {
		new_node->next = curr->next;
		curr->next = new_node;
	} else {
		new_node->next = NULL;
		curr->next = new_node;
	}

	// TODO : envoi

}

void remove_line(File *file, Line *line) {
	LineNode *curr = file->lines;
	LineNode *prev = NULL;


	while(curr) {
		if(curr->line == line) {
			if(prev) {
				prev->next = curr->next;
			} else {
				file->lines = curr->next;
			}

			free(curr->line->text);
			free(curr->line);
			free(curr);

			// TODO : envoi
			return;
		}

		prev = curr;
		curr = curr->next;
	}

	// ici : aucune ligne supprimée
}

void edit_line(Line *line, char *text) {
	free(line->text);
	line->text = strdup(text);

	// TODO : envoi
}

File * open_local_file(char *filepath) {
	FILE *fp = fopen(filepath, "r"); // read only
	
	if(!fp) {
		fprintf(stderr, "CLIENT API: failed opening file \"%s\" (open_local_file())\n", filepath);
		return NULL;
	}

	// Init structs : LineNode et File
	// LineNode *lines = (LineNode *)malloc(sizeof(LineNode));
	
	// lines->line = NULL;
	// lines->next = NULL;

	// LineNode * lines = NULL;

	File *file_struct = (File *)malloc(sizeof(File));

	file_struct->filename = (char *)malloc(MAX_FILENAME_LENGTH);	
	file_struct->host_client = NULL; // TODO : récupérer ID client
	file_struct->lines = NULL;
	file_struct->line_count = 0;
	file_struct->clients = NULL; // TODO : changer ce NULL
	file_struct->client_count = 0; // TODO : changer ce 0

	strncpy(file_struct->filename, basename(filepath), MAX_FILENAME_LENGTH);

	Line *curr_line = NULL;

	// Parsing du fichier (ligne par ligne)
	char line_buffer[MAX_LINE_LENGTH];
	// int line_id = 0;
	while(fgets(line_buffer, MAX_LINE_LENGTH, fp)) {
		// Suppression retour à la ligne le cas échéant (remplacement '\n' par '\0')
		char *newline = strchr(line_buffer, '\n');
		if(newline)
			*newline = '\0';

		printf("line_buffer: %s\n", line_buffer);
		
		// Malloc pour nouvelle ligne
		Line *new_line;
		// if(!new_line){ // échec malloc -> libération ressources allouées
		// 	fprintf(stderr, "CLIENT API: malloc error for new line, file=\"%s\" (open_local_file())\n", filepath);
		// 	fclose(fp);
			
		// 	LineNode *curr = lines;
		// 	while(curr) {
		// 		LineNode *next = curr->next;
		// 		free(curr);
		// 		curr = next;
		// 	}

		// 	return NULL;
		// }

		// Init de la nouvelle ligne
		// new_line->next = NULL;

		// Insertion de la nouvelle ligne
		if(!curr_line) {
			// new_line = (Line *)malloc(sizeof(Line));
			// new_line->text = strdup(line_buffer); // allocation mémoire !
			// new_line->id = file_struct->line_count;
			// lines->line = new_line;
			// file_struct->line_count++;
			add_line(file_struct, NULL, line_buffer, ADD_INSERT);
			new_line = file_struct->lines->line;
		} else {
			add_line(file_struct, file_struct->lines, line_buffer, ADD_APPEND);
			new_line = file_struct->lines->next->line;
		}

		curr_line = new_line;
		// line_id++;
		// file_struct->line_count++;
	}

	fclose(fp);

	// TODO : Envoi au serveur (nom fichier + id client)

	return file_struct;
}

void save_file(File * file, const char * filepath) {
	FILE *fp = fopen(filepath, "w"); // write only

	if(!fp) {
		fprintf(stderr, "CLIENT API: failed opening file \"%s\" (save_file())\n", filepath);
		return;
	}

	LineNode *curr = file->lines;
	while(curr) {
		fprintf(fp, "%s\n", curr->line->text);
		curr = curr->next;
	}

	fclose(fp);

	// TODO : envoi
}

void close_file(File * file) {
	LineNode *curr = file->lines;
	while(curr) {
		LineNode *next = curr->next;
		free(curr->line->text);
		free(curr->line);
		free(curr);
		curr = next;
	}

	free(file->filename);
	free(file);

	// TODO : envoi
}