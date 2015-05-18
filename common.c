//-------------------------------------------------------------------------------------------------
// FORLINI Tony et CLEMENT Rosane
// T2
// ENSEIRB-MATMECA
// 2013/2014
// Projet PR 204 : Système et Réseau
// Fichier : common.c
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
#include <assert.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

// Lecture
ssize_t readline(int fd, void *str, size_t maxlen) {
	int n, rc;
	char c;
	char * buffer = str;
	for (n = 1; n < maxlen; n++) {
		rc = read(fd, &c, 1);
		if (rc == 1) {
			*buffer++ = c;
			if (c == '\n') {
				break;
			}
		} else if (rc == 0) {
			if (n == 1) {
				return 0;
			} else
				break;
		}

		else {
			if (errno == EINTR)
				continue;
			return -1;
		}

	}
	*buffer = 0;
	return n;
}

// Envoie
ssize_t sendline(int fd, const void *str, size_t maxlen) {

	int nleft, nwritten;
	const char * buffer = str;
	nleft = maxlen;
	while (nleft > 0) {
		nwritten = write(fd, str, nleft);
		if (nwritten <= 0) {

			if (errno == EINTR) {
				nwritten = 0;
			} else {
				return -1;
			}
		}
		nleft -= nwritten;
		buffer += nwritten;
	}
	return maxlen;

}

// Convertir un int en char
char * int2char(int chiffre) {
	char * buffer = malloc(MAXBUF * sizeof(char));
	buffer[0] = '\0';
	sprintf(buffer, "%d", chiffre);
	return buffer;
}

// Récupérer un élément d'une chaîne de caractère:
//-------------------------------------------------------------------------------------------------
// 1ère création :
// 	BAYLE Yann et CLEMENT Rosane
// 	T2
// 	ENSEIRB-MATMECA
//	2013/2014
// 	Projet RE 216
// ------------------------------------
// Mise à jour:
// 	FORLINI Tony et CLEMENT Rosane
// 	T2
// 	ENSEIRB-MATMECA
// 	2013/2014
// 	Projet PR 204 : Système et Réseau
//-------------------------------------------------------------------------------------------------
extern char * recuperer_element(char * chaine, int num, char separateur) {
	assert(chaine);
	char * tmp = malloc(MAXBUF * sizeof(char));
	assert(tmp);
	memset(tmp, 0, MAXBUF);
	int iter = 0; // Iteration sur la chaine de caractères
	int information = 0; // Information courante dans la chaine de caractères
	int i = 0;
	for (information = 0; information < num - 1; information++, iter++) {
		while (chaine[iter] != separateur) {
			iter++;
		}
	}
	while (chaine[iter] != separateur && chaine[iter] != '\0') {
		tmp[i] = chaine[iter];
		iter++;
		i++;
	}
	tmp[i + 1] = '\0';
	return tmp;
}

//-------------------------------------------------------------------------------------------------
// Fin du fichier
