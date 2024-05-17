#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <json-c/json.h>

#define SERVER_PORT 8080
#define CLIENT_PORT 1234
ClientSession *session;
Client *client;
int client_fd;  
const int UID = 123;
const char * IP_ADD = "localhost";

void *initialize_client() {
    
    // Initialisation de la session du client
    session = (ClientSession *)malloc(sizeof(ClientSession));
    if (session == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialisation de la liste des fichiers
    session->files = NULL;
    session->file_count = 0;

    // Création de la structure Client
    // Allocation de mémoire pour le client
    client = (Client *)malloc(sizeof(Client));
    if (client == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Attribution des valeurs
    client->uid = UID;
    client->port = CLIENT_PORT;
    client->ip_address = strdup(IP_ADD);
    if (client->ip_address == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }


    // Création de la socket client
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Création de la socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(CLIENT_PORT);

    /// Attachement de la socket au port 8080
    if (bind(client_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Mise en écoute de la socket
    if (listen(client_fd, 0) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Client initialized and listening on port %d\n", CLIENT_PORT);

    while (1) {
        printf("Waiting Connection ... \n");
        int new_socket;
        struct sockaddr_in new_address;
        int addrlen = sizeof(new_address);

        // Acceptation de la connexion entrante
        if ((new_socket = accept(client_fd, (struct sockaddr *)&new_address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        pthread_t request_thread;
        if (pthread_create(&request_thread, NULL, receive_request, &new_socket) != 0) {
            perror("Thread creation failed");
            close(new_socket);
            continue; // Continue vers la prochaine itération de la boucle
        }

        // Détachement du thread
        pthread_detach(request_thread);
    }

    // Fermeture de la socket client
    close(client_fd);
}

void *receive_request(void *args){
    int new_socket = *(int *)args;
    
    // Traitement de la requete ici...
    // Lecture des données envoyées 
    char buffer[1024]= {0};
    ssize_t bytes_read = read(new_socket, buffer , sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading data");
        close(new_socket);
        return NULL;
    }

    // Conversion de la chaîne de caractères JSON en objet JSON
    json_object *json_obj = json_tokener_parse(buffer);

    // Déclaration d'un pointeur pour stocker l'objet JSON correspondant à "number"
    json_object *number_obj = NULL;

    // Extrait l'objet JSON correspondant à la clé "number"
    if (json_object_object_get_ex(json_obj, "number", &number_obj)) {
        // Obtient la valeur entière à partir de l'objet JSON
        int number = json_object_get_int(number_obj);
        // Utilisation de la valeur de "number" dans votre traitement
        switch (number) {
            case 1:
                // Demande des informations du fichier distant
                printf("-> Demande des informations du fichier distant\n");
                ask_information_external_file(new_socket, json_obj);
                break;
            case 2:
                // Arrivé d'un nouveau client dans un fichier
                printf("-> Arrivé d'un nouveau client dans un fichier\n");
                new_client_file(new_socket, json_obj);
                break;
            case 3:
                // Autorisation de Verrouiller d'une ligne après Attente
                printf("Autorisation de Verrouiller d'une ligne après Attente\n");
                permission_lock_line(new_socket, json_obj);
                break;
            case 4:
                // Demande de fermeture d'un fichier distant
                printf("Demande de fermeture d'un fichier distant\n");
                ask_close_file(new_socket, json_obj);
                break;
            case 5:
                // Information de la ligne après Modification
                printf("-> Information de la ligne après Modification\n");
                information_modification_line(new_socket, json_obj);
                break;
            case 6:
                // Information de la ligne après l'ajout
                printf("-> Information de la ligne après l'ajout\n");
                information_add_line(new_socket, json_obj);
                break;
            case 7:
                // Information sur la ligne a supprimer
                printf("Information sur la ligne a supprimer\n");
                information_delete_line(new_socket, json_obj);
                break;
            default:
                // Code de requête invalide
                break;
        }
    }

    // Fermeture de la socket 
    close(new_socket);
    
}








//______________________________________________________________________________________________________________________________________________________
void connexion(const char *server_ip){
    // Création de la socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "number", json_object_new_int(1));
    json_object_object_add(json_obj, "uid", json_object_new_int(UID));
    json_object_object_add(json_obj, "ip_address", json_object_new_string(IP_ADD));
    json_object_object_add(json_obj, "port", json_object_new_int(CLIENT_PORT));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur
    char response[1024];    
    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    printf("Connection request response : %s\n",response);
    if(strcmp(response, "Success Connexion") == 0){
        // Connexion Réussie
    }else{
        // Connexion Echoué
    }
    
    // Fermeture de la socket
    close(client_socket);
}








//______________________________________________________________________________________________________________________________________________________
void deconnexion(const char *server_ip){
    // Création de la socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "number", json_object_new_int(2));
    json_object_object_add(json_obj, "uid", json_object_new_int(UID));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur
    char response[1024];    
    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    printf("Deconnexion request response : %s\n",response);
    if(strcmp(response, "Success Deconnexion") == 0){
        // Deconnexion Réussie
    }else{
        // Deconnexion Echoué
    }
    
    // Fermeture de la socket
    close(client_socket);

}







//______________________________________________________________________________________________________________________________________________________
void open_local_file(const char *server_ip, char *filename){
    // Création de la socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "number", json_object_new_int(3));
    json_object_object_add(json_obj, "uid", json_object_new_int(UID));
    json_object_object_add(json_obj, "filename", json_object_new_string(filename));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur
    char response[1024];    
    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    printf("Open Local File request response : %s\n",response);
    char success[100]; 
    strcpy(success, "Success Open Local File ");
    strcat(success, filename);
    if(strcmp(response, success) == 0){
        // Ouverture d'un fichier local Réussie

        // Création du nouveau fichier
        File *new_file = (File *)malloc(sizeof(File));
        if (new_file == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }

        // Attribution des valeurs
        new_file->filename = strdup(filename);
        if (new_file->filename == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        new_file->host_client = client;
        new_file->lines = NULL; // Liste des lignes vide
        new_file->line_count = 0;
        new_file->clients = NULL; // Liste des clients vide
        new_file->client_count = 0;

        // Ajout du fichier à la liste des fichiers dans ClientSession
        FileNode *new_file_node = (FileNode *)malloc(sizeof(FileNode));
        if (new_file_node == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        new_file_node->file = new_file;

        // Si la liste des fichiers n'est pas vide, ajoutez le nouveau fichier en tête de liste
        if (session->files != NULL) {
            new_file_node->next = session->files;
        }

        session->files = new_file_node;
        session->file_count++;

        printf("J'ai crée un nouveau fichier. Il y a %d fichiers dans ma liste\n", session->file_count);

    }else{
        // Ouverture d'un fichier local Echoué
    }
    
    // Fermeture de la socket
    close(client_socket);
}







//______________________________________________________________________________________________________________________________________________________
void open_external_file(const char *server_ip, char *filename){
    // Création de la socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "number", json_object_new_int(4));
    json_object_object_add(json_obj, "filename", json_object_new_string(filename));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur
    // Lecture des données envoyées par le serveur
    char buffer[1024]= {0};
    ssize_t bytes_read = read(client_socket, buffer , sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading data from client");
        close(client_socket);
    }
    
    // Fermeture de la socket
    close(client_socket);

    // Conversion de la chaîne de caractères JSON en objet JSON
    json_obj = json_tokener_parse(buffer);

    // Extraire la reponse
    char *reponse;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *reponse_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(json_obj, "response", &reponse_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (reponse_obj != NULL) {
        reponse = strdup(json_object_get_string(reponse_obj)); 
    }

    printf("Open External File request response : %s\n",reponse);
    char success[100]; 
    strcpy(success, "Success Open External File ");
    strcat(success, filename);
    if(strcmp(reponse, success) == 0){
        //Ouverture d'un fichier external Réussie
        
        // Extraire la reponse
        int host_id;
        char *host_ip;
        int host_port;

        // Initialisation de la variable pour stocker la valeur extraite
        json_object *id_obj = NULL;
        json_object *ip_obj = NULL;
        json_object *port_obj = NULL;

        // Lecture des données du client depuis le token
        json_object_object_get_ex(json_obj, "host_client_id", &id_obj);
        json_object_object_get_ex(json_obj, "host_client_ip_address", &ip_obj);
        json_object_object_get_ex(json_obj, "host_client_port", &port_obj);

        // Vérification de la présence des clés et extraction des valeurs
        if (id_obj != NULL && ip_obj != NULL && port_obj != NULL) {
            host_id = json_object_get_int(id_obj);
            host_ip = strdup(json_object_get_string(ip_obj)); 
            host_port = json_object_get_int(port_obj);
        }

        printf("HOST : id = %d | ip = %s | port = %d\n",host_id, host_ip, host_port);

        // Création de la socket
        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Configuration de la structure sockaddr_in
        struct sockaddr_in host_address;
        host_address.sin_family = AF_INET;
        host_address.sin_port = htons(host_port);

        // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
        if (inet_pton(AF_INET, host_ip, &host_address.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }

        // Établissement de la connexion avec le serveur
        if (connect(client_socket, (struct sockaddr *)&host_address, sizeof(host_address)) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        // Création d'un objet JSON
        json_object *json_obj_host = json_object_new_object();
        json_object_object_add(json_obj_host, "number", json_object_new_int(1));
        json_object_object_add(json_obj_host, "filename", json_object_new_string(filename));
        json_object_object_add(json_obj_host, "uid", json_object_new_int(UID));
        json_object_object_add(json_obj_host, "ip_address", json_object_new_string(IP_ADD));
        json_object_object_add(json_obj_host, "port", json_object_new_int(CLIENT_PORT));

        // Conversion de l'objet JSON en chaîne de caractères
        const char *json_str_host = json_object_to_json_string(json_obj_host);

        // Envoi des informations du client au host
        if (write(client_socket, json_str_host, strlen(json_str_host)) == -1) {
            perror("Error sending client information to host");
            exit(EXIT_FAILURE);
        }

        // Attente de la réponse du host
        // Lecture des données envoyées par le host
        char buffer_host[1024]= {0};
        ssize_t bytes_read_host = read(client_socket, buffer_host , sizeof(buffer_host));
        if (bytes_read_host < 0) {
            perror("Error reading data from client");
            close(client_socket);
        }
        
        // Fermeture de la socket
        close(client_socket);

        json_obj_host = json_tokener_parse(buffer_host);
        char *reponse_host;
        json_object *reponse_host_obj = NULL;
        json_object_object_get_ex(json_obj_host, "response", &reponse_host_obj);
        if (reponse_host_obj != NULL) {
            reponse_host = strdup(json_object_get_string(reponse_host_obj)); 
        }

        printf("Open External File request response from host : %s\n",reponse);
        char success_host[100]; 
        strcpy(success_host, "Success Open External File from host ");
        strcat(success_host, filename);
        if(strcmp(reponse_host, success_host) == 0){
            //Ouverture d'un fichier external depuis le host Réussie

            // Création de la structure Client
            // Allocation de mémoire pour le client
            Client *client_host = (Client *)malloc(sizeof(Client));
            if (client_host == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }

            // Attribution des valeurs
            client_host->uid = host_id;
            client_host->port = host_port;
            client_host->ip_address = strdup(host_ip);
            if (client_host->ip_address == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }

            // Création du nouveau fichier
            File *new_file = (File *)malloc(sizeof(File));
            if (new_file == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }

            // Attribution des valeurs
            new_file->filename = strdup(filename);
            if (new_file->filename == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            new_file->host_client = client_host;
            new_file->lines = NULL; // Liste des lignes vide
            new_file->line_count = 0;
            new_file->clients = NULL; // Liste des clients vide
            new_file->client_count = 0;

            // Liste des Clients
            int taille_client;
            json_object *taille_client_obj = NULL;
            json_object_object_get_ex(json_obj_host, "taille_client", &taille_client_obj);
            if (taille_client_obj != NULL) {
                taille_client = json_object_get_int(taille_client_obj); 
            }

            new_file->client_count=taille_client;

            for(int i=0; i<taille_client; i++){
                int client_id;
                char *client_ip;
                int client_port;
                json_object *client_id_obj = NULL;
                json_object *client_ip_obj = NULL;
                json_object *client_port_obj = NULL;
                char key_id[20];
                char key_ip[20];
                char key_port[20];
                snprintf(key_id, sizeof(key_id), "client_id_%d", i);
                snprintf(key_ip, sizeof(key_ip), "client_ip_%d", i);
                snprintf(key_port, sizeof(key_port), "client_port_%d", i);

                json_object_object_get_ex(json_obj_host, key_id, &client_id_obj);
                json_object_object_get_ex(json_obj_host, key_ip, &client_ip_obj);
                json_object_object_get_ex(json_obj_host, key_port, &client_port_obj);
                if (client_id_obj != NULL && client_ip_obj != NULL && client_port_obj != NULL) {
                    client_id = json_object_get_int(client_id_obj); 
                    client_ip = strdup(json_object_get_string(client_ip_obj)); 
                    client_port = json_object_get_int(client_port_obj); 
                }

                Client *new_client = (Client *)malloc(sizeof(Client));
                if (new_client == NULL) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }

                // Attribution des valeurs
                new_client->uid = client_id;
                new_client->port = client_port;
                new_client->ip_address = strdup(client_ip);

                // Ajout du client à la liste des clients du fichier
                ClientNode *new_client_node = (ClientNode *)malloc(sizeof(ClientNode));
                if (new_client_node == NULL) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }

                new_client_node->client = new_client;

                if (new_file->clients != NULL) {
                    new_client_node->next = new_file->clients;
                }
                new_file->clients = new_client_node;
            }

            // Liste des lignes
            int taille_line;
            json_object *taille_line_obj = NULL;
            json_object_object_get_ex(json_obj_host, "taille_line", &taille_line_obj);
            if (taille_line_obj != NULL) {
                taille_line = json_object_get_int(taille_line_obj); 
            }

            new_file->line_count=taille_line;

            for(int i=0; i<taille_line; i++){
                int line_id;
                char *line_text;
                json_object *line_id_obj = NULL;
                json_object *line_text_obj = NULL;
                char key_line_id[20];
                char key_line_text[20];
                snprintf(key_line_id, sizeof(key_line_id), "line_id_%d", i);
                snprintf(key_line_text, sizeof(key_line_text), "line_text_%d", i);

                json_object_object_get_ex(json_obj_host, key_line_id, &line_id_obj);
                json_object_object_get_ex(json_obj_host, key_line_text, &line_text_obj);
                if (line_text_obj != NULL && line_id_obj != NULL) {
                    line_id = json_object_get_int(line_id_obj); 
                    line_text = strdup(json_object_get_string(line_text_obj)); 
                }

                Line *new_line = (Line *)malloc(sizeof(Line));
                if (new_line == NULL) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }

                // Attribution des valeurs
                new_line->id = line_id;
                new_line->text = strdup(line_text);

                // Ajout du client à la liste des clients du fichier
                LineNode *new_line_node = (LineNode *)malloc(sizeof(LineNode));
                if (new_line_node == NULL) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }

                new_line_node->line = new_line;

                if (new_file->lines != NULL) {
                    new_line_node->next = new_file->lines;
                }
                new_file->lines = new_line_node;
            }

            // Ajout du fichier à la liste des fichiers dans ClientSession
            FileNode *new_file_node = (FileNode *)malloc(sizeof(FileNode));
            if (new_file_node == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            new_file_node->file = new_file;

            // Si la liste des fichiers n'est pas vide, ajoutez le nouveau fichier en tête de liste
            if (session->files != NULL) {
                new_file_node->next = session->files;
            }

            session->files = new_file_node;
            session->file_count++;

            // Création d'un objet JSON
            json_object *json_obj_clients = json_object_new_object();
            json_object_object_add(json_obj_clients, "number", json_object_new_int(2));
            json_object_object_add(json_obj_clients, "uid", json_object_new_int(UID));
            json_object_object_add(json_obj_host, "ip_address", json_object_new_string(IP_ADD));
            json_object_object_add(json_obj_host, "port", json_object_new_int(CLIENT_PORT));
            json_object_object_add(json_obj_clients, "filename", json_object_new_string(filename));
            

            // Envoi d'un message de fermeture de fichier à tous les clients
            ClientNode *current_client_node = new_file->clients;
            while (current_client_node != NULL) {

                // Allocation et initialisation de la structure des arguments
                struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
                args->obj = json_obj_clients;
                args->client = current_client_node->client;
                args->file = new_file;
                args->num = 6;

                pthread_t client_thread;
                if (pthread_create(&client_thread, NULL, send_all,args) != 0) {
                    perror("Thread creation failed");
                }
                current_client_node = current_client_node->next;
            }

            printf("Ajout d'un nouveau fichier. Il y a %d fichiers dans ma liste\n", session->file_count);

        }else{
            //Ouverture d'un fichier external depuis le host Echoué
        }

    }else{
        // Ouverture d'un fichier distant Echoué
    }
}








//______________________________________________________________________________________________________________________________________________________
void close_file(const char *server_ip, char *filename){
    // Création de la socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "number", json_object_new_int(5));
    json_object_object_add(json_obj, "uid", json_object_new_int(UID));
    json_object_object_add(json_obj, "filename", json_object_new_string(filename));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur
    char response[1024];    
    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    printf("Close File request response : %s\n",response);
    char success[100]; 
    strcpy(success, "Success Close File ");
    strcat(success, filename);
    if(strcmp(response, success) == 0){
        // Fermeture d'un fichier local Réussie

        // Recherche du fichier dans la liste des fichiers
        FileNode *current_file_node = session->files;
        while (current_file_node != NULL) {
            if (strcmp(current_file_node->file->filename, filename) == 0) {
                // Envoi d'un message de fermeture de fichier à tous les clients
                ClientNode *current_client_node = current_file_node->file->clients;
                while (current_client_node != NULL) {
                    // Création d'un objet JSON
                    json_object *json_obj_clients = json_object_new_object();
                    json_object_object_add(json_obj_clients, "number", json_object_new_int(4));
                    json_object_object_add(json_obj_clients, "uid", json_object_new_int(UID));
                    json_object_object_add(json_obj_clients, "filename", json_object_new_string(filename));

                    // Allocation et initialisation de la structure des arguments
                    struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
                    args->obj = json_obj_clients;
                    args->client = current_client_node->client;
                    args->file = current_file_node->file;
                    args->num = 1;

                    pthread_t client_thread;
                    if (pthread_create(&client_thread, NULL, send_all,args) != 0) {
                        perror("Thread creation failed");
                    }
                    current_client_node = current_client_node->next;
                }

                while(current_file_node->file->client_count>0){
                    // Attendre que client_count == 0
                    sleep(1000);
                }

                if(current_file_node->file->client_count == 0){
                    // Suppression de tous les clients
                    ClientNode *temp_client_node;
                    while (current_file_node->file->clients != NULL) {
                        temp_client_node = current_file_node->file->clients;
                        current_file_node->file->clients = current_file_node->file->clients->next;
                        free(temp_client_node->client->ip_address);
                        free(temp_client_node->client);
                        free(temp_client_node);
                    } 

                    
                    // Suppression du fichier
                    if (current_file_node->file == session->files->file) {
                        // Si le fichier est le premier dans la liste
                        session->files = session->files->next;
                        free(current_file_node->file->filename);
                        free(current_file_node->file);
                    } else {
                        // Si le fichier est ailleurs dans la liste
                        FileNode *temp_file_node = session->files;
                        while (temp_file_node->next->file != current_file_node->file) {
                            temp_file_node = temp_file_node->next;
                        }
                        temp_file_node->next = current_file_node->next;
                        free(current_file_node->file->filename);
                        free(current_file_node->file);
                    }
                }

                break;
            }
            current_file_node = current_file_node->next;
        }
    }else{
        // Fermeture d'un fichier Echoué
    }
    
    printf("Suppression du fichier %s Réussi !!!\n", filename);
    // Fermeture de la socket
    close(client_socket);
}








//______________________________________________________________________________________________________________________________________________________
void lock_line(const char *server_ip, char *filename, int line_id){
    // Création de la socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "number", json_object_new_int(6));
    json_object_object_add(json_obj, "client_id", json_object_new_int(UID));
    json_object_object_add(json_obj, "line_id", json_object_new_int(line_id));
    json_object_object_add(json_obj, "filename", json_object_new_string(filename));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur
    char response[1024];    
    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    printf("Lock Line request response : %s\n",response);

    char line_id_str[20];
    snprintf(line_id_str, sizeof(line_id_str), "%d", line_id);

    char success[100]; 
    strcpy(success, "Success Lock Line ");
    strcat(success, filename);
    strcat(success, " : ");
    strcat(success, line_id_str);

    char waiting[100]; 
    strcpy(waiting, "Waiting ... ");
    strcat(waiting, filename);
    strcat(waiting, " : ");
    strcat(waiting, line_id_str);

    if(strcmp(response, success) == 0){
        // Verrouillage de la ligne Réussie
    }else if(strcmp(response, waiting) == 0){
        // En Attente du Verrouillage ...
    }else{
        // Verrouillage de la ligne Echoué
    }
    
    // Fermeture de la socket
    close(client_socket);
}






//______________________________________________________________________________________________________________________________________________________
void unlock_line(const char *server_ip, char *filename, int line_id){
    // Création de la socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "number", json_object_new_int(7));
    json_object_object_add(json_obj, "client_id", json_object_new_int(UID));
    json_object_object_add(json_obj, "line_id", json_object_new_int(line_id));
    json_object_object_add(json_obj, "filename", json_object_new_string(filename));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur
    char response[1024];    
    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    printf("Lock Line request response : %s\n",response);

    char line_id_str[20];
    snprintf(line_id_str, sizeof(line_id_str), "%d", line_id);

    char success[100]; 
    strcpy(success, "Success Unlock Line ");
    strcat(success, filename);
    strcat(success, " : ");
    strcat(success, line_id_str);

    if(strcmp(response, success) == 0){
        // Deverrouillage de la ligne Réussie
    }
    
    // Fermeture de la socket
    close(client_socket);
}







//______________________________________________________________________________________________________________________________________________________
void add_line(char *filename, char *text, int line_before_id){ 
    // Recherche du fichier dans la liste des fichiers de la session
    FileNode *current_file_node = session->files;
    File *target_file = NULL;
    while (current_file_node != NULL) {
        if (strcmp(current_file_node->file->filename, filename) == 0) {
            target_file = current_file_node->file;
            break;
        }
        current_file_node = current_file_node->next;
    }

    if (target_file == NULL) {
        perror("Le fichier n'a pas été trouvé.\n");
        return;
    }

    // Récupération des informations sur le host
    Client *host_client = target_file->host_client;
    int host_port = host_client->port;
    char *host_ip = host_client->ip_address;

    // Création d'un objet JSON pour envoyer les informations
    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "number", json_object_new_int(6));
    json_object_object_add(json_obj, "filename", json_object_new_string(filename));
    json_object_object_add(json_obj, "line_before_id", json_object_new_int(line_before_id));
    json_object_object_add(json_obj, "text", json_object_new_string(text));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Création de la socket client
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in pour le serveur
    struct sockaddr_in host_address;
    host_address.sin_family = AF_INET;
    host_address.sin_port = htons(host_port);
    if (inet_pton(AF_INET, host_ip, &host_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&host_address, sizeof(host_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Envoi des informations au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending JSON object to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur
    // Lecture des données envoyées par le serveur
    char buffer[1024]= {0};
    ssize_t bytes_read = read(client_socket, buffer , sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading data from client");
        close(client_socket);
    }
    
    // Fermeture de la socket
    close(client_socket);

    // Récuperation de la réponse
    json_obj = json_tokener_parse(buffer);
    char *reponse;
    json_object *reponse_obj = NULL;
    json_object_object_get_ex(json_obj, "response", &reponse_obj);
    if (reponse_obj != NULL) {
        reponse = strdup(json_object_get_string(reponse_obj)); 
    }

    printf("Add Line request response : %s\n",reponse);
    char success[100]; 
    strcpy(success, "Success Add Line ");
    strcat(success, filename);
    if(strcmp(reponse, success) == 0){
        // Ajout d'une ligne Réussie

        // Récupération de l'id de la ligne
        int line_id;
        json_object *line_id_obj = NULL;
        json_object_object_get_ex(json_obj, "line_id", &line_id_obj);
        if (line_id_obj != NULL) {
            line_id = json_object_get_int(line_id_obj); 
        }

        
        // Création d'un objet JSON
        json_object *json_obj_clients = json_object_new_object();
        json_object_object_add(json_obj_clients, "number", json_object_new_int(6));
        json_object_object_add(json_obj_clients, "filename", json_object_new_string(filename));
        json_object_object_add(json_obj_clients, "line_before_id", json_object_new_int(line_before_id));
        json_object_object_add(json_obj_clients, "line_id", json_object_new_int(line_id));
        json_object_object_add(json_obj_clients, "text", json_object_new_string(text));

        // Envoi d'un message de fermeture de fichier à tous les clients
        ClientNode *current_client_node = target_file->clients;
        while (current_client_node != NULL) {

            // Allocation et initialisation de la structure des arguments
            struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
            args->obj = json_obj_clients;
            args->client = current_client_node->client;
            args->file = target_file;
            args->num = 2;

            pthread_t client_thread;
            if (pthread_create(&client_thread, NULL, send_all,args) != 0) {
                perror("Thread creation failed");
            }
            current_client_node = current_client_node->next;
        }

        // Création de la ligne
        Line *new_line = (Line *)malloc(sizeof(Line));
        if (new_line == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }

        // Attribution des valeurs
        new_line->id = line_id;
        new_line->text = strdup(text);

        // Ajout de la ligne a la liste lines dans le fichier
        LineNode *new_line_node = (LineNode *)malloc(sizeof(LineNode));
        if (new_line_node == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        new_line_node->line = new_line;
        new_line_node->next = NULL;

        LineNode *current_line_node = target_file->lines;
        while (current_line_node->next != NULL && current_line_node->line->id != line_before_id) {
            current_line_node = current_line_node->next;
        }

        // Insérez la nouvelle ligne après la ligne précédente
        new_line_node->next = current_line_node->next;
        current_line_node->next = new_line_node;
        target_file->line_count++;

        printf("La ligne avec l'ID %d a été ajouter dans le fichier %s. Il y a %d de ligne dans le fichier\n",line_id, filename, target_file->line_count);

    }else{
        // Ajout d'une ligne Echoué
    }
}







//______________________________________________________________________________________________________________________________________________________
void delete_line(const char *server_ip, char *filename, int line_id){
    // Création de la socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "number", json_object_new_int(8));
    json_object_object_add(json_obj, "line_id", json_object_new_int(line_id));
    json_object_object_add(json_obj, "filename", json_object_new_string(filename));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur
    char response[1024];    
    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }
    
    // Fermeture de la socket
    close(client_socket);

    printf("Delete Line request response : %s\n",response);
    char success[100]; 
    strcpy(success, "Success Delete Line ");
    strcat(success, filename);
    if(strcmp(response, success) == 0){
        // Suppression d'une ligne Réussie

        // Recherche du fichier dans la liste des fichiers de la session
        FileNode *current_file_node = session->files;
        File *target_file = NULL;
        while (current_file_node != NULL) {
            if (strcmp(current_file_node->file->filename, filename) == 0) {
                target_file = current_file_node->file;
                break;
            }
            current_file_node = current_file_node->next;
        }

        if (target_file == NULL) {
            perror("Le fichier n'a pas été trouvé.\n");
            return;
        }

        // Récupération des informations sur le host
        Client *host_client = target_file->host_client;
        int host_port = host_client->port;
        char *host_ip = host_client->ip_address;
        int host_id = host_client->uid;

        // Suppression de la ligne
        LineNode *current_line_node = target_file->lines;
        LineNode *prev_line_node = NULL;

        // Recherche de la ligne à supprimer
        while (current_line_node != NULL && current_line_node->line->id != line_id) {
            prev_line_node = current_line_node;
            current_line_node = current_line_node->next;
        }

        // Vérification si la ligne a été trouvée
        if (current_line_node == NULL) {
            printf("La ligne avec l'ID %d n'a pas été trouvée dans le fichier %s.\n", line_id, filename);
            return;
        }

        // La ligne à supprimer a été trouvée
        if (prev_line_node == NULL) {   
            // Si la ligne à supprimer est la première de la liste  
            current_line_node->line->text="";
        } else {
            // Sinon, relier la ligne précédente à la suivante, en sautant la ligne à supprimer
            prev_line_node->next = current_line_node->next;
        }

        
        // Création d'un objet JSON
        json_object *json_obj_clients = json_object_new_object();
        json_object_object_add(json_obj_clients, "number", json_object_new_int(7));
        json_object_object_add(json_obj_clients, "filename", json_object_new_string(filename));
        json_object_object_add(json_obj_clients, "line_id", json_object_new_int(line_id));

        // Envoi d'un message de fermeture de fichier à tous les clients
        ClientNode *current_client_node = target_file->clients;
        while (current_client_node != NULL) {

            // Allocation et initialisation de la structure des arguments
            struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
            args->obj = json_obj_clients;
            args->client = current_client_node->client;
            args->file = target_file;
            args->num = 3;

            pthread_t client_thread;
            if (pthread_create(&client_thread, NULL, send_all,args) != 0) {
                perror("Thread creation failed");
            }
            current_client_node = current_client_node->next;
        }

        if(UID != host_id){
            // Allocation et initialisation de la structure des arguments
            struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
            args->obj = json_obj_clients;
            args->client = host_client;
            args->file = target_file;
            args->num = 3;

            pthread_t client_thread;
            if (pthread_create(&client_thread, NULL, send_all,args) != 0) {
                perror("Thread creation failed");
            }
        }

        target_file->line_count--;
        printf("La ligne avec l'ID %d a été supprimer dans le fichier %s. Il y a %d de ligne dans le fichier\n",line_id, filename, target_file->line_count);
    }else{
        // Suppression d'une ligne Echoué
    }
}










//______________________________________________________________________________________________________________________________________________________
void modify_line(char *filename, char *text, int line_id){
    // Recherche du fichier dans la liste des fichiers de la session
    FileNode *current_file_node = session->files;
    File *target_file = NULL;
    while (current_file_node != NULL) {
        if (strcmp(current_file_node->file->filename, filename) == 0) {
            target_file = current_file_node->file;
            break;
        }
        current_file_node = current_file_node->next;
    }

    if (target_file == NULL) {
        perror("Le fichier n'a pas été trouvé.\n");
        return;
    }

    // Récupération des informations sur le host
    Client *host_client = target_file->host_client;
    int host_port = host_client->port;
    char *host_ip = host_client->ip_address;
    int host_id = host_client->uid;

    // Suppression de la ligne
    LineNode *current_line_node = target_file->lines;
    LineNode *prev_line_node = NULL;

    // Recherche de la ligne à supprimer
    while (current_line_node != NULL && current_line_node->line->id != line_id) {
        prev_line_node = current_line_node;
        current_line_node = current_line_node->next;
    }

    // Vérification si la ligne a été trouvée
    if (current_line_node == NULL) {
        printf("La ligne avec l'ID %d n'a pas été trouvée dans le fichier %s.\n", line_id, filename);
        return;
    }

    current_line_node->line->text=text;

    
    // Création d'un objet JSON
    json_object *json_obj_clients = json_object_new_object();
    json_object_object_add(json_obj_clients, "number", json_object_new_int(5));
    json_object_object_add(json_obj_clients, "filename", json_object_new_string(filename));
    json_object_object_add(json_obj_clients, "line_id", json_object_new_int(line_id));
    json_object_object_add(json_obj_clients, "text", json_object_new_string(text));

    // Envoi d'un message de fermeture de fichier à tous les clients
    ClientNode *current_client_node = target_file->clients;
    while (current_client_node != NULL) {

        // Allocation et initialisation de la structure des arguments
        struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
        args->obj = json_obj_clients;
        args->client = current_client_node->client;
        args->file = target_file;
        args->num = 3;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, send_all,args) != 0) {
            perror("Thread creation failed");
        }
        current_client_node = current_client_node->next;
    }

    if(UID != host_id){
        // Allocation et initialisation de la structure des arguments
        struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
        args->obj = json_obj_clients;
        args->client = host_client;
        args->file = target_file;
        args->num = 4;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, send_all,args) != 0) {
            perror("Thread creation failed");
        }
    }

    printf("La ligne avec l'ID %d a été modifié dans le fichier %s\n", line_id, filename);
}






//______________________________________________________________________________________________________________________________________________________
void *send_all(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    // Extraire les informations des arguments
    json_object *object = args->obj;
    Client *client = args->client;
    File *file = args->file;
    int num = args->num;

    // Création de la socket
    int client_socket;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(client->port);

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, client->ip_address, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    
    // Convertit l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(object);

    // Envoi de l'objet JSON au client
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending JSON object to client");
        exit(EXIT_FAILURE);
    }

    // Attendre la réponse du client
    char response[1024];
    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from client");
        exit(EXIT_FAILURE);
    }

    printf("Response from client: %s\n", response);
    char success[100]; 
    strcpy(success, "Success Close File ");
    strcat(success, file->filename);
    if(strcmp(response, success) == 0){
         // Traitement de la réponse en fonction de la valeur de num
        switch (num) {
            case 1:
                // Si num est égal à 1, décrémenter client_count du fichier
                if (file != NULL) {
                    file->client_count--;
                }
                break;
            default:
                break;
        }
    }
    
    // Fermeture de la socket
    close(client_socket);
}










//______________________________________________________________________________________________________________________________________________________
void ask_information_external_file(int socket, json_object *object){
    // Extraire la reponse
    char *filename;
    int client_id;
    char *client_ip;
    int client_port;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *filename_obj = NULL;
    json_object *client_id_obj = NULL;
    json_object *client_ip_obj = NULL;
    json_object *client_port_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "filename", &filename_obj);
    json_object_object_get_ex(object, "uid", &client_id_obj);
    json_object_object_get_ex(object, "ip_address", &client_ip_obj);
    json_object_object_get_ex(object, "port", &client_port_obj);
    

    // Vérification de la présence des clés et extraction des valeurs
    if (filename_obj != NULL && client_id_obj != NULL && client_ip_obj != NULL && client_port_obj != NULL) {
        client_id = json_object_get_int(client_id_obj);
        client_ip = strdup(json_object_get_string(client_ip_obj)); 
        client_port = json_object_get_int(client_port_obj);
        filename = strdup(json_object_get_string(filename_obj)); 
    }

    // Recherche du nom du fichier dans la liste des fichiers de la session
    FileNode *current_file_node = session->files;
    File *target_file = NULL;
    while (current_file_node != NULL) {
        if (strcmp(current_file_node->file->filename, filename) == 0) {
            target_file = current_file_node->file;
            break;
        }
        current_file_node = current_file_node->next;
    }

    if (target_file == NULL) {
        perror("Le fichier n'a pas été trouvé.\n");
        return;
    }

    // Création d'un objet JSON
    json_object *json_obj = json_object_new_object();
    char response[1024];
    snprintf(response, sizeof(response), "Success Open External File from host %s", filename);   
    json_object_object_add(json_obj, "response", json_object_new_string(response));

    // Préparer l'envoie des Clients
    json_object_object_add(json_obj, "taille_client", json_object_new_int(target_file->client_count));
    ClientNode *current_client_node = target_file->clients;
    int i=0;
    while (current_client_node != NULL) {
        char key_id[20];
        char key_ip[20];
        char key_port[20];
        snprintf(key_id, sizeof(key_id), "client_id_%d", i);
        snprintf(key_ip, sizeof(key_ip), "client_ip_%d", i);
        snprintf(key_port, sizeof(key_port), "client_port_%d", i);
        json_object_object_add(json_obj, key_id, json_object_new_int(current_client_node->client->uid));
        json_object_object_add(json_obj, key_ip, json_object_new_string(current_client_node->client->ip_address));
        json_object_object_add(json_obj, key_port, json_object_new_int(current_client_node->client->port));
        current_client_node = current_client_node->next;
        i++;
    }

    // Préparer l'envoie des Lignes
    json_object_object_add(json_obj, "taille_line", json_object_new_int(target_file->line_count));
    LineNode *current_line_node = target_file->lines;
    i=0;
    while (current_line_node != NULL) {
        char key_line_id[20];
        char key_line_text[20];
        snprintf(key_line_id, sizeof(key_line_id), "line_id_%d", i);
        snprintf(key_line_text, sizeof(key_line_text), "line_text_%d", i);
        json_object_object_add(json_obj, key_line_id, json_object_new_int(current_line_node->line->id));
        json_object_object_add(json_obj, key_line_text, json_object_new_string(current_line_node->line->text));
        current_line_node = current_line_node->next;
        i++;
    }

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du host au client
    if (write(socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending host information to client");
        exit(EXIT_FAILURE);
    }

    // Création du nouveau client
    Client *new_client = (Client *)malloc(sizeof(Client));
    if (new_client == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Attribution des valeurs
    new_client->uid = client_id;
    new_client->port = client_port;
    new_client->ip_address = strdup(client_ip);
    if (new_client->ip_address == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Ajout du nouveau client à la liste des clients du fichier
    ClientNode *new_client_node = (ClientNode *)malloc(sizeof(ClientNode));
    if (new_client_node == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    new_client_node->client = new_client;

    if (target_file->clients != NULL) {
        new_client_node->next = target_file->clients;
    }
    target_file->clients = new_client_node;

    target_file->client_count++;
    printf("Un nouveau client est avec nous dans le fichier %s. Maintenant il y a %d clients\n",filename,target_file->client_count);
}









//______________________________________________________________________________________________________________________________________________________
void new_client_file(int socket, json_object *object){
    // Extraire la reponse
    char *filename;
    int client_id;
    char *client_ip;
    int client_port;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *filename_obj = NULL;
    json_object *client_id_obj = NULL;
    json_object *client_ip_obj = NULL;
    json_object *client_port_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "filename", &filename_obj);
    json_object_object_get_ex(object, "uid", &client_id_obj);
    json_object_object_get_ex(object, "ip_address", &client_ip_obj);
    json_object_object_get_ex(object, "port", &client_port_obj);
    

    // Vérification de la présence des clés et extraction des valeurs
    if (filename_obj != NULL && client_id_obj != NULL && client_ip_obj != NULL && client_port_obj != NULL) {
        client_id = json_object_get_int(client_id_obj);
        client_ip = strdup(json_object_get_string(client_ip_obj)); 
        client_port = json_object_get_int(client_port_obj);
        filename = strdup(json_object_get_string(filename_obj)); 
    }

    // Recherche du nom du fichier dans la liste des fichiers de la session
    FileNode *current_file_node = session->files;
    File *target_file = NULL;
    while (current_file_node != NULL) {
        if (strcmp(current_file_node->file->filename, filename) == 0) {
            target_file = current_file_node->file;
            break;
        }
        current_file_node = current_file_node->next;
    }

    if (target_file == NULL) {
        perror("Le fichier n'a pas été trouvé.\n");
        return;
    }

    // Création du nouveau client
    Client *new_client = (Client *)malloc(sizeof(Client));
    if (new_client == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Attribution des valeurs
    new_client->uid = client_id;
    new_client->port = client_port;
    new_client->ip_address = strdup(client_ip);
    if (new_client->ip_address == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Ajout du nouveau client à la liste des clients du fichier
    ClientNode *new_client_node = (ClientNode *)malloc(sizeof(ClientNode));
    if (new_client_node == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    new_client_node->client = new_client;

    if (target_file->clients != NULL) {
        new_client_node->next = target_file->clients;
    }
    target_file->clients = new_client_node;

    target_file->client_count++;

    // Envoi d'un message au client
    if (write(socket, "Success Add Client", sizeof("Success Add Client")) == -1) {
        perror("Error sending host information to client");
        exit(EXIT_FAILURE);
    }

    printf("Un nouveau client est avec nous dans le fichier %s. Maintenant il y a %d clients\n",filename,target_file->client_count);
}









//______________________________________________________________________________________________________________________________________________________
void permission_lock_line(int socket, json_object *object){
    // Extraire la reponse
    char *filename;
    int line_id;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *filename_obj = NULL;
    json_object *line_id_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "filename", &filename_obj);
    json_object_object_get_ex(object, "line_id", &line_id_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (filename_obj != NULL && line_id_obj != NULL) {
        line_id = json_object_get_int(line_id_obj);
        filename = strdup(json_object_get_string(filename_obj)); 
    }

    printf("J'ai maintenant verrouilé la ligne %d du fichier %s\n",line_id,filename);

    // J'ai le verroue sur une ligne

}












//______________________________________________________________________________________________________________________________________________________
void ask_close_file(int socket, json_object *object){
    // Extraire la reponse
    char *filename;
    int client_id;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *filename_obj = NULL;
    json_object *client_id_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "filename", &filename_obj);
    json_object_object_get_ex(object, "uid", &client_id_obj);
    

    // Vérification de la présence des clés et extraction des valeurs
    if (filename_obj != NULL && client_id_obj != NULL) {
        client_id = json_object_get_int(client_id_obj);
        filename = strdup(json_object_get_string(filename_obj)); 
    }

    // Recherche du nom du fichier dans la liste des fichiers de la session
    FileNode *current_file_node = session->files;
    File *target_file = NULL;
    FileNode *prev_file_node = NULL;
    while (current_file_node != NULL) {
        if (strcmp(current_file_node->file->filename, filename) == 0) {
            target_file = current_file_node->file;
            break;
        }
        prev_file_node = current_file_node;
        current_file_node = current_file_node->next;
    }

    if (target_file == NULL) {
        perror("Le fichier n'a pas été trouvé.\n");
        return;
    }

    if(target_file->host_client->uid == client_id){
        // Si le client est le host
        // Fermeture du fichier

        // Suppression des lignes
        while (target_file->lines != NULL) {
            LineNode *temp_line_node = target_file->lines;
            target_file->lines = target_file->lines->next;
            free(temp_line_node->line->text);
            free(temp_line_node->line);
            free(temp_line_node);
        }

        // Suppression des clients
        while (target_file->clients != NULL) {
            ClientNode *temp_client_node = target_file->clients;
            target_file->clients = target_file->clients->next;
            free(temp_client_node->client->ip_address);
            free(temp_client_node->client);
            free(temp_client_node);
        }

        // Suppression du fichier de la liste session->files
        if (prev_file_node != NULL) {
            prev_file_node->next = current_file_node->next;
        } else {
            session->files = current_file_node->next;
        }
        free(current_file_node->file->filename);
        free(current_file_node->file->host_client->ip_address);
        free(current_file_node->file->host_client);
        free(current_file_node->file);
        free(current_file_node);
    }else{
        // Si le client n'est pas le host
        // Suppression du client

        ClientNode *current_client_node = target_file->clients;
        ClientNode *prev_client_node = NULL; 

        while (current_client_node != NULL) {
            if (current_client_node->client->uid == client_id) {
                if (prev_client_node != NULL) {
                    prev_client_node->next = current_client_node->next;
                } else {
                    target_file->clients = current_client_node->next;
                }
                free(current_client_node->client->ip_address);
                free(current_client_node->client);
                free(current_client_node);
                target_file->client_count--;
                break;
            }
            prev_client_node = current_client_node;
            current_client_node = current_client_node->next;
        }
    }

    // Envoi d'un message au client
    char response[1024];
    snprintf(response, sizeof(response), "Success Close File %s", filename);   
    if (write(socket, response, sizeof(response)) == -1) {
        perror("Error sending success message to client");
    }
}









//______________________________________________________________________________________________________________________________________________________
void information_modification_line(int socket, json_object *object){
    // Extraire la reponse
    char *filename;
    int line_id;
    char *text;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *filename_obj = NULL;
    json_object *line_id_obj = NULL;
    json_object *text_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "filename", &filename_obj);
    json_object_object_get_ex(object, "line_id", &line_id_obj);
    json_object_object_get_ex(object, "text", &text_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (filename_obj != NULL && line_id_obj != NULL && text_obj != NULL) {
        line_id = json_object_get_int(line_id_obj);
        filename = strdup(json_object_get_string(filename_obj)); 
        text = strdup(json_object_get_string(text_obj)); 
    }

    // Recherche du nom du fichier dans la liste des fichiers de la session
    FileNode *current_file_node = session->files;
    File *target_file = NULL;
    while (current_file_node != NULL) {
        if (strcmp(current_file_node->file->filename, filename) == 0) {
            target_file = current_file_node->file;
            break;
        }
        current_file_node = current_file_node->next;
    }

    if (target_file == NULL) {
        perror("Le fichier n'a pas été trouvé.\n");
        return;
    }

    // Recherche de la ligne dans la liste des lignes du fichier
    LineNode *current_line_node = target_file->lines;
    while (current_line_node != NULL) {
        if (current_line_node->line->id == line_id) {
            // Modification du texte de la ligne
            free(current_line_node->line->text);
            current_line_node->line->text = strdup(text);
            break;
        }
        current_line_node = current_line_node->next;
    }

    // Envoi d'un message au client
    char response[1024];
    snprintf(response, sizeof(response), "Success Modify Line from File %s", filename);   
    if (write(socket, response, sizeof(response)) == -1) {
        perror("Error sending success message to client");
    }
    printf("Modification de la ligne %d du fichier %s\n",line_id, filename);

}












//______________________________________________________________________________________________________________________________________________________
void information_add_line(int socket, json_object *object){
   // Extraire la reponse
    char *filename;
    int line_before_id;
    char *text;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *filename_obj = NULL;
    json_object *line_before_id_obj = NULL;
    json_object *text_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "filename", &filename_obj);
    json_object_object_get_ex(object, "line_before_id", &line_before_id_obj);
    json_object_object_get_ex(object, "text", &text_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (filename_obj != NULL && line_before_id_obj != NULL && text_obj != NULL) {
        line_before_id = json_object_get_int(line_before_id_obj);
        filename = strdup(json_object_get_string(filename_obj)); 
        text = strdup(json_object_get_string(text_obj)); 
    }

    // Recherche du nom du fichier dans la liste des fichiers de la session
    FileNode *current_file_node = session->files;
    File *target_file = NULL;
    while (current_file_node != NULL) {
        if (strcmp(current_file_node->file->filename, filename) == 0) {
            target_file = current_file_node->file;
            break;
        }
        current_file_node = current_file_node->next;
    }

    if (target_file == NULL) {
        perror("Le fichier n'a pas été trouvé.\n");
        return;
    }

    int line_id;
    if(target_file->host_client->uid == UID){
        // Si je suis le host
        // Attribué un ID a la nouvelle ligne
        line_id = target_file->line_count;
        // Création d'un objet JSON
        json_object *json_obj = json_object_new_object();
        char response[1024];
        snprintf(response, sizeof(response), "Success Add Line %s", filename);
        json_object_object_add(json_obj, response, json_object_new_string(text));
        json_object_object_add(json_obj, "line_id", json_object_new_int(line_id));
        const char *json_str = json_object_to_json_string(json_obj);
        if (write(socket, json_str, strlen(json_str)) == -1) {
            perror("Error sending JSON object to server");
            exit(EXIT_FAILURE);
        }
    }else{
        // Si je ne suis pas le host
        // Reception du id de la ligne
        json_object *line_id_obj = NULL;
        json_object_object_get_ex(object, "line_id", &line_id_obj);
        if (line_id_obj != NULL) {
            line_id = json_object_get_int(line_id_obj);
        }

        // Envoi d'un message au client
        char response[1024];
        snprintf(response, sizeof(response), "Success Add Line from File %s", filename);   
        if (write(socket, response, sizeof(response)) == -1) {
            perror("Error sending success message to client");
        }
    }

    // Ajout de la ligne
    // Création de la ligne
        Line *new_line = (Line *)malloc(sizeof(Line));
        if (new_line == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }

        // Attribution des valeurs
        new_line->id = line_id;
        new_line->text = strdup(text);

        // Ajout de la ligne a la liste lines dans le fichier
        LineNode *new_line_node = (LineNode *)malloc(sizeof(LineNode));
        if (new_line_node == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        new_line_node->line = new_line;
        new_line_node->next = NULL;

        LineNode *current_line_node = target_file->lines;
        while (current_line_node->next != NULL && current_line_node->line->id != line_before_id) {
            current_line_node = current_line_node->next;
        }

        // Insérez la nouvelle ligne après la ligne précédente
        new_line_node->next = current_line_node->next;
        current_line_node->next = new_line_node;
        target_file->line_count++;

        printf("La ligne avec l'ID %d a été ajouter dans le fichier %s. Il y a %d de ligne dans le fichier\n",line_id, filename, target_file->line_count);
}












//______________________________________________________________________________________________________________________________________________________
void information_delete_line(int socket, json_object *object){
    // Extraire la reponse
    char *filename;
    int line_id;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *filename_obj = NULL;
    json_object *line_id_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "filename", &filename_obj);
    json_object_object_get_ex(object, "line_id", &line_id_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (filename_obj != NULL && line_id_obj != NULL) {
        line_id = json_object_get_int(line_id_obj);
        filename = strdup(json_object_get_string(filename_obj)); 
    }

    // Recherche du nom du fichier dans la liste des fichiers de la session
    FileNode *current_file_node = session->files;
    File *target_file = NULL;
    while (current_file_node != NULL) {
        if (strcmp(current_file_node->file->filename, filename) == 0) {
            target_file = current_file_node->file;
            break;
        }
        current_file_node = current_file_node->next;
    }

    if (target_file == NULL) {
        perror("Le fichier n'a pas été trouvé.\n");
        return;
    }

    // Recherche de la ligne dans la liste des lignes du fichier
    LineNode *current_line_node = target_file->lines;
    LineNode *prev_line_node = NULL;
    while (current_line_node != NULL) {
        if (current_line_node->line->id == line_id) {
            // Suppression de la ligne
            if (prev_line_node != NULL) {
                prev_line_node->next = current_line_node->next;
                free(current_line_node->line->text);
                free(current_line_node->line);
                free(current_line_node);
            } else {
                // La ligne à supprimer est la première dans la liste
                current_line_node->line->text="";
            }
            break;
        }
        prev_line_node = current_line_node;
        current_line_node = current_line_node->next;
    }

    // Envoi d'un message au client
    char response[1024];
    snprintf(response, sizeof(response), "Success Delete Line from File %s", filename);   
    if (write(socket, response, sizeof(response)) == -1) {
        perror("Error sending success message to client");
    }
    printf("Suppression de la ligne %d du fichier %s\n",line_id, filename);
}