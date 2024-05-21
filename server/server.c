#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <json-c/json.h>

#define PORT 8080
ServerSession *session;
pthread_mutex_t client_mutex;
pthread_mutex_t file_mutex;
pthread_mutex_t line_mutex;

void initialize_server() {
    // Initialisation des mutex
    if (pthread_mutex_init(&client_mutex, NULL) != 0 ||
        pthread_mutex_init(&file_mutex, NULL) != 0 ||
        pthread_mutex_init(&line_mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }

    // Initialisation de la session du serveur
    session = (ServerSession *)malloc(sizeof(ServerSession));
    if (session == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialisation de la liste des clients
    session->clients = NULL;
    session->client_count = 0;

    // Initialisation de la liste des fichiers
    session->files = NULL;
    session->file_count = 0;

    // Initialisation de la liste des lignes verrouillées
    session->locked_lines = NULL;
    session->locked_line_count = 0;

    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Création de la socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration de la structure sockaddr_in
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Attachement de la socket au port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Mise en écoute de la socket
    if (listen(server_fd, 0) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server initialized and listening on port %d\n", PORT);

    while (1) {
        printf("Waiting Connection ... \n");
        int new_socket;
        struct sockaddr_in new_address;
        int addrlen = sizeof(new_address);

        // Acceptation de la connexion entrante
        if ((new_socket = accept(server_fd, (struct sockaddr *)&new_address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, receive_request_from_client, &new_socket) != 0) {
            perror("Thread creation failed");
            close(new_socket);
            continue; // Continue vers la prochaine itération de la boucle
        }

        // Détachement du thread
        pthread_detach(client_thread);
    }

    close(server_fd);
}

void *receive_request_from_client(void *args) {
    int client_socket = *(int *)args;
    
    // Traitement du client ici...
    // Lecture des données envoyées par le client
    char buffer[1024]= {0};
    ssize_t bytes_read = read(client_socket, buffer , sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading data from client");
        close(client_socket);
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
                // Connexion au serveur
                printf("-> Connexion au Serveur\n");
                connexion(client_socket, json_obj);
                break;
            case 2:
                // Demande de déconnexion
                printf("-> Déconnexion du Serveur \n");
                deconnexion(client_socket, json_obj);
                break;
            case 3:
                // Ouverture d'un fichier local
                printf("Ouverture d'un fichier local\n");
                open_local_file(client_socket, json_obj);
                break;
            case 4:
                // Ouverture d'un fichier distant
                printf("Ouverture d'un fichier distant\n");
                open_external_file(client_socket, json_obj);
                break;
            case 5:
                // Fermeture d'un fichier
                printf("Fermeture d'un fichier\n");
                close_file(client_socket, json_obj);
                break;
            case 6:
                // Lock d'une ligne
                printf("Verrouille une ligne\n");
                lock_line(client_socket, json_obj);
                break;
            case 7:
                // Unlock d'une ligne
                printf("Deverrouille une ligne\n");
                unlock_line(client_socket, json_obj);
                break;
            case 8:
                // Suppression d'une ligne
                printf("Suppression une ligne\n");
                delete_line(client_socket, json_obj);
                break;    
            default:
                // Code de requête invalide
                break;
        }
    }

    close(client_socket); // Fermeture de la socket du client après traitement
}





//______________________________________________________________________________________________________________________________________________________
void connexion(int client_socket, json_object *object){
    Client *new_client = (Client *)malloc(sizeof(Client));
    if (new_client == NULL) {
        perror("Error allocating memory for new client");
        // Envoi d'un message d'échoue au client
        if (write(client_socket, "Failed Connexion", sizeof("Failed Connexion")) == -1) {
            perror("Error sending failed message to client");
        }
        return;
    }

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *uid_obj = NULL;
    json_object *ip_obj = NULL;
    json_object *port_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "uid", &uid_obj);
    json_object_object_get_ex(object, "ip_address", &ip_obj);
    json_object_object_get_ex(object, "port", &port_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (uid_obj != NULL && ip_obj != NULL && port_obj != NULL) {
        new_client->uid = json_object_get_int(uid_obj);
        new_client->ip_address = strdup(json_object_get_string(ip_obj)); 
        new_client->port = json_object_get_int(port_obj);
    }

    // Création d'un nouveau nœud de client
    ClientNode *new_node = (ClientNode *)malloc(sizeof(ClientNode));
    if (new_node == NULL) {
        perror("Error allocating memory for new client node");
        free(new_client); // Libérer la mémoire allouée pour le client

        // Envoi d'un message d'échoue au client
        if (write(client_socket, "Failed Connexion", sizeof("Failed Connexion")) == -1) {
            perror("Error sending failed message to client");
        }
        return;
    }
    new_node->client = new_client;

    // Ajout du nouveau nœud à la liste des clients
    pthread_mutex_lock(&client_mutex);
    new_node->next = session->clients;
    session->clients = new_node;
    session->client_count++;
    pthread_mutex_unlock(&client_mutex);

    // Affichage des informations du nouveau client
    printf("New client connected: %d\n", session->client_count);
    printf("Client ID: %d\n", new_client->uid);
    printf("IP Address: %s\n", new_client->ip_address);
    printf("Port: %d\n", new_client->port);

    // Envoi d'un message de succès au client
    if (write(client_socket, "Success Connexion", sizeof("Success Connexion")) == -1) {
        perror("Error sending success message to client");
    }
}




//______________________________________________________________________________________________________________________________________________________
void deconnexion(int client_socket, json_object *object){
    int uid;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *uid_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "uid", &uid_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (uid_obj != NULL) {
        uid = json_object_get_int(uid_obj);
    }

    // Recherche du client dans la liste des clients de la session
    ClientNode *prev = NULL;
    ClientNode *current = session->clients;
    while (current != NULL && current->client->uid != uid) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        perror("Client not found\n");
        // Envoi d'un message d'échoue au client
        if (write(client_socket, "Failed Deconnexion", sizeof("Failed Deconnexion")) == -1) {
            perror("Error sending failed message to client");
        }
        return;
    }

    // Suppression du client de la liste
    pthread_mutex_lock(&client_mutex);
    if (prev == NULL) {
        session->clients = current->next;
    } else {
        prev->next = current->next;
    }
    free(current->client);
    free(current);

    session->client_count--;   
    printf("Nombre total de clients : %d\n", session->client_count);
    pthread_mutex_unlock(&client_mutex);

    // Recherche des fichiers hébergés par ce client dans la liste des fichiers du serveur
    pthread_mutex_lock(&file_mutex);
    FileNode *prev_file = NULL;
    FileNode *current_file = session->files;
    while (current_file != NULL) {
        if (current_file->file->host_client_id == uid) {
            // Le fichier est hébergé par ce client, on le supprime de la liste
            if (prev_file == NULL) {
                session->files = current_file->next;
                free(current_file->file->filename);
                free(current_file->file);
                free(current_file);
                current_file = session->files;
            } else {
                prev_file->next = current_file->next;
                free(current_file->file->filename);
                free(current_file->file);
                free(current_file);
                current_file = prev_file->next;
            }
            session->file_count--;
        } else {
            // Le fichier n'est pas hébergé par ce client, on passe au suivant
            prev_file = current_file;
            current_file = current_file->next;
        }
    }
    pthread_mutex_unlock(&file_mutex);
    printf("Nombre total de fichiers : %d\n", session->file_count);

    // Envoi d'un message de succès au client
    if (write(client_socket, "Success Deconnexion", sizeof("Success Deconnexion")) == -1) {
        perror("Error sending success message to client");
    }
    
}




//______________________________________________________________________________________________________________________________________________________
void open_local_file(int client_socket, json_object *object){
    // Extraction de l'ID du client et du nom du fichier à partir du token
    int uid;
    char *filename;
    
    // Initialisation de la variable pour stocker la valeur extraite
    json_object *uid_obj = NULL;
    json_object *filename_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "uid", &uid_obj);
    json_object_object_get_ex(object, "filename", &filename_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (uid_obj != NULL && filename_obj != NULL) {
        uid = json_object_get_int(uid_obj);
        filename = strdup(json_object_get_string(filename_obj)); 
    }

    // Création d'une nouvelle structure de fichier
    File *new_file = (File *)malloc(sizeof(File));
    if (new_file == NULL) {
        perror("Error allocating memory for new file");
        if (write(client_socket, "Failed Open Local File", sizeof("Failed Open Local File")) == -1) {
            perror("Error sending failed message to client");
        }
        return;
    }

    // Allocation de mémoire pour le nom du fichier et copie du nom
    new_file->filename = strdup(filename);
    if (new_file->filename == NULL) {
        perror("Error allocating memory for filename");
        free(new_file);
        if (write(client_socket, "Failed Open Local File", sizeof("Failed Open Local File")) == -1) {
            perror("Error sending failed message to client");
        }
        return;
    }

    // Attribution de l'ID du client hôte
    new_file->host_client_id = uid;

    // Création d'un nouveau nœud de fichier
    FileNode *new_node = (FileNode *)malloc(sizeof(FileNode));
    if (new_node == NULL) {
        perror("Error allocating memory for new file node");
        free(new_file->filename);
        free(new_file);
        if (write(client_socket, "Failed Open Local File", sizeof("Failed Open Local File")) == -1) {
            perror("Error sending failed message to client");
        }
        return;
    }
    new_node->file = new_file;
    new_node->next = NULL;

    // Ajout du nouveau nœud à la liste des fichiers de la session
    pthread_mutex_lock(&file_mutex);
    if (session->files == NULL) {
        session->files = new_node;
    } else {
        FileNode *current = session->files;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
    session->file_count++;
    pthread_mutex_unlock(&file_mutex);

    printf("New local file opened:\n");
    printf("Total File: %d\n", session->file_count);
    printf("File Name: %s\n", new_file->filename);
    printf("Host Client ID: %d\n", new_file->host_client_id);

    // Envoi d'un message de succès au client
    char response[1024];
    snprintf(response, sizeof(response), "Success Open Local File %s", filename);                
    if (write(client_socket, response, sizeof(response)) == -1) {
        perror("Error sending success message to client");
    }
}




//______________________________________________________________________________________________________________________________________________________
void open_external_file(int client_socket, json_object *object) {
    int find_file = 0;
    int host_client_id ;

    // Extrait le nom du fichier à partir du token
    char *filename;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *filename_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "filename", &filename_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (filename_obj != NULL) {
        filename = strdup(json_object_get_string(filename_obj)); 
    }

    // Recherche du fichier dans la liste des fichiers de la session
    pthread_mutex_lock(&file_mutex);
    FileNode *current_file = session->files;
    while (current_file != NULL) {
        if (strcmp(current_file->file->filename, filename) == 0) {
            // Le fichier a été trouvé
            find_file = 1;
            host_client_id = current_file->file->host_client_id;
            break;
        }
        current_file = current_file->next;
    }
    pthread_mutex_unlock(&file_mutex);

    if(find_file){
        // Recherche du client hôte dans la liste des clients de la session
        pthread_mutex_lock(&client_mutex);
        ClientNode *current_client = session->clients;
            while (current_client != NULL) {
                if (current_client->client->uid == host_client_id) {
                    // Le client hôte a été trouvé
                    
                    // Envoi de l'adresse IP et du numéro de port du client hôte au client
                    // Création d'un objet JSON
                    json_object *json_obj = json_object_new_object();
                    char response[100]; 
                    strcpy(response, "Success Open External File ");
                    strcat(response, filename);
                    json_object_object_add(json_obj, "response", json_object_new_string(response));
                    json_object_object_add(json_obj, "host_client_id", json_object_new_int(host_client_id));
                    json_object_object_add(json_obj, "host_client_ip_address", json_object_new_string(current_client->client->ip_address));
                    json_object_object_add(json_obj, "host_client_port", json_object_new_int(current_client->client->port));

                    // Conversion de l'objet JSON en chaîne de caractères
                    const char *json_str = json_object_to_json_string(json_obj);

                    // Envoi des informations du serveur au client
                    if (write(client_socket, json_str, strlen(json_str)) == -1) {
                        perror("Error sending client information to server");
                        exit(EXIT_FAILURE);
                    }

                    pthread_mutex_unlock(&client_mutex);
                    printf("External file opened:\n");
                    printf("File Name: %s\n", filename);
                    printf("Host Client ID: %d\n", host_client_id);
                    printf("Host IP Address: %s\n", current_client->client->ip_address);
                    printf("Host Port: %d\n", current_client->client->port);
                    return;
                }
                current_client = current_client->next;
            }
            pthread_mutex_unlock(&client_mutex);
            // Le client hôte n'a pas été trouvé
            // Création d'un objet JSON
            json_object *json_obj = json_object_new_object();
            json_object_object_add(json_obj, "response", json_object_new_string("Failed Open External File"));
            
            // Conversion de l'objet JSON en chaîne de caractères
            const char *json_str = json_object_to_json_string(json_obj);

            // Envoi des informations du serveur au client
            if (write(client_socket, json_str, strlen(json_str)) == -1) {
                perror("Error sending client information to server");
                exit(EXIT_FAILURE);
            }
            return;
    }else{
        // Le fichier n'a pas été trouvé
        // Création d'un objet JSON
        json_object *json_obj = json_object_new_object();
        json_object_object_add(json_obj, "response", json_object_new_string("Failed Open External File"));
        
        // Conversion de l'objet JSON en chaîne de caractères
        const char *json_str = json_object_to_json_string(json_obj);

        // Envoi des informations du serveur au client
        if (write(client_socket, json_str, strlen(json_str)) == -1) {
            perror("Error sending client information to server");
            exit(EXIT_FAILURE);
        }
    }
}





//______________________________________________________________________________________________________________________________________________________
void close_file(int client_socket, json_object *object) {
    int uid;
    char *filename;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *uid_obj = NULL;
    json_object *filename_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "uid", &uid_obj);
    json_object_object_get_ex(object, "filename", &filename_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (uid_obj != NULL && filename_obj != NULL) {
        uid = json_object_get_int(uid_obj);
        filename = strdup(json_object_get_string(filename_obj)); 
    }

    // Vérification si l'utilisateur est l'hôte du fichier ou non
    pthread_mutex_lock(&file_mutex);
    int is_host = 0;
    FileNode *file_node = session->files;
    while (file_node != NULL) {
        if (strcmp(file_node->file->filename, filename) == 0 && file_node->file->host_client_id == uid) {
            // L'utilisateur est l'hôte du fichier
            is_host = 1;
            break;
        }
        file_node = file_node->next;
    }
    pthread_mutex_unlock(&file_mutex);

    // Suppression des requêtes associées à la ligne verrouillée
    pthread_mutex_lock(&line_mutex);
    LockedLineNode *locked_line_node = session->locked_lines;
    while (locked_line_node != NULL) {
        if (strcmp(locked_line_node->line->file_id,filename)==0) {
            // Suppression des requêtes associées à cette ligne
            RequestNode *prev_req = NULL;
            RequestNode *current_req = locked_line_node->line->requests;
            while (current_req != NULL) {
                if (current_req->request->client_id == uid) {
                    // Suppression de la requête
                    if (prev_req == NULL) {
                        locked_line_node->line->requests = current_req->next;
                        free(current_req->request);
                        free(current_req);
                        current_req = locked_line_node->line->requests;
                    } else {
                        prev_req->next = current_req->next;
                        free(current_req->request);
                        free(current_req);
                        current_req=prev_req->next;
                    }
                    locked_line_node->line->request_count--;
                    printf("Suppression d'une requete! Total Requete: %d\n",locked_line_node->line->request_count);
                    break;
                }else{    
                    prev_req = current_req;
                    current_req = current_req->next;
                }
            }
        }
        locked_line_node = locked_line_node->next;
    }
    pthread_mutex_unlock(&line_mutex);

    // Suppression du fichier de la liste des fichiers
    if (is_host) {
        // Suppression du fichier seulement si l'utilisateur est l'hôte
        pthread_mutex_lock(&file_mutex);
        FileNode *prev_file_node = NULL;
        FileNode *current_file_node = session->files;
        while (current_file_node != NULL) {
            if (current_file_node->file->host_client_id == uid && strcmp(current_file_node->file->filename, filename) == 0) {
                // Le fichier a été trouvé, on le supprime
                if (prev_file_node == NULL) {
                    session->files = current_file_node->next;
                } else {
                    prev_file_node->next = current_file_node->next;
                }
                free(current_file_node->file->filename);
                free(current_file_node->file);
                free(current_file_node);
                session->file_count--;
                printf("Suppression du fichier: %s\n",filename);
                break;
            }
            prev_file_node = current_file_node;
            current_file_node = current_file_node->next;
        }
        pthread_mutex_unlock(&file_mutex);
    }

    // Envoi d'un message de succès au client
    char response[1024];
    snprintf(response, sizeof(response), "Success Close File %s", filename);   
    if (write(client_socket, response, sizeof(response)) == -1) {
        perror("Error sending success message to client");
    }
}




//______________________________________________________________________________________________________________________________________________________
void lock_line(int client_socket, json_object *object) {
    // Extraction des IDs client, ligne et fichier du token
    int client_id;
    int line_id;
    char *file_id;
    
    // Initialisation de la variable pour stocker la valeur extraite
    json_object *client_id_obj = NULL;
    json_object *line_id_obj = NULL;
    json_object *file_id_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "client_id", &client_id_obj);
    json_object_object_get_ex(object, "line_id", &line_id_obj);
    json_object_object_get_ex(object, "filename", &file_id_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (client_id_obj != NULL && line_id_obj != NULL && file_id_obj != NULL) {
        client_id = json_object_get_int(client_id_obj);
        line_id = json_object_get_int(line_id_obj);
        file_id = strdup(json_object_get_string(file_id_obj));
    }

    // Recherche de la ligne verrouillée dans la liste des lignes verrouillées
    pthread_mutex_lock(&line_mutex);
    LockedLineNode *current_locked_line_node = session->locked_lines;
    while (current_locked_line_node != NULL) {
        if (strcmp(current_locked_line_node->line->file_id,file_id)==0 && current_locked_line_node->line->line_id == line_id) {
            // La ligne est déjà verrouillée

            // Création d'une nouvelle requête
            Request *new_request = (Request *)malloc(sizeof(Request));
            if (new_request == NULL) {
                perror("Error allocating memory for new request");
                if (write(client_socket, "Failed Lock", sizeof("Failed Lock")) == -1) {
                    perror("Error sending failed message to client");
                }
                return;
            }
            new_request->client_id = client_id;
            new_request->line_id = line_id;
            new_request->file_id = file_id;

            // Création d'un nouveau nœud de requête
            RequestNode *new_request_node = (RequestNode *)malloc(sizeof(RequestNode));
            if (new_request_node == NULL) {
                perror("Error allocating memory for new request node");
                free(new_request);
                if (write(client_socket, "Failed Lock", sizeof("Failed Lock")) == -1) {
                    perror("Error sending failed message to client");
                }
                return;
            }
            new_request_node->request = new_request;
            new_request_node->next = NULL;

            // Ajout de la requête à la liste d'attente de la ligne verrouillée
            if (current_locked_line_node->line->requests == NULL) {
                current_locked_line_node->line->requests = new_request_node;
            } else {
                RequestNode *current_request_node = current_locked_line_node->line->requests;
                while (current_request_node->next != NULL) {
                    current_request_node = current_request_node->next;
                }
                current_request_node->next = new_request_node;
            }
            current_locked_line_node->line->request_count++;

            // Envoi d'un message d'Attente au client
            char response[1024];
            snprintf(response, sizeof(response), "Waiting ... %s : %d", file_id, line_id);   
            if (write(client_socket, response, sizeof(response)) == -1) {
                perror("Error sending waiting message to client");
            }

            pthread_mutex_unlock(&line_mutex);
            return; // Sortie de la fonction après traitement
        }
        current_locked_line_node = current_locked_line_node->next;
    }

    // Si la ligne n'est pas déjà verrouillée, créer une nouvelle ligne verrouillée
    LockedLine *new_locked_line = (LockedLine *)malloc(sizeof(LockedLine));
    if (new_locked_line == NULL) {
        perror("Error allocating memory for new locked line");
        if (write(client_socket, "Failed Lock", sizeof("Failed Lock")) == -1) {
            perror("Error sending failed message to client");
        }
        return;
    }
    new_locked_line->line_id = line_id;
    new_locked_line->file_id = file_id;
    new_locked_line->client_id = client_id;
    new_locked_line->requests = NULL;
    new_locked_line->request_count = 0;

    // Création d'un nouveau nœud de ligne verrouillée
    LockedLineNode *new_locked_line_node = (LockedLineNode *)malloc(sizeof(LockedLineNode));
    if (new_locked_line_node == NULL) {
        perror("Error allocating memory for new locked line node");
        free(new_locked_line);
        if (write(client_socket, "Failed Lock", sizeof("Failed Lock")) == -1) {
            perror("Error sending failed message to client");
        }
        return;
    }
    new_locked_line_node->line = new_locked_line;
    new_locked_line_node->next = NULL;

    // Ajout de la nouvelle ligne verrouillée à la liste des lignes verrouillées
    if (session->locked_lines == NULL) {
        session->locked_lines = new_locked_line_node;
    } else {
        LockedLineNode *current_node = session->locked_lines;
        while (current_node->next != NULL) {
            current_node = current_node->next;
        }
        current_node->next = new_locked_line_node;
    }
    session->locked_line_count++;
    
    // Envoi d'un message de succès au client
    char response[1024];
    snprintf(response, sizeof(response), "Success Lock Line %s : %d", file_id, line_id);   
    if (write(client_socket, response, sizeof(response)) == -1) {
        perror("Error sending success message to client");
    }
    
    pthread_mutex_unlock(&line_mutex);
}




//______________________________________________________________________________________________________________________________________________________
void unlock_line(int client_socket, json_object *object) {
    int request_waiting = 0; 

    // Extraction des IDs client, ligne et fichier du token
    int client_id;
    int line_id;
    char *file_id;

    // Initialisation de la variable pour stocker la valeur extraite
    json_object *client_id_obj = NULL;
    json_object *line_id_obj = NULL;
    json_object *file_id_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "client_id", &client_id_obj);
    json_object_object_get_ex(object, "line_id", &line_id_obj);
    json_object_object_get_ex(object, "filename", &file_id_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (client_id_obj != NULL && line_id_obj != NULL && file_id_obj != NULL) {
        client_id = json_object_get_int(client_id_obj);
        line_id = json_object_get_int(line_id_obj);
        file_id = strdup(json_object_get_string(file_id_obj));
    }

    // Recherche de la ligne verrouillée dans la liste des lignes verrouillées
    pthread_mutex_lock(&line_mutex);
    LockedLineNode *current_locked_line_node = session->locked_lines;
    LockedLineNode *prev_locked_line_node = NULL;
    while (current_locked_line_node != NULL) {
        if (strcmp(current_locked_line_node->line->file_id,file_id)==0 && current_locked_line_node->line->line_id == line_id) {
            // La ligne est verrouillée

            // Cas où le client a accès à la ligne
            if (current_locked_line_node->line->client_id == client_id) {

                if (current_locked_line_node->line->request_count == 0) {
                    // Aucune requête en attente, supprimer la ligne verrouillée
                    if (prev_locked_line_node == NULL) {
                        session->locked_lines = current_locked_line_node->next;
                    } else {
                        prev_locked_line_node->next = current_locked_line_node->next;
                    }
                    free(current_locked_line_node->line);
                    free(current_locked_line_node);
                    session->locked_line_count--;
                } else {
                    // Il y a des requêtes en attente, attribuer la ligne au prochain client
                    request_waiting = 1;
                }
                
                // Envoi d'un message de succès au client
                char response[1024];
                snprintf(response, sizeof(response), "Success Unlock Line %s : %d", file_id, line_id);   
                if (write(client_socket, response, sizeof(response)) == -1) {
                    perror("Error sending success message to client");
                }

            } else {
                // Recherche de la requête du client spécifié dans la liste des requêtes de la ligne
                RequestNode *prev_request_node = NULL;
                RequestNode *current_request_node = current_locked_line_node->line->requests;
                while (current_request_node != NULL) {
                    if (current_request_node->request->client_id == client_id) {
                        // Trouvé la requête du client

                        // Supprimer la requête de la liste
                        if (prev_request_node == NULL) {
                            current_locked_line_node->line->requests = current_request_node->next;
                        } else {
                            prev_request_node->next = current_request_node->next;
                        }
                        free(current_request_node->request);
                        free(current_request_node);
                        current_locked_line_node->line->request_count--;

                        // Envoi d'un message de succès au client
                        char response[1024];
                        snprintf(response, sizeof(response), "Success Unlock Line %s : %d", file_id, line_id);   
                        if (write(client_socket, response, sizeof(response)) == -1) {
                            perror("Error sending success message to client");
                        }

                        break; // Sortir de la boucle après avoir traité la requête du client
                    }
                    prev_request_node = current_request_node;
                    current_request_node = current_request_node->next;
                }
            }
            
            pthread_mutex_unlock(&line_mutex);

            if(request_waiting){
                pthread_mutex_lock(&client_mutex);
                Request *next_request = current_locked_line_node->line->requests->request;
                ClientNode *current_client_node = session->clients;
                while (current_client_node != NULL) {
                    if (current_client_node->client->uid == next_request->client_id) {
                        // Trouvé le client associé à la première requête
                        // Mettre à jour les informations de la ligne verrouillée
                        current_locked_line_node->line->client_id = next_request->client_id;

                        // Envoi d'un message de succès au nouveau client
                        // Création d'un nouveau client pour envoyer le message de succès
                        int new_client_socket;
                        struct sockaddr_in new_client_address;

                        if ((new_client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                            perror("Socket creation failed");
                            exit(EXIT_FAILURE);
                        }

                        // Configuration de la structure sockaddr_in
                        new_client_address.sin_family = AF_INET;
                        new_client_address.sin_port = htons(current_client_node->client->port);

                        // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
                        if (inet_pton(AF_INET, current_client_node->client->ip_address, &new_client_address.sin_addr) <= 0) {
                            perror("Invalid address / Address not supported");
                            exit(EXIT_FAILURE);
                        }

                        // Établissement de la connexion avec le client
                        if (connect(new_client_socket, (struct sockaddr *)&new_client_address, sizeof(new_client_address)) == -1) {
                            perror("Connection failed");
                            exit(EXIT_FAILURE);
                        }

                        // Envoi des informations du serveur au client
                        // Création d'un objet JSON
                        json_object *json_obj = json_object_new_object();
                        json_object_object_add(json_obj, "number", json_object_new_int(3));
                        json_object_object_add(json_obj, "line_id", json_object_new_int(line_id));
                        json_object_object_add(json_obj, "filename", json_object_new_string(file_id));

                        // Conversion de l'objet JSON en chaîne de caractères
                        const char *json_str = json_object_to_json_string(json_obj);

                        // Envoi des informations du client au serveur
                        if (write(client_socket, json_str, strlen(json_str)) == -1) {
                            perror("Error sending client information to server");
                            exit(EXIT_FAILURE);
                        }

                        close(new_client_socket);
                        pthread_mutex_unlock(&client_mutex);

                        // Supprimer la première requête de la liste
                        pthread_mutex_lock(&line_mutex);
                        RequestNode *temp_request_node = current_locked_line_node->line->requests;
                        current_locked_line_node->line->requests = current_locked_line_node->line->requests->next;
                        free(temp_request_node->request);
                        free(temp_request_node);
                        current_locked_line_node->line->request_count--;
                        pthread_mutex_unlock(&line_mutex);

                        break; // Sortir de la boucle après avoir traité la première requête
                    }
                    current_client_node = current_client_node->next;
                }
                pthread_mutex_unlock(&client_mutex);
            }
            return; // Sortie de la fonction après traitement
        }
        prev_locked_line_node = current_locked_line_node;
        current_locked_line_node = current_locked_line_node->next;
    }
    
    pthread_mutex_unlock(&line_mutex);
    

    // Si la ligne n'est pas verrouillée, envoyer un message d'erreur au client
    if (write(client_socket, "Failed Unlocked Line", sizeof("Failed Unlocked Line")) == -1) {
        perror("Error sending failed message to client");
    }
}






//______________________________________________________________________________________________________________________________________________________
void delete_line(int client_socket, json_object *object){
    // Extraction des IDs client, ligne et fichier du token
    int line_id;
    char *file_id;
    
    // Initialisation de la variable pour stocker la valeur extraite
    json_object *line_id_obj = NULL;
    json_object *file_id_obj = NULL;

    // Lecture des données du client depuis le token
    json_object_object_get_ex(object, "line_id", &line_id_obj);
    json_object_object_get_ex(object, "filename", &file_id_obj);

    // Vérification de la présence des clés et extraction des valeurs
    if (line_id_obj != NULL && file_id_obj != NULL) {
        line_id = json_object_get_int(line_id_obj);
        file_id = strdup(json_object_get_string(file_id_obj));
    }

    // Suppression des requêtes associées à la ligne verrouillée
    pthread_mutex_lock(&line_mutex);
    LockedLineNode *locked_line_node = session->locked_lines;
    while (locked_line_node != NULL) {
        if (strcmp(locked_line_node->line->file_id,file_id)==0) {
            // Suppression des requêtes associées à cette ligne
            RequestNode *prev_req = NULL;
            RequestNode *current_req = locked_line_node->line->requests;
            while (current_req != NULL) {
                if (current_req->request->line_id == line_id) {
                    // Suppression de la requête
                    if (prev_req == NULL) {
                        locked_line_node->line->requests = current_req->next;
                        free(current_req->request);
                        free(current_req);
                        current_req = locked_line_node->line->requests;
                    } else {
                        prev_req->next = current_req->next;
                        free(current_req->request);
                        free(current_req);
                        current_req=prev_req->next;
                    }
                    locked_line_node->line->request_count--;
                    break;
                }    
                prev_req = current_req;
                current_req = current_req->next;
                
            }
        }
        locked_line_node = locked_line_node->next;
    }
    pthread_mutex_unlock(&line_mutex);

    printf("Lignes Supprimées\n");

    // Envoi d'un message de succès au client
    char response[1024];
    snprintf(response, sizeof(response), "Success Delete Line %s", file_id);   
    if (write(client_socket, response, sizeof(response)) == -1) {
        perror("Error sending success message to client");
    }
}