#ifndef JEU_HEADER
#define JEU_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>

/*
	Header du script principal:

        - Définition des constantes de paramètrage du jeu
        - Macros
        - Enumérations et structures
*/

// Paramètres du jeu
#define NB_COLONNE  7   // Taille du plateau
#define NB_LIGNE    6

#define TAILLE_POUR_GAGNER  4 // Taille de la suite gagnante

#define VIDE        ' '
#define PION_1      'O'
#define PION_0      'X'

#define LARGEUR_MAX NB_COLONNE 		// nb max de fils pour un noeud (= nb max de coups possibles)

#define TEMPS 5		// temps de calcul pour un coup avec MCTS (en secondes)

// macros
#define AUTRE_JOUEUR(i) (1-(i))
#define JOUEUR_HUMAIN   0
#define JOUEUR_ORDI     1
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))


// Critères de fin de partie
typedef enum {NON, MATCHNUL, ORDI_GAGNE, HUMAIN_GAGNE } FinDePartie;

// Definition du type Etat (état/position du jeu)
typedef struct EtatSt {
	int joueur; // à qui de jouer ?
	char plateau[NB_LIGNE][NB_COLONNE]; // plateau du puissance 4
	char nombre_pions[NB_COLONNE]; // tableau qui indique pour chaque colonne la hauteur courante
} Etat;

// Definition du type Coup
typedef struct {
    int colonne; // colonne choisie pour le mouvement
} Coup;

// Définition des méthodes
Coup ** coups_possibles( Etat * etat );
Coup * nouveauCoup( int colonne );
Coup * demanderCoup ();
Etat * copieEtat( Etat * src );
int jouerCoup( Etat * etat, Coup * coup );
FinDePartie testFin( Etat * etat );
void afficheJeu(Etat * etat);
Etat * etat_initial( void );
void lancerJeu();

#endif
