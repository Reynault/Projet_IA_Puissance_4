#ifndef IA_HEADER
#define IA_HEADER

/*
    Header de d�finition des structures et define
    pour le script concernant l'ia
*/

#include <math.h>
#include <string.h>
#define C sqrt(2)

// Definition du type Noeud
typedef struct NoeudSt {

	int joueur; // joueur qui a jou� pour arriver ici
	Coup * coup;   // coup jou� par ce joueur pour arriver ici

	Etat * etat; // etat du jeu

	struct NoeudSt * parent;
	struct NoeudSt * enfants[LARGEUR_MAX]; // liste d'enfants : chaque enfant correspond � un coup possible
	int nb_enfants;	// nb d'enfants pr�sents dans la liste

	// POUR MCTS:
	int nb_victoires;
	int nb_simus;
    
    int estParcouru; // bool�en permettant d'indiquer si le noeud a d�j� �t� parcouru ou non (0 = non, 1 = oui)

} Noeud;

// D�finition des m�thodes
void freeNoeud ( Noeud * noeud);
void ordijoue_mcts(Etat * etat, int tempsmax);
void remonterValeurVersRacine(Noeud * noeud, FinDePartie resultat);
FinDePartie effectuerMarcheAleatoire(Noeud * noeud);
FinDePartie effectuerMarchePseudoAleatoire(Noeud * noeud);
Noeud * ajouterEnfant(Noeud * parent, Coup * coup);
Noeud * nouveauNoeud (Noeud * parent, Coup * coup );
Noeud * getNoeudPrioritaire(Noeud * noeud);
Noeud * getMeilleurNoeud(Noeud * noeud);
double getBValeur(Noeud * noeud);
int creationFils(Noeud * noeud);

#endif
