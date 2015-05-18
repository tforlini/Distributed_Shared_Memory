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
// 	Fichier : liste.c
//-------------------------------------------------------------------------------------------------
// Librairies
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "liste.h"
//-------------------------------------------------------------------------------------------------

struct s_noeud {
	struct s_noeud * suivant;
	struct s_noeud * prec;
	void * element;
};
typedef struct s_noeud * t_noeud;

struct s_liste {
	struct s_noeud * tete;
	struct s_noeud * queue;
	int taille;
	void (*free)(void *val);
	void (*display)(void *val_display);
};

t_liste liste_new(void (*ptr_free), void (*ptr_display)) {
	t_liste liste = malloc(sizeof(*liste));
	assert(liste);

	liste->tete = NULL;
	liste->queue = NULL;
	liste->taille = 0;
	liste->free = ptr_free;
	liste->display = ptr_display;

	return liste;
}

// Ajouter un nouvel élément à la queue de la liste et retourner la liste
t_liste liste_add_queue(t_liste liste, void * ptr) {
	assert(liste);
	assert(ptr);
	t_noeud new_noeud = malloc(sizeof(*new_noeud));
	assert(new_noeud);
	t_noeud temp = malloc(sizeof(*temp));
	assert(temp);

	if (liste->tete == NULL && liste->queue == NULL ) {
		new_noeud->element = ptr;
		new_noeud->suivant = NULL;
		new_noeud->prec = NULL;
		liste->tete = new_noeud;
		liste->queue = new_noeud;
		liste->taille = 1;

		return liste;
	}

	new_noeud->element = ptr;
	new_noeud->suivant = NULL;
	new_noeud->prec = liste->queue;
	temp = liste->tete;
	while (temp->suivant != NULL ) {
		temp = temp->suivant;
	}
	temp->suivant = new_noeud;
	liste->queue = new_noeud;
	liste->taille++;

	return liste;
}

// Mise à jour de la liste
t_liste liste_update(t_liste liste, int position, void * ptr) {
	assert(liste);
	assert(ptr);
	t_noeud noeud = malloc(sizeof(*noeud));
	assert(noeud);
	noeud = liste->tete;
	int i;
	for (i = 1; i < position; i++)
		noeud = noeud->suivant;

	noeud->element = ptr;

	return liste;
}

// Affichage de la liste
void liste_display(t_liste liste) {
	assert(liste);
	t_noeud noeud = malloc(sizeof(*noeud));
	assert(noeud);
	noeud = liste->tete;

	printf("\n=== Affichage de la liste  === \n");
	printf("taille %d\n", liste->taille);
	printf("liste %p\n", liste);
	printf("tete %p\n", liste->tete);
	printf("queue %p\n", liste->queue);

	while (noeud != NULL ) {
		printf("\n");
		printf("noeud %p\n", noeud);
		printf("\tprec %p\n", noeud->prec);
		printf("\tsuivant %p\n", noeud->suivant);
		printf("\telem %p\n", noeud->element);
		noeud = noeud->suivant;
	}
	printf("========= FIN =========\n\n");
}

void * liste_get_value(t_liste liste, int position) {
	assert(liste);
	t_noeud noeud = malloc(sizeof(*noeud));
	assert(noeud);
	noeud = liste->tete;
	int i;
	for (i = 1; i < position; i++)
		noeud = noeud->suivant;

	return noeud->element;
}

void * liste_get_queue_value(t_liste liste) {
	assert(liste);
	return liste->queue->element;
}

void liste_display_element(t_liste liste) {
	assert(liste);
	int taille = liste->taille;
	t_noeud suivant;
	t_noeud curr = liste->tete;
	while (taille--) {
		suivant = curr->suivant;
		if (liste->display)
			liste->display(curr->element);
		curr = suivant;
	}
}

void liste_free(t_liste liste) {
	assert(liste);
	int taille = liste->taille;
	t_noeud suivant;
	t_noeud curr = liste->tete;
	while (taille--) {
		suivant = curr->suivant;
		if (liste->free)
			liste->free(curr->element);
		free(curr);
		curr = suivant;
	}
	free(liste);

}

int liste_get_taille(t_liste liste) {
	assert(liste);
	return liste->taille;
}

void liste_free_element_tete(t_liste liste) {
	assert(liste);
	t_noeud temp;
	temp = liste->tete->suivant; // deuxième éléement de la liste
	temp->prec = NULL;
	liste->free(liste->tete->element);
	liste->tete = temp;
	liste->taille--;

}

void liste_free_element_queue(t_liste liste) {
	assert(liste);
	t_noeud temp;
	temp = liste->queue->prec;
	temp->suivant = NULL;
	liste->free(liste->queue->element);
	liste->queue = temp;
	liste->taille--;
}

void liste_free_element(t_liste liste, int position) {
	assert(liste);
	if (liste->taille < 1 || position < 1 || position > liste->taille) // gestion de erreur concernant la position
			{
		printf(
				"[ERREUR] liste_free_element : taille de la liste ou position\n");
		return;
	}
	if (liste->taille == 1) // free l'unique élément de la liste
			{
		liste->free(liste->tete->element);
		liste->tete = NULL;
		liste->queue = NULL;
		liste->taille = 0;
		return;
	}
	if (position == 1) // free la tete
			{
		liste_free_element_tete(liste);
		return;
	}
	if (position == liste->taille) // free la queue
			{
		liste_free_element_queue(liste);
		return;
	}
	// Dans les autres cas
	t_noeud temp;
	temp = liste->tete;
	int i;
	for (i = 1; i < position; i++)
		temp = temp->suivant;
	temp->suivant->prec = temp->prec;
	temp->prec->suivant = temp->suivant;
	liste->free(temp->element);
	free(temp);
	liste->taille--;
}

//-------------------------------------------------------------------------------------------------
// Fin du fichier
