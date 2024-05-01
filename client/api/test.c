#include <stdio.h>
#include "client.h"

void print_file_content(File * file) {
	LineNode * curr = file->lines;
	while(curr){
		printf("LINE N° %d:\t%s\n",curr->line->id ,curr->line->text);
		curr = curr->next;
	}
}

int main(int argc, char **argv){
	if(argc < 2){
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		return -1;
	}

	File * file = open_local_file(argv[1]);

	if(!file){
		fprintf(stderr, "Failed to open file: %s\n", argv[1]);
		return -1;
	}

	printf("TEST: PRINT FILE CONTENT\n");
	print_file_content(file);

	// Test delete line
	remove_line(file, file->lines->line);

	printf("\nTEST: PRINT FILE CONTENT AFTER DELETE\n");
	print_file_content(file);

	// Test add line
	add_line(file, file->lines, "Test add line 1", ADD_APPEND);

	printf("\nTEST: PRINT FILE CONTENT AFTER ADD APPEND\n");
	print_file_content(file);

	// Test add line
	add_line(file, file->lines->next, "Test add line 2", ADD_INSERT);

	printf("\nTEST: PRINT FILE CONTENT AFTER ADD INSERT\n");
	print_file_content(file);
	
	// Test add line at the beginning
	add_line(file, NULL, "Test add line 3", ADD_INSERT);

	printf("\nTEST: PRINT FILE CONTENT AFTER ADD INSERT\n");
	print_file_content(file);

	// Test edit line
	edit_line(file->lines->line, "Test edit line");

	printf("\nTEST: PRINT FILE CONTENT AFTER EDIT\n");
	print_file_content(file);


	// Close file
	close_file(file);

	return 0;
}