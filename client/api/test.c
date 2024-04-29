#include <stdio.h>
#include "client.h"

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

	// Print file content
	LineNode * curr = file->lines;
	while(curr){
		printf("LINE N° %d:\t%s\n",curr->line->id ,curr->line->text);
		curr = curr->next;
	}

	return 0;
}