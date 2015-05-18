
//-------------------------------------------------------------------------------------------------
// FORLINI Tony et CLEMENT Rosane
// T2
// ENSEIRB-MATMECA
// 2013/2014
// Projet PR 204 : Système et Réseau
// Fichier : machine.h
//-------------------------------------------------------------------------------------------------
// Librairies
#pragma once

#ifndef _MACHINE_H_
#define _MACHINE_H_
//-------------------------------------------------------------------------------------------------

// Définition de la structure
typedef struct s_machine * t_machine ;

// Nouvelle machine
extern t_machine machine_new(char * nom, int taille, int num_port, int position, int fd);

// Modifier l'élément
extern t_machine machine_update(t_machine machine, char * nom, int taille, int num_port, int position, int fd);
extern t_machine machine_update_port(t_machine machine, int num_port);
extern t_machine machine_update_fd(t_machine machine, int fd);

// Obtenir informations d'une machine
extern char * machine_get_nom(t_machine machine);
extern int machine_get_position(t_machine machine);
extern int machine_get_fd(t_machine machine);
extern int machine_get_port(t_machine machine);

// Free de la machine
extern void machine_free(t_machine machine);

// Afficher la machine
extern void machine_display(t_machine machine);

//-------------------------------------------------------------------------------------------------
#endif // _MACHINE_H_
//-------------------------------------------------------------------------------------------------
// Fin du fichier
