//-------------------------------------------------------------------------------------------------
// FORLINI Tony et CLEMENT Rosane
// T2
// ENSEIRB-MATMECA
// 2013/2014
// Projet PR 204 : Système et Réseau
// Fichier : dsmwrap.c
// Commentaire : Processus intermediaire pour "nettoyer" la liste des arguments qu'on va passer
//		a la commande a executer vraiment.
//-------------------------------------------------------------------------------------------------
// Librairies
#include "common_impl.h"
#include <stdio.h>        // printf, perror...
#include <unistd.h>   // read, write, close...
#include <stdlib.h>   // malloc(), exit()...
#include <string.h>   // memset, strcmp...
#include <sys/select.h>   // select(), FD_ macros
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include "machine.h"
#include "liste.h"

/* variables globales */
#define BACKLOG 1024  // longueur maximale pour la file des connexions en attente (utilisée par listen())
#define PORT 0

int main(int argc, char **argv) {
	char * buffer = malloc(MAXBUF * sizeof(char));
	char * buffer_port = malloc(MAXBUF * sizeof(char));
	int port = 0; // Utilisée pour récupérer le numéro de port de la socket créée en dsmexec
	int position = 0; // Utilisée pour récupérer la position de la machine
	int socketfd = 0;
	int socket_ecoute = 0;
	struct sockaddr_in lanceuraddr; // structure des adresses pour socket lanceur
	struct sockaddr_in dsm_adresse;  // structure des adresses pour socket dsm
	int test = 0; // Gestion de EINTR
	char buf[MAXBUF];
	char buf_arg[MAXBUF];
	t_liste liste_machine = liste_new(machine_free, machine_display);

	// Récupération de argv[1] -> numéro port socket en dsmexec
	port = atoi(argv[1]); // Convertir argv[1] en int
	// Récupération de argv[3] -> @IP de machine qui exécute dsmexec
	buffer = argv[3];

	/* creation d'une socket pour se connecter au */
	/* au lanceur et envoyer/recevoir les infos */
	/* necessaires pour la phase dsm_init */
	while ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 && errno == EINTR);
	if (socketfd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Remplissage des champs de la structure de l'adresse de la socket */
	memset(&lanceuraddr, '\0', sizeof(lanceuraddr));
	lanceuraddr.sin_family = AF_INET;
	lanceuraddr.sin_port = htons(port);
	inet_aton(buffer, &lanceuraddr.sin_addr);

	//Connexion au serveur
	test = 0;
	while ((test = connect(socketfd, (struct sockaddr *) &lanceuraddr,
			sizeof(lanceuraddr))) == -1 && errno == EINTR);
	if (test == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	// Envoie de la position de la machine au lanceur -> la position est liée au nom
	position = atoi(argv[2]); // Convertir argv[2] en int
	// Envoie de la position (int)
	if (sendline(socketfd, &position, sizeof(int)) == -1) {
		perror("send dsmwrap position");
		exit(EXIT_FAILURE);
	}

	/* Creation de la socket d'ecoute pour les */
	/* connexions avec les autres processus dsm */
	while ((socket_ecoute = socket(AF_INET, SOCK_STREAM, 0)) == -1 && errno == EINTR);
	if (socket_ecoute == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	/* Remplissage des champs de la structure de l'adresse de la socket */
	memset(&dsm_adresse, '\0', sizeof(dsm_adresse));
	dsm_adresse.sin_family = AF_INET;
	dsm_adresse.sin_port = htons(PORT);
	dsm_adresse.sin_addr.s_addr = INADDR_ANY;

	// Ecoute
	test = 0;
	while ((test = listen(socket_ecoute, BACKLOG)) == -1 && errno == EINTR);
	if (test == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	// Récupération du numéro de port choisi par le système
	int size_adresse = sizeof(dsm_adresse);
	test = 0;
	while ((test = getsockname(socket_ecoute, (struct sockaddr *) &dsm_adresse,
			&size_adresse)) == -1 && errno == EINTR);
	if (test == -1) {
		perror("getsockname");
		exit(EXIT_FAILURE);
	}

	/* Envoi du numero de port au lanceur */
	/* pour qu'il le propage à tous les autres */
	/* processus dsm */
	sprintf(buffer_port, "%d", ntohs(dsm_adresse.sin_port));
	int toto = ntohs(dsm_adresse.sin_port);
	if (sendline(socketfd, &toto, sizeof(int)) == -1) {
		perror("send dsmwrap port");
		exit(EXIT_FAILURE);
	}

	// Récupération des éléments de connexion
	// envoyés par dsmexec
	buf_arg[0] = '\0';
	if (readline(socketfd, buf_arg, MAXBUF) == -1) {
		perror("read dsmwrap");
		exit(EXIT_FAILURE);
	}

	int nb_proc = atoi(recuperer_element(buf_arg, 1, ' '));
	buf[0] = '\0';
	int i = 0;
	for (i = 0; i < nb_proc; i++) {
		if (readline(socketfd, buf, MAXBUF) == -1) {
			perror("read dsmwrap");
			exit(EXIT_FAILURE);
		}
		// Ajout dans la liste des machines
		liste_add_queue(liste_machine,
				machine_new(recuperer_element(buf, 1, ' '),
						strlen(recuperer_element(buf, 1, ' ')),
						atoi(recuperer_element(buf, 2, ' ')),
						atoi(recuperer_element(buf, 3, ' ')),
						atoi(recuperer_element(buf, 4, ' '))));
	}

	// Fermeture de la socket de dsmexec (avant le exevp)
	close(socketfd);

	/* on execute la bonne commande */
	char * * newargv = malloc(sizeof(char*) * (5 + (argc - 4)));
	int iter = 0;
	for (iter = 0; iter < argc - 4; iter++) {
		newargv[iter] = argv[4 + iter];
	} // "null" est dans argv
	newargv[0] = argv[4];
	newargv[1] = recuperer_element(buf_arg, 1, ' ');
	newargv[2] = recuperer_element(buf_arg, 2, ' ');
	newargv[3] = machine_get_nom(liste_get_value(liste_machine, 1));
	newargv[4] = int2char(socket_ecoute);

	// Affichage du contenu des buffers
	fflush(stdout);
	fflush(stderr);
	// Passage au nouveau programme
	execvp(newargv[0], newargv);

	return 0;
}

//-------------------------------------------------------------------------------------------------
// Fin du fichier

