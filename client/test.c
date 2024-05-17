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

int main() {
    pthread_t thread;
    if (pthread_create(&thread, NULL, initialize_client, NULL) != 0) {
        perror("Thread creation failed");
    }

    // Attente de la fin du thread
    pthread_join(thread, NULL);

}