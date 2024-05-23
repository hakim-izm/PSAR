# PSAR - Éditeur de fichiers

## Dépendances des include :

* gtk 4
* glib 2.0
* cairo
* pango 1.0
* harfbuzz
* gdk-pixbuf-2.0
* libjson-c-dev

### Include path (pour ma machine) : 

```
/usr/include/gtk-4.0
/usr/include/glib-2.0
/usr/lib/x86_64-linux-gnu/glib-2.0/include
/usr/include/cairo
/usr/include/pango-1.0
/usr/include/harfbuzz
/usr/include/gdk-pixbuf-2.0
/usr/include/graphene-1.0
/usr/lib/x86_64-linux-gnu/graphene-1.0/include
```

## Utilisation du serveur

### 1. Compilation du serveur

Le serveur peut être compilé en entrant la commande `make` dans le répertoire `server/` en utilisant le Makefile.

Si vous ne souhaitez pas utiliser le makefile, vous pouvez utiliser le compilateur de votre choix sur les fichiers `server.c` et `main.c` en veillant à ajouter la librairie `json-c`.

Exemple pour `gcc` : `gcc server.c main.c -o server -ljson-c`.

### 2. Démarrage du serveur

Pour démarrer le serveur, vous pouvez simplement entrer le nom de l'exécutable : `./server`.

Attention, l'utilisateur doit avoir les droits d'exécution pour ce fichier, si ce n'est pas le cas, entrez la commande suivante : `chmod +x ./server`.

## Utilisation du logiciel client (éditeur de texte)

### 1. Compilation de l'éditeur de fichier

Vous pouvez compiler en utilisant la commande `make` dans le répertoire `client/` en utilisant le Makefile.

Vous pouvez consulter le Makefile si vous souhaitez compiler manuellement.

### 2. Démarrage de l'éditeur de texte

Pour démarrer l'éditeur, vous pouvez simplement entrer le nom de l'exécutable : `./fileeditor`.

Attention, l'utilisateur doit avoir les droits d'exécution pour ce fichier, si ce n'est pas le cas, entrez la commande suivante : `chmod +x ./fileeditor`.

### 3. Définition de l'adresse IP du client

Vous devez définir manuellement l'adresse IP du client, de votre machine. Entrez l'adreesse IP qui sera visible par les autres utilisateurs. Si vous communiquez avec une machine qui est sur le même réseau que le votre, vous pouvez consulter les adresses IP pour chaque interface en entrant la commande `ip a`.

Renseignez cette adresse IP dans le menu : "Set client IP address".

Vous pouvez sauter cette étape si vous l'avez effectuée lors d'une exécution précédente, ce paramètre est sauvegardé de manière persistante.

### 4. Connexion au serveur

Ensuite, vous devez vous connecter au serveur en renseignant son adresse IP. Vous pouvez le faire dans le menu : "Connect to server".

Si la connexion s'effectue normalement, vous verrez la connexion s'afficher sur le serveur. Sinon, le logiciel client se fermera.

### 5. Ouverture d'un fichier local

Pour ouvrir un fichier local, cliquez sur l'option "Open" et sélectionnez le fichier de votre choix depuis la boîte de sélection de fichier.

Ce fichier sera maintenant accessible par les autres clients qui sont connectés sur le même serveur que vous.

Attention, seuls les fichiers purement textuels sont supportés par l'éditeur de texte.

L'éditeur de fichier peut ne pas afficher le texte, dans ce cas il faut redimensionner la fenêtre pour forcer le rafraîchissement du contenu.

### 5bis. Ouverture d'un fichier distant

Pour ouvrir un fichier local, cliquez sur l'option "Open external file" et entrez le nom du fichier que vous souhaitez ouvrir (en incluant l'extension de fichier).

Il se peut que l'ouverture d'un fichier distant échoue, dans ce cas, redémarrez l'éditeur hôte.

### 6. Sauvegarde d'un fichier local

Vous pouvez sauvegarder le contenu du fichier en cours d'affichage soit en cliquant sur l'option "Save" pour l'enregistrer dans votre répertoire courant, soit sur l'option "Save As" pour l'enregistrer dans le répertoire de votre choix.

## Bugs connus

- **PROBLÈME :** L'éditeur de fichier peut ne pas afficher le texte après une modification ou une ouverture. \
**SOLUTION :** Redimensionnez la fenêtre ou cliquez sur une des zones de texte à l'écran.

- **PROBLÈME :** La suppression d'une ligne pendant qu'un autre client la modifie entraîne un plantage du programme.

- **PROBLÈME :** Durant l'ouverture d'un fichier distant, il se peut que le nouveau client ne parvienne par à ouvrir ce fichier. Ce bug ne se produit pas à chaque ouverture. \
**SOLUTION :** Redémarrez les autres clients connectés sur ce fichiers ainsi que l'hôte. Retentez l'opération. Si l'échec persiste, redémarrez le serveur. 