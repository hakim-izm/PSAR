#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <pthread.h>
#include "client.h"

#define PORT 8080
#define ip "localhost"
#define file "fichier.txt"

int main() {
    pthread_t thread;
    if (pthread_create(&thread, NULL, initialize_client, NULL) != 0) {
        perror("Thread creation failed");
    }

    srand( time( NULL ) );

    connexion(ip);
    open_local_file(ip, file);
    sleep(rand()%1000);
    lock_line(ip,file,0);
    sleep(rand()%1000);
    unlock_line(ip,file,0);
    sleep(rand()%1000);
    close_file(ip,file);
    deconnexion(ip);

    // Attente de la fin du thread
    pthread_join(thread, NULL);

}