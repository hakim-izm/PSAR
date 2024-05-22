#pragma once

#include <stdlib.h>  // Pour la gestion de la mémoire dynamique
#include <json-c/json.h> // Pour la manipulation des objets JSON

#define MAX_FILENAME_LENGTH 	256 // Longueur maximale d'un nom de fichier (safe)
#define MAX_LINE_LENGTH 	1024 // Longueur maximale d'une ligne (safe)
#define ADD_APPEND 		0 // Mode d'ajout de ligne : ajout à la fin
#define ADD_INSERT 		1 // Mode d'ajout de ligne : insertion
#define SERVER_PORT		8080 // Port du serveur

// Variables globales
// int line_locked = 1; // État de verrouillage de la ligne courante

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

// Structure d'une ligne
typedef struct Line {
    char *text;         // Contenu de la ligne
    int id;             // Identifiant unique de la ligne
} Line;

// Définition de la structure de nœud pour la liste chaînée des lignes
typedef struct LineNode {
    Line *line;
    struct LineNode *next;
} LineNode;

// Structure d'un fichier
typedef struct File {
    char *filename;         // Nom du fichier
    Client *host_client;    // Structure de l'hôte
    LineNode *lines;        // Liste des lignes
    int line_count;        
    ClientNode *clients;    // Liste des clients
    int client_count;
} File;

// Définition de la structure de nœud pour la liste chaînée des fichier
typedef struct FileNode {
    File *file;             
    struct FileNode *next;
} FileNode;

// Structure d'une session sur le client
typedef struct ClientSession {
    FileNode *files;                       // Liste des fichiers
    int file_count;
} ClientSession;

// Création d'une structure pour les arguments du thread
struct ThreadArgs {
    json_object *obj;
    Client *client;
    File *file;
    int num;
};

// Méthodes 
void *initialize_client(); // Initialise le client et configure les paramètres de communication.
void *receive_request(void *args); // Reçoit une demande d'un client ou serveur.
int connexion(const char *server_ip, const char *client_ip);  // Connexion au serveur.
int deconnexion(const char *server_ip); // Déconnexion du serveur.
int open_local_file(const char *server_ip, char *filename); // Ouverture d'un fichier local.
int open_external_file(const char *server_ip, char *filename); // Ouverture d'un fichier distant.
int close_file(const char *server_ip, char *filename); // Fermeture d'un fichier.
int lock_line(const char *server_ip, char *filename, int line_id, pthread_cond_t *cond); // Verrouille une ligne.
int unlock_line(const char *server_ip, char *filename, int line_id); // Deverrouille une ligne.
int add_line(char *filename, char *text, int line_before_id); // Ajouter une ligne.
int delete_line(const char *server_ip, char *filename, int line_id); // Suppression une ligne.
int modify_line(char *filename, char *text, int line_id); // Modification d'une ligne.
void *send_all(void *arg); // Envoie un message a un clients.

void ask_information_external_file(int socket, json_object *object); // Demande des informations du fichier distant.
void new_client_file(int socket, json_object *object); // Arrivé d'un nouveau client dans un fichier.
void permission_lock_line(int socket, json_object *object); // Autorisation de Verrouiller d'une ligne après Attente.
void ask_close_file(int socket, json_object *object); // Demande de fermeture d'un fichier distant.
void information_modification_line(int socket, json_object *object); // Information de la ligne après Modification.
void information_add_line(int socket, json_object *object); // Information de la ligne après l'ajout.
void information_delete_line(int socket, json_object *object); // Information sur la ligne a supprimer.

// Méthodes locales
void local_add_line(File *file, LineNode *lines, char *text, int mode, int open); // Ajouter une ligne.
void local_delete_line(File *file, Line *line, const char *server_ip); // Supprimer une ligne.
void local_edit_line(File *file, Line *line, char *text); // Modifier une ligne.
File * local_open_external_file(char *filename, const char *server_ip); // Ouvrir un fichier externe.
File * local_open_local_file(char *filepath, const char *server_ip); // Ouvrir un fichier local.
void local_save_file(File * file, const char * filepath); // Sauvegarder un fichier.
void local_close_file(File * file, const char *server_ip); // Fermer un fichier.
