#ifndef IA_HEADER
#define IA_HEADER

/*
    Header de définition des structures et define
    pour le script concernant l'ia
*/

// Definition du type Noeud
typedef struct NoeudSt {

	int joueur; // joueur qui a joué pour arriver ici
	Coup * coup;   // coup joué par ce joueur pour arriver ici

	Etat * etat; // etat du jeu

	struct NoeudSt * parent;
	struct NoeudSt * enfants[LARGEUR_MAX]; // liste d'enfants : chaque enfant correspond à un coup possible
	int nb_enfants;	// nb d'enfants présents dans la liste

	// POUR MCTS:
	int nb_victoires;
	int nb_simus;

} Noeud;

// Définition des méthodes
void freeNoeud ( Noeud * noeud);
Noeud * ajouterEnfant(Noeud * parent, Coup * coup);
Noeud * nouveauNoeud (Noeud * parent, Coup * coup );
void ordijoue_mcts(Etat * etat, int tempsmax);

#endif
