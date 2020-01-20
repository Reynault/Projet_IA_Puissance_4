#include "jeu.h"
#include "ia.h"

/*
 * Cr�er un nouveau noeud en jouant un coup � partir d'un parent
 * utiliser nouveauNoeud(NULL, NULL) pour cr�er la racine
 */
Noeud * nouveauNoeud (Noeud * parent, Coup * coup ) {
	Noeud * noeud = (Noeud *)malloc(sizeof(Noeud));

	if ( parent != NULL && coup != NULL ) {
		noeud->etat = copieEtat ( parent->etat );
		jouerCoup ( noeud->etat, coup );
		noeud->coup = coup;
		noeud->joueur = AUTRE_JOUEUR(parent->joueur);
	}
	else {
		noeud->etat = NULL;
		noeud->coup = NULL;
		noeud->joueur = 0;
	}
	noeud->parent = parent;
	noeud->nb_enfants = 0;

	// POUR MCTS:
	noeud->nb_victoires = 0;
	noeud->nb_simus = 0;


	return noeud;
}

/*
 * Ajouter un enfant � un parent en jouant un coup
 * retourne le pointeur sur l'enfant ajout�
 */
Noeud * ajouterEnfant(Noeud * parent, Coup * coup) {
	Noeud * enfant = nouveauNoeud (parent, coup ) ;
	parent->enfants[parent->nb_enfants] = enfant;
	parent->nb_enfants++;
	return enfant;
}

/*
 * M�thode de lib�ration des noeuds inutilis�s
 * ainsi que des ces enfants
 */
void freeNoeud ( Noeud * noeud) {
	if ( noeud->etat != NULL)
		free (noeud->etat);

	while ( noeud->nb_enfants > 0 ) {
		freeNoeud(noeud->enfants[noeud->nb_enfants-1]);
		noeud->nb_enfants --;
	}
	if ( noeud->coup != NULL)
		free(noeud->coup);

	free(noeud);
}

/*
 * Calcule et joue un coup de l'ordinateur avec MCTS-UCT
 * en tempsmax secondes
 */
void ordijoue_mcts(Etat * etat, int tempsmax) {

	clock_t tic, toc;
	tic = clock();
	int temps;

	Coup ** coups;
	Coup * meilleur_coup ;

	// Cr�er l'arbre de recherche
	Noeud * racine = nouveauNoeud(NULL, NULL);
	racine->etat = copieEtat(etat);

	// cr�er les premiers noeuds:
	coups = coups_possibles(racine->etat);
	int k = 0;
	Noeud * enfant;
	while ( coups[k] != NULL) {
		enfant = ajouterEnfant(racine, coups[k]);
		k++;
	}


	meilleur_coup = coups[ rand()%k ]; // choix al�atoire

	/*  TODO :
		- supprimer la s�lection al�atoire du meilleur coup ci-dessus
		- impl�menter l'algorithme MCTS-UCT pour d�terminer le meilleur coup ci-dessous

	int iter = 0;

	do {



		// � compl�ter par l'algorithme MCTS-UCT...




		toc = clock();
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iter ++;
	} while ( temps < tempsmax );

	/* fin de l'algorithme  */

	// Jouer le meilleur premier coup
	jouerCoup(etat, meilleur_coup );

	// Penser � lib�rer la m�moire :
	freeNoeud(racine);
	free (coups);
}
