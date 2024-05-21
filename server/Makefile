all : server

server : server.c main.c
	gcc server.c main.c -g -Wall -o server -ljson-c
clean : 
	rm server