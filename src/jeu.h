#ifndef JEU_HEADER
#define JEU_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>

/*
	Header du script principal:

        - D�finition des constantes de param�trage du jeu
        - Macros
        - Enum�rations et structures
*/

// Param�tres du jeu
#define NB_COLONNE  7   // Taille du plateau
#define NB_LIGNE    6

#define TAILLE_POUR_GAGNER  4 // Taille de la suite gagnante

#define VIDE        ' '
#define PION_1      'O'
#define PION_0      'X'

#define LARGEUR_MAX NB_COLONNE 		// nb max de fils pour un noeud (= nb max de coups possibles)

#define TEMPS 1		// temps de calcul pour un coup avec MCTS (en secondes)

// macros
#define AUTRE_JOUEUR(i) (1-(i))
#define JOUEUR_HUMAIN   0
#define JOUEUR_ORDI     1
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))


// Crit�res de fin de partie
typedef enum {NON, MATCHNUL, ORDI_GAGNE, HUMAIN_GAGNE } FinDePartie;

// Definition du type Etat (�tat/position du jeu)
typedef struct EtatSt {
	int joueur; // � qui de jouer ?
	char plateau[NB_LIGNE][NB_COLONNE]; // plateau du puissance 4
	char nombre_pions[NB_COLONNE]; // tableau qui indique pour chaque colonne la hauteur courante
    int version; // 1 : marchePseudoAleatoire (question 3); autre sinon

} Etat;


// Definition du type Coup
typedef struct {
    int colonne; // colonne choisie pour le mouvement
} Coup;

// D�finition des m�thodes
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
