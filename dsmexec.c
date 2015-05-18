
//-------------------------------------------------------------------------------------------------
// FORLINI Tony et CLEMENT Rosane
// T2
// ENSEIRB-MATMECA
// 2013/2014
// Projet PR 204 : Système et Réseau
// Fichier : dsmexec.c
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
#include <poll.h>
#include <netdb.h>
#include "machine.h"
#include "liste.h"

/* variables globales */
#define BACKLOG 1024  // Longueur maximale pour la file des connexions en attente (utilisée par listen())
#define PORT 33000

/* Tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;

/* Le nombre de processus effectivement crées */
volatile int num_procs_creat = 0;

void usage(void) {
	fprintf(stdout, "Usage : dsmexec machine_file executable arg1 arg2 ...\n");
	fflush(stdout);
	exit(EXIT_FAILURE);
}

/* Fonction: on traite les fils qui se terminent */
/* pour eviter les zombies */
void sigchld_handler(int sig) {
	if (sig == SIGCHLD) {
		int status;
		wait(&status);
		printf("=== Fils terminé ===\n");
	}
}

int main(int argc, char *argv[]) {

	// Déclaration et initialisation des variables

	int sock_serv;             // socket d'écoute
	int port;                  // numéro de port
	struct sockaddr_in servaddr;  // structure des adresses pour socket
	struct sockaddr_in csin;     // taille des adresses de connexion
	socklen_t recsize = sizeof(csin); // utilisée pour accept()
	const char* fichier = malloc(MAXBUF * sizeof(char));
	fichier = argv[1]; // obtenir machine_file par la ligne de cmd
	FILE * fd; // utilisée pour le fichier machines
	int num_procs = 0; // utilisée pour compter le nombre de processus (nb de lignes dans le fichier machines)
	char buf[MAXBUF]; // pour fgets()
	char buffer[MAXBUF]; // = malloc(MAXBUF*sizeof(char)) ; // Pour le read
	int buf_port = 0; // Pour le read
	t_liste liste_machine = liste_new(machine_free, machine_display); // Création d'une liste de machines
	char recu[MAXBUF]; // recu sur le fds
	int resultat_recu = 0; // Résultat du read
	int test = 0;
	int i;
	int position;
	int fd_port; // Utilisée par accept()

	// Nettoie le terminal avant execution
	system("clear");

	if (argc < 3) {
		usage();
	} else {

		// Affichage
		printf("=====================================\n");
		printf("===== PR204 : Système et Réseaux ====\n");
		printf("=== FORLINI Tony et CLEMENT Rosane ==\n");
		printf("======= ENSEIRB-MATMECA - T2 ========\n");
		printf("============= 2013-2014 =============\n");
		printf("=====================================\n");
		printf("====== Excécution du programme ======\n");
		printf("=====================================\n");

		/* Mise en place d'un traitant pour recuperer les fils zombies*/
		struct sigaction sigchld;
		sigchld.sa_handler = sigchld_handler;
		sigaction(SIGCHLD, &sigchld, NULL );

		/* Lecture du fichier de machines */
		fd = fopen(fichier, "r");
		if (fd == 0 && errno != EINTR) { // Gestion des erreurs de fopen()
			perror("fopen");
			exit(EXIT_FAILURE);
		}

		while (fgets(buf, MAXBUF, fd) != NULL ) {
			/* 1- on recupere les noms des machines et on les ajoute dans la liste des machines */
			liste_add_queue(liste_machine,
					machine_new(buf, strlen(buf) - 1, 0, num_procs + 1, 0)); // Pour la taille on a "strlen(buf)-1", car sinon on compte le "\n"
			/* 2- on recupere le nombre de processus a lancer */
			num_procs++;
		}
		printf("\n=== Affichage des machines ===\n");
		for (i = 0; i < liste_get_taille(liste_machine); i++) {
			machine_display(liste_get_value(liste_machine, i + 1)); // Affichage de toutes les machines
		}
		printf("=== Il y a %d machine(s). ===\n", num_procs); // Affichage du nb de lignes récupérées

		fclose(fd); // Fermeture du fichier

		/* Creation de la socket d'ecoute */
		port = PORT; // Numéro de port
		while ((sock_serv = socket(AF_INET, SOCK_STREAM, 0)) == -1
				&& errno == EINTR);
		if (sock_serv == -1) {
			perror("socket");
			exit(EXIT_FAILURE);
		}

		/* Remplissage des champs de la structure de l'adresse de la socket */
		memset(&servaddr, '\0', sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(port);
		servaddr.sin_addr.s_addr = INADDR_ANY;

		signal(SIGPIPE, SIG_IGN); // Utilisé pour récupérer le signal "Broken pipe"

		// Permettre la réutilisation de la socket
		int optval = 1;
		setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, &optval,
				sizeof(optval));

		/* fournir un nom à la socket (bind())*/
		while ((test = bind(sock_serv, (struct sockaddr *) &servaddr,
				sizeof(servaddr))) == -1 && errno == EINTR);
		if (test == -1) {
			perror("bind");
			exit(EXIT_FAILURE);
		}

		printf("\n=== La socket %d est connectée en mode TCP/IP ===\n",
				sock_serv);

		// Ecoute
		while ((test = listen(sock_serv, BACKLOG)) == -1 && errno == EINTR);
		if (test == -1) {
			perror("listen");
			exit(EXIT_FAILURE);
		}

		/* Creation des fils */
		pid_t pid[num_procs];
		pid_t pid_pere = getpid();

		// Création des tableaux qui contiendront les tubes de tous les fils
		int * * tube_stdout = malloc(sizeof(int*) * num_procs);
		int * * tube_stderr = malloc(sizeof(int*) * num_procs);

		for (i = 0; i < num_procs; i++) {

			/* Creation du tube pour rediriger stdout */
			tube_stdout[i] = malloc(sizeof(int) * 2);
			if (pipe(tube_stdout[i]) == -1 && errno != EINTR) {
				perror("tube_stdout");
				exit(EXIT_FAILURE);
			}

			/* Creation du tube pour rediriger stderr */
			tube_stderr[i] = malloc(sizeof(int) * 2);
			if (pipe(tube_stderr[i]) == -1 && errno != EINTR) {
				perror("tube_stderr");
				exit(EXIT_FAILURE);
			}

			if (getpid() == pid_pere) { // S'il s'agit du père alors on effetue le fork()
				while((pid[i] = fork())==-1 && errno == EINTR);
				if (pid[i] == -1)
					ERROR_EXIT("fork");
			}

			if (pid[i] == 0) { /* fils */
				// Fermeture de tous les fd non-utiles
				int ite = 0; // Initialisation de la variable temporaire utilisée
				for (ite = i - 1; ite >= 0; ite--) {
					if (close(tube_stdout[ite][0]) == -1 && errno != EINTR) {
						perror("close tube_stdout 0");
						exit(EXIT_FAILURE);
					}
					if (close(tube_stderr[ite][0]) == -1 && errno != EINTR) {
						perror("close tube_stderr 0");
						exit(EXIT_FAILURE);
					}
				}

				/* redirection stdout */
				if (close(1) == -1 && errno != EINTR) {
					perror("close stdout fils");
					exit(EXIT_FAILURE);
				}
				if (close(tube_stdout[i][0]) == -1 && errno != EINTR) {
					perror("close tube_stdout fils");
					exit(EXIT_FAILURE);
				}
				dup(tube_stdout[i][1]);
				if (close(tube_stdout[i][1]) == -1 && errno != EINTR) {
					perror("close tube_stdout end fils");
					exit(EXIT_FAILURE);
				}

				/* redirection stderr */
				if (close(2) == -1 && errno != EINTR) {
					perror("close stderr fils");
					exit(EXIT_FAILURE);
				}
				if (close(tube_stderr[i][0]) == -1 && errno != EINTR) {
					perror("close tube_stderr fils");
					exit(EXIT_FAILURE);
				}
				dup(tube_stderr[i][1]);
				if (close(tube_stderr[i][1]) == -1 && errno != EINTR) {
					perror("close tube_stderr end fils");
					exit(EXIT_FAILURE);
				}

				/* Creation du tableau d'arguments pour le ssh */
				char * * newargv = malloc(sizeof(char*) * (argc + 6));
				newargv[0] = "ssh";
				newargv[1] = machine_get_nom(
						liste_get_value(liste_machine, i + 1));
				newargv[2] = "./bin/dsmwrap";
				// Conversion du numéro de port
				newargv[3] = int2char(port);
				// Conversion de la position
				newargv[4] = int2char(
						machine_get_position(
								liste_get_value(liste_machine, i + 1)));

				char hostname[256];
				struct hostent * res;
				struct in_addr * addr;
				// Obtenir le nom de la machine
				if (gethostname(hostname, sizeof(hostname)) == -1) {
					perror("gethostname");
					exit(EXIT_FAILURE);
				}
				// Obtenir l'@IP à partir du nom de la machine
				res = gethostbyname(hostname);
				addr = (struct in_addr*) res->h_addr_list[0];
				newargv[5] = inet_ntoa(*addr);

				// On complète le tableau par la fin de la ligne de commande
				int iter = 0;
				for (iter = 0; iter <= argc - 2; iter++) { // " iter <= argc-2 " permet d'obtenir (null) en dernier
					newargv[6 + iter] = argv[2 + iter];
				}

				// Affichage du contenus des buffers
				fflush(stdout);
				fflush(stderr);
				/* On passe dans l'autre programme : */
				execvp("ssh", newargv);

			} // Fin fils
			else if (pid[i] > 0) { /* pere */

				/* Fermeture des extremites des tubes non utiles */
				if (close(tube_stdout[i][1]) == -1 && errno != EINTR) {
					perror("close tube_stdout père");
					exit(EXIT_FAILURE);
				}
				if (close(tube_stderr[i][1]) == -1 && errno != EINTR) {
					perror("close tube_stderr père");
					exit(EXIT_FAILURE);
				}
				num_procs_creat++;
			} // Fin père

		} // Fin for


		printf("\n=== Début de la mise à jour des machines ===\n");
		for (i = 0; i < num_procs; i++) {
			/* on accepte les connexions des processus dsm */
			fd_port = accept(sock_serv, (struct sockaddr *) &csin, &recsize);
			while (fd_port == -1 && errno == EINTR) {
				fd_port = accept(sock_serv, (struct sockaddr *) &csin,
						&recsize);
			}
			if (fd_port == -1 && errno != EINTR) {
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//  Récupération de la position de la machine distante
			//  (int)
			if (readline(fd_port, &position, sizeof(int)) == -1) {
				perror("read position");
				exit(EXIT_FAILURE);
			}

			/* On recupere le numero de port de la socket */
			/* d'ecoute des processus distants */
			if (readline(fd_port, &buf_port, sizeof(int)) == -1) {
				perror("read port number");
				exit(EXIT_FAILURE);
			}

			// Mise à jour de la liste des machines
			liste_update(liste_machine, position,
					machine_update_port(
							liste_get_value(liste_machine, position),
							buf_port));
			liste_update(liste_machine, position,
					machine_update_fd(liste_get_value(liste_machine, position),
							fd_port));
			buf_port = 0; // Réinitialisation de la variable pour une utilisation ultérieure
			// Affichage de la liste des machines
			printf("=== La liste des machines a été mise à jour ===\n");
			machine_display(liste_get_value(liste_machine, position));

		} // Fin for

		/* envoi du nombre de processus aux processus dsm*/
		/* envoi des rangs aux processus dsm */
		/* envoi des infos de connexion aux processus */
		int j = 0;

		printf("\n===  Début de l'envoie des informations  ===\n");
		for (i = 0; i < num_procs_creat; i++) {
			position = machine_get_position(
					liste_get_value(liste_machine, i + 1)) - 1; // Car position commence à 1 et ID commence à 0
			fd_port = machine_get_fd(liste_get_value(liste_machine, i + 1));
			// Envoi des infos de connexion aux processus : nombre de processus et position du processus
			buffer[0] = '\0';
			strcat(buffer, int2char(num_procs));
			strcat(buffer, " ");
			strcat(buffer, int2char(position));
			strcat(buffer, "\n\0");
			if (sendline(fd_port, buffer, strlen(buffer))
					== -1 && errno != EINTR) {
				perror("send nom machine");
				exit(EXIT_FAILURE);
			}

			// Envoi des infos de connexion aux processus : parcourt de la liste des machines
			for (j = 0; j < num_procs_creat; j++) {
				t_machine machine_courante = liste_get_value(liste_machine,
						j + 1);
				buffer[0] = '\0';
				strcat(buffer, machine_get_nom(machine_courante));
				strcat(buffer, " ");
				strcat(buffer, int2char(machine_get_port(machine_courante)));
				strcat(buffer, " ");
				strcat(buffer,
						int2char(machine_get_position(machine_courante)));
				strcat(buffer, " ");
				strcat(buffer, int2char(fd_port));
				strcat(buffer, "\n\0");
				if (sendline(fd_port, buffer, strlen(buffer)) == -1) {
					perror("send parcourt de la liste des machines");
					exit(EXIT_FAILURE);
				}
			}

			printf("=== Envoie effectué pour la machine : %s ===\n",
					machine_get_nom(liste_get_value(liste_machine, position)));
		}

		/* gestion des E/S : on recupere les caracteres */
		/* sur les tubes de redirection de stdout/stderr */
		struct pollfd fds[num_procs_creat * 2]; // tableau de fd pour poll (stdout et stderr)
		int numero = 0; // utilisée pour poll
		int tube_ferme = 0; // Compteur pour arrêter le while quand tous les tubes sont fermés

		// Initialisation
		for (i = 0; i < num_procs_creat; i++) {
			// Pour stderr
			fds[i].fd = tube_stderr[i][0];
			fds[i].events = POLLIN | POLLHUP;
		}
		for (i = num_procs_creat; i < num_procs_creat * 2; i++) {
			// Pour stdout
			fds[i].fd = tube_stdout[i - num_procs_creat][0];
			fds[i].events = POLLIN | POLLHUP;
		}

		printf("\n===  Récupération des E/S  ===\n");
		while (tube_ferme != num_procs_creat * 2) {
			//   on recupere les infos sur les tubes de redirection
			//   jusqu'à ce qu'ils soient inactifs (ie fermes par les
			//   processus dsm ecrivains de l'autre cote ...)

			numero = poll(fds, num_procs_creat * 2, 0);
			if (numero == -1 && errno != EINTR) {
				perror("poll");
				exit(EXIT_FAILURE);
			} else if (numero > 0) {
				for (i = 0; i < num_procs_creat * 2; i++) {
					if (i < num_procs_creat) {

						if (fds[i].revents & POLLIN) {
							recu[0] = '\0';
							resultat_recu = read(tube_stderr[i][0], recu,
									MAXBUF + 1);
							if (resultat_recu == -1 && errno != EINTR) {
								perror("read de tube_stderr[i][0] : ");
								exit(EXIT_FAILURE);
							}
							recu[resultat_recu - 1] = '\0';
							printf("child stderr %s\n", recu);
						}
						// Si revents POLLHUP alors tube fermé
						// Fermeture tube et incrémentation tube_ferme
						if (fds[i].revents & POLLHUP) {
							// Fermeture de notre côté du tube
							if (close(tube_stderr[i][0]) == -1 && errno != EINTR) {
								perror("close tube_stderr POLLHUP");
								exit(EXIT_FAILURE);
							}
							tube_ferme++;
							// A decommenter si besoin
							//printf("POLLHUP \n"); 
						}
					} // Fin de if i < num_procs_creat
					else { // if i > num_procs_creat
						if (fds[i].revents & POLLIN) {
							recu[0] = '\0';
							resultat_recu = read(
									tube_stdout[i - num_procs_creat][0], recu,
									MAXBUF + 1);
							if (resultat_recu == -1 && errno != EINTR) {
								perror(
										"read de tube_stdout[i-num_procs_creat][0] : ");
								exit(EXIT_FAILURE);
							}
							recu[resultat_recu - 1] = '\0';
							printf("child stdout %s\n", recu);
						}
						// Si revents POLLHUP alors tube fermé
						// Fermeture tube et incrémentation tube_ferme
						if (fds[i].revents & POLLHUP) {
							// Fermeture de notre côté du tube
							if (close(tube_stdout[i - num_procs_creat][0])
									== -1&& errno != EINTR) {
								perror("close tube_stdout POLLHUP");
								exit(EXIT_FAILURE);
							}
							tube_ferme++;
							// A decommenter si besoin
							//printf("POLLHUP \n"); 
						}

					} // Fin else numero > num_procs_creat

				} // Fin for

			} // Fin else

		};  // Fin While(tube_ferme != num_procs_creat * 2)

		/* on attend les processus fils */
		printf("\n===  Attente des processus fils  ===\n");
		for (i = 0; i < num_procs; i++) {
			int status;
			wait(&status);
			printf("Mon fils n°%i s'est terminé avec le code %i\n", pid[i],
					WEXITSTATUS(status));
		}

		/* on ferme les descripteurs proprement */
		/* on ferme la socket d'ecoute */
		close(sock_serv);

	} // Fin else pour la vérification de la ligne de commande (argc >= 3)

	// On fait un free de la liste des machines
	liste_free(liste_machine);

	exit(EXIT_SUCCESS);

} // Fin main

//-------------------------------------------------------------------------------------------------
// Fin du fichier
