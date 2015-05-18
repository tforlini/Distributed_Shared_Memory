//-------------------------------------------------------------------------------------------------
// FORLINI Tony et CLEMENT Rosane
// T2
// ENSEIRB-MATMECA
// 2013/2014
// Projet PR 204 : Système et Réseau
// Fichier : common_impl.h
//-------------------------------------------------------------------------------------------------
// Librairies
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}

/* variables globales */
#define BACKLOG 1024  // longueur maximale pour la file des connexions en attente (utilisée par listen())
#define MAXBUF 1024


ssize_t readline(int fd,  void *str, size_t maxlen);
ssize_t sendline(int fd, const void *str, size_t maxlen);

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank;
   /* a completer */
};
typedef struct dsm_proc_conn dsm_proc_conn_t; 

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {   
  pid_t pid;
  dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;


// Convertir un int en char
char * int2char(int chiffre);



// Récupérer un élément d'une chaîne de caractères:
//-------------------------------------------------------------------------------------------------
// 1ère création :
// 	BAYLE Yann et CLEMENT Rosane
// 	T2
// 	ENSEIRB-MATMECA
//	2013/2014
//  Projet RE 216
// ------------------------------------
// Mise à jour:
// 	FORLINI Tony et CLEMENT Rosane
// 	T2
// 	ENSEIRB-MATMECA
// 	2013/2014
// 	Projet PR 204 : Système et Réseau
//-------------------------------------------------------------------------------------------------
extern char * recuperer_element(char * chaine, int num, char separateur);


//-------------------------------------------------------------------------------------------------
// Fin du fichier

