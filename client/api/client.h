#pragma once

#define MAX_FILENAME_LENGTH 	256 // Longueur maximale d'un nom de fichier (safe)
#define MAX_LINE_LENGTH 	1024 // Longueur maximale d'une ligne (safe)
#define ADD_APPEND 		0 // Mode d'ajout de ligne : ajout à la fin
#define ADD_INSERT 		1 // Mode d'ajout de ligne : insertion

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
//     struct Line *next;  // Pointeur vers l'élément suivant
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

// Méthode 
// initialize_client() // Initialise le client et configure les paramètres de communication.
// connect_to_server() // Établit une connexion avec le serveur.
// send_request_to_server() // Envoie une demande au serveur.
// receive_response_from_server() // Reçoit une réponse du serveur.
// send_request_to_client() // Envoie une demande à un client.
// receive_response_from_client() // Reçoit une réponse d'un client.
void local_add_line(File *file, LineNode *lines, const char *text, int mode); // Ajouter une ligne.
void local_remove_line(File *file, Line *line); // Supprimer une ligne.
void local_edit_line(Line *line, char *text); // Modifier une ligne.
// create_file() // Créer un fichier.
File * local_open_local_file(char *filepath); // Ouvrir un fichier local.
// open_external_file() // Ouvrir un fichier distant.
void local_save_file(File * file, const char * filepath); // Sauvegarder un fichier.
void local_close_file(File * file); // Fermer un fichier.
// quit() // Quitter l'éditeur.
