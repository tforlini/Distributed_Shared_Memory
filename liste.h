
//-------------------------------------------------------------------------------------------------
// 1ère création :
// 	BAYLE Yann et CLEMENT Rosane
// 	T1
// 	ENSEIRB-MATMECA
//	2012/2013
// ------------------------------------
// Mise à jour:
// 	FORLINI Tony et CLEMENT Rosane
// 	T2
// 	ENSEIRB-MATMECA
// 	2013/2014
// 	Projet PR 204 : Système et Réseau
// 	Fichier : liste.h
//-------------------------------------------------------------------------------------------------
// 
#pragma once

#ifndef _LISTE_H_
#define _LISTE_H_
//-------------------------------------------------------------------------------------------------

// Structure's definition
typedef struct s_liste * t_liste ;

// Set up of the liste
extern t_liste 	liste_new (  );
extern t_liste 	liste_add_queue ( t_liste liste , void * ptr );
extern t_liste  liste_update ( t_liste liste , int position, void * ptr);

// Free of the liste
extern void 	liste_free ( t_liste liste );
extern void		liste_free_element ( t_liste liste , int position );
extern void 	liste_free_element_tete ( t_liste liste );
extern void 	liste_free_element_queue ( t_liste liste );

// Display the liste
extern void 	liste_display ( t_liste liste );
extern void     liste_display_element ( t_liste liste );

// liste get
extern int		liste_get_taille(t_liste liste);
extern void * 	liste_get_value ( t_liste liste , int position);
extern void * 	liste_get_tail_value ( t_liste liste);

//-------------------------------------------------------------------------------------------------
#endif // _LISTE_H_
//-------------------------------------------------------------------------------------------------

// Fin du fichier

