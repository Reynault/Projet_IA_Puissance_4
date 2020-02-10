#ifndef IA_HEADER
#define IA_HEADER

/*
    Header de définition des structures et define
    pour le script concernant l'ia
*/

#include <math.h>
#include <string.h>
#define C sqrt(2)

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
    
    int estParcouru; // booléen permettant d'indiquer si le noeud a déjà été parcouru ou non (0 = non, 1 = oui)

} Noeud;

// Définition des méthodes
void freeNoeud ( Noeud * noeud);
void ordijoue_mcts(Etat * etat, int tempsmax);
Noeud * effectuerMarcheAleatoire(Noeud * noeud);
Noeud * ajouterEnfant(Noeud * parent, Coup * coup);
Noeud * nouveauNoeud (Noeud * parent, Coup * coup );
Noeud * getNoeudPrioritaire(Noeud * noeud);
Coup * getMeilleurCoup(Noeud * noeud);
double getBValeur(Noeud * noeud);
int creationFils(Noeud * noeud);

#endif
