//-------------------------------------------------------------------------------------------------
// FORLINI Tony et CLEMENT Rosane
// T2
// ENSEIRB-MATMECA
// 2013/2014
// Projet PR 204 : Système et Réseau
// Fichier : machine.c
//-------------------------------------------------------------------------------------------------
// Librairies
#include <stdio.h>      	// printf, perror...
#include <unistd.h>		// read, write, close...
#include <stdlib.h>		// malloc(), exit()...
#include <string.h>		// memset, strcmp...
#include <sys/select.h>		// select(), FD_ macros
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "machine.h"

struct s_machine {
	char * nom;
	int taille;
	int num_port;
	int position;
	int fd; // pour le accept
};

// Nouvelle machine
t_machine machine_new(char * nom, int taille, int num_port, int position,
		int fd) {
	t_machine machine = malloc(sizeof(*machine));
	if (!machine)
		perror("Memory error");
	machine->nom = malloc(sizeof(nom));
	machine->taille = taille;
	machine->num_port = num_port;
	machine->position = position;
	machine->fd = fd;
	int i = 0;
	while (nom[i] != '\0' && nom[i] != '\n') {
		machine->nom[i] = nom[i];
		i++;
	}
	return machine;
}

// Modifier l'élément
t_machine machine_update(t_machine machine, char * nom, int taille,
		int num_port, int position, int fd) {
	assert(machine != NULL);
	machine->taille = taille;
	machine->num_port = num_port;
	machine->position = position;
	machine->fd = fd;
	int i = 0;
	while (nom[i] != '\0') {
		machine->nom[i] = nom[i];
		i++;
	}
	return machine;
}

t_machine machine_update_port(t_machine machine, int num_port) {
	assert(machine != NULL);
	machine->num_port = num_port;
	return machine;
}

t_machine machine_update_fd(t_machine machine, int fd) {
	assert(machine != NULL);
	machine->fd = fd;
	return machine;
}

// Obtenir informations à propos de la machine
extern char * machine_get_nom(t_machine machine) {
	assert(machine != NULL);
	return machine->nom;
}
extern int machine_get_position(t_machine machine) {
	return machine->position;
}
extern int machine_get_fd(t_machine machine) {
	return machine->fd;
}
extern int machine_get_port(t_machine machine) {
	return machine->num_port;
}

// Free de la machine
void machine_free(t_machine machine) {
	if (machine->nom != NULL )
		free(machine->nom);
}

// Affichage de la machine
void machine_display(t_machine machine) {
	assert(machine != NULL);
	printf(
			"MACHINE : %s , taille du nom : %i , numéro de port : %i , position : %i\n",
			machine_get_nom(machine), machine->taille, machine->num_port,
			machine->position);
}

//-------------------------------------------------------------------------------------------------
// Fin du fichier

