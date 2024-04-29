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
void add_line(LineNode *lines, Line *line); // Ajouter une ligne.
// remove_line() // Supprimer une ligne.
// edit_line() // Modifier une ligne.
// create_file() // Créer un fichier.
File * open_local_file(char *filepath); // Ouvrir un fichier local.
// open_external_file() // Ouvrir un fichier distant.
// save_file() // Sauvegarder un fichier.
// close_file() // Fermer un fichier.
// quit() // Quitter l'éditeur.
