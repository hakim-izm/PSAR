#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json-c/json.h>

#define PORT 8080

int main() {
    char response[1024];
    int client_socket;
    int client_socket2;
    struct sockaddr_in server_address;

    
    // _____________________________________________________________________________________________________ C 1


    // Configuration de la structure sockaddr_in
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);


    // Création de la socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }


    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
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
    json_object_object_add(json_obj, "uid", json_object_new_int(123));
    json_object_object_add(json_obj, "ip_address", json_object_new_string("localhost"));
    json_object_object_add(json_obj, "port", json_object_new_int(123));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str = json_object_to_json_string(json_obj);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str, strlen(json_str)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }


    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket);




    // _____________________________________________________________________________________________________ C 2


    // Configuration de la structure sockaddr_in
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);


    // Création de la socket
    if ((client_socket2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }


    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket2, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj2 = json_object_new_object();
    json_object_object_add(json_obj2, "number", json_object_new_int(1));
    json_object_object_add(json_obj2, "uid", json_object_new_int(000));
    json_object_object_add(json_obj2, "ip_address", json_object_new_string("localhost"));
    json_object_object_add(json_obj2, "port", json_object_new_int(000));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str2 = json_object_to_json_string(json_obj2);

    // Envoi des informations du client au serveur
    if (write(client_socket2, json_str2, strlen(json_str2)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket2, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }


    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket2);


    // _______________________________________________________________________________________________________________ C 1 ____ Open Local File
    // Création de la socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }


    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj4 = json_object_new_object();
    json_object_object_add(json_obj4, "number", json_object_new_int(3));
    json_object_object_add(json_obj4, "uid", json_object_new_int(123));
    json_object_object_add(json_obj4, "filename", json_object_new_string("fichier.txt"));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str4 = json_object_to_json_string(json_obj4);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str4, strlen(json_str4)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }


    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket);



    // _______________________________________________________________________________________________________________ C 2 ____ Open External File


    // Création de la socket
    if ((client_socket2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    // Établissement de la connexion avec le serveur
    if (connect(client_socket2, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj5 = json_object_new_object();
    json_object_object_add(json_obj5, "number", json_object_new_int(4));
    json_object_object_add(json_obj5, "filename", json_object_new_string("fichier.txt"));

    // Conversion de l'objet JSON en chaîne de caractères
    const char*json_str5 = json_object_to_json_string(json_obj5);

    // Envoi des informations du client au serveur
    if (write(client_socket2, json_str5, strlen(json_str5)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket2, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket2);

//________________________________________________________________________________________________________________________________________________________________



// _______________________________________________________________________________________________________________ C 1 ____ Lock Line 1
    // Création de la socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }


    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj8 = json_object_new_object();
    json_object_object_add(json_obj8, "number", json_object_new_int(6));
    json_object_object_add(json_obj8, "client_id", json_object_new_int(123));
    json_object_object_add(json_obj8, "line_id", json_object_new_int(1));
    json_object_object_add(json_obj8, "filename", json_object_new_string("fichier.txt"));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str8 = json_object_to_json_string(json_obj8);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str8, strlen(json_str8)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }


    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket);





    // _______________________________________________________________________________________________________________ C 2 ____ lOCK lINE 1

    // Création de la socket
    if ((client_socket2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    // Établissement de la connexion avec le serveur
    if (connect(client_socket2, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj9 = json_object_new_object();
    json_object_object_add(json_obj9, "number", json_object_new_int(6));
    json_object_object_add(json_obj9, "client_id", json_object_new_int(000));
    json_object_object_add(json_obj9, "line_id", json_object_new_int(1));
    json_object_object_add(json_obj9, "filename", json_object_new_string("fichier.txt"));

    // Conversion de l'objet JSON en chaîne de caractères
    const char*json_str9 = json_object_to_json_string(json_obj9);

    // Envoi des informations du client au serveur
    if (write(client_socket2, json_str9, strlen(json_str9)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket2, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket2);



// _______________________________________________________________________________________________________________ C 2 ____ UNlOCK lINE 1

    // Création de la socket
    if ((client_socket2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    // Établissement de la connexion avec le serveur
    if (connect(client_socket2, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj10 = json_object_new_object();
    json_object_object_add(json_obj10, "number", json_object_new_int(7));
    json_object_object_add(json_obj10, "client_id", json_object_new_int(000));
    json_object_object_add(json_obj10, "line_id", json_object_new_int(1));
    json_object_object_add(json_obj10, "filename", json_object_new_string("fichier.txt"));

    // Conversion de l'objet JSON en chaîne de caractères
    const char*json_str10 = json_object_to_json_string(json_obj10);

    // Envoi des informations du client au serveur
    if (write(client_socket2, json_str10, strlen(json_str10)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket2, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket2);





    // _______________________________________________________________________________________________________________ C 1 ____ UNLock Line 1
    // Création de la socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }


    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj11 = json_object_new_object();
    json_object_object_add(json_obj11, "number", json_object_new_int(7));
    json_object_object_add(json_obj11, "client_id", json_object_new_int(123));
    json_object_object_add(json_obj11, "line_id", json_object_new_int(1));
    json_object_object_add(json_obj11, "filename", json_object_new_string("fichier.txt"));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str11 = json_object_to_json_string(json_obj11);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str11, strlen(json_str11)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }


    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket);










//________________________________________________________________________________________________________________________________________________________________
    // _______________________________________________________________________________________________________________ C 2 ____ Close File

    // Création de la socket
    if ((client_socket2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    // Établissement de la connexion avec le serveur
    if (connect(client_socket2, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj6 = json_object_new_object();
    json_object_object_add(json_obj6, "number", json_object_new_int(5));
    json_object_object_add(json_obj6, "uid", json_object_new_int(000));
    json_object_object_add(json_obj6, "filename", json_object_new_string("fichier.txt"));

    // Conversion de l'objet JSON en chaîne de caractères
    const char*json_str6 = json_object_to_json_string(json_obj6);

    // Envoi des informations du client au serveur
    if (write(client_socket2, json_str6, strlen(json_str6)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket2, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket2);



    // _______________________________________________________________________________________________________________ C 1 ____ Close File
    // Création de la socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }


    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj7 = json_object_new_object();
    json_object_object_add(json_obj7, "number", json_object_new_int(5));
    json_object_object_add(json_obj7, "uid", json_object_new_int(123));
    json_object_object_add(json_obj7, "filename", json_object_new_string("fichier.txt"));

    // Conversion de l'objet JSON en chaîne de caractères
    const char *json_str7 = json_object_to_json_string(json_obj7);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str7, strlen(json_str7)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }


    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket);




    //___________________________________________________________________________________________________________ C 2
    
    // Création de la socket
    if ((client_socket2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    // Établissement de la connexion avec le serveur
    if (connect(client_socket2, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj3 = json_object_new_object();
    json_object_object_add(json_obj3, "number", json_object_new_int(2));
    json_object_object_add(json_obj3, "uid", json_object_new_int(000));

    // Conversion de l'objet JSON en chaîne de caractères
    const char*json_str3 = json_object_to_json_string(json_obj3);

    // Envoi des informations du client au serveur
    if (write(client_socket2, json_str3, strlen(json_str3)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket2, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket2);




    //___________________________________________________________________________________________________________ C 1
    
    // Création de la socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Convertit l'adresse IPv4 et l'assigne à la structure sockaddr_in
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    // Établissement de la connexion avec le serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Création d'un objet JSON
    json_object *json_obj1 = json_object_new_object();
    json_object_object_add(json_obj1, "number", json_object_new_int(2));
    json_object_object_add(json_obj1, "uid", json_object_new_int(123));

    // Conversion de l'objet JSON en chaîne de caractères
    const char*json_str1 = json_object_to_json_string(json_obj1);

    // Envoi des informations du client au serveur
    if (write(client_socket, json_str1, strlen(json_str1)) == -1) {
        perror("Error sending client information to server");
        exit(EXIT_FAILURE);
    }

    // Attente de la réponse du serveur

    if (read(client_socket, response, sizeof(response)) == -1) {
        perror("Error receiving response from server");
        exit(EXIT_FAILURE);
    }

    // Vérification de la réponse du serveur
    printf("%s\n",response);


    // Fermeture de la socket
    close(client_socket);


    return 0;
}
