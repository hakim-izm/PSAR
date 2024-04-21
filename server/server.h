#include <pthread.h> // Pour la gestion des threads
#include <stdlib.h>  // Pour la gestion de la mémoire dynamique
#include <json-c/json.h> // Pour la manipulation des objets JSON

// Structure d'une requête
typedef struct Request {
    int client_id;          // ID du client
    int line_id;            // ID de la ligne
    char *file_id;            // Nom du fichier
} Request;

// Définition de la structure de nœud pour la liste chaînée des Request
typedef struct RequestNode {
    Request *request;             
    struct RequestNode *next;
} RequestNode;

// Structure d'une ligne verrouillée
typedef struct LockedLine {
    int line_id;                      // ID de la ligne
    char *file_id;                      // Nom du fichier
    int client_id;                    // ID du client qui a verrouillé la ligne
    RequestNode *requests;          // File d'attente des requêtes en attente
    int request_count;
} LockedLine;

// Définition de la structure de nœud pour la liste chaînée des LockedLine
typedef struct LockedLineNode {
    LockedLine *line;             
    struct LockedLineNode *next;
} LockedLineNode;

// Structure d'un client
typedef struct Client {
    int uid;            // ID du client
    char *ip_address;   // Adresse IP du client
    int port;           // Port du client
} Client;

// Définition de la structure de nœud pour la liste chaînée des clients
typedef struct ClientNode {
    Client *client;             
    struct ClientNode *next;
} ClientNode;

// Structure d'un fichier
typedef struct File {
    char *filename;     // Nom du fichier
    int host_client_id; // ID du client hôte
} File;

// Définition de la structure de nœud pour la liste chaînée des files
typedef struct FileNode {
    File *file;             
    struct FileNode *next;
} FileNode;

// Structure d'une session sur le serveur
typedef struct ServerSession {
    ClientNode *clients;                    // Liste des clients
    int client_count;
    FileNode *files;                        // Liste des fichiers
    int file_count;
    LockedLineNode *locked_lines;           // Liste des lignes verrouillées
    int locked_line_count;
} ServerSession;

// Méthode 
void initialize_server(); // Initialise le serveur et configure les paramètres de communication.
void *receive_request_from_client(void *args); // Reçoit une demande d'un client.
void connexion(int client_socket, json_object *object); // Connexion du client au serveur.
void deconnexion(int client_socket, json_object *object); // Déconnexion du client du serveur.
void open_local_file(int client_socket, json_object *object); // Ouverture d'un fichier local.
void open_external_file(int client_socket, json_object *object); // Ouverture d'un fichier distant.
void close_file(int client_socket, json_object *object); // Fermeture d'un fichier.
void lock_line(int client_socket, json_object *object); // Verrouille une ligne.
void unlock_line(int client_socket, json_object *object); // Deverrouille une ligne.