#include "jeu.h"
#include "ia.h"

/*
	Script contenant les méthodes nécéssaires au fonctionnement
	du jeu.

	joueur 0 : humain
	joueur 1 : ordinateur
*/

/*
 * Méthode de copie d'un état
 *
 * @param état à copier
 * @return copie de l'état
 */
Etat * copieEtat( Etat * src ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));

	etat->joueur = src->joueur;

	/* remplissage du plateau : */
	int i,j;
	for (i=0; i< NB_LIGNE; i++)
		for ( j=0; j<NB_COLONNE; j++)
			etat->plateau[i][j] = src->plateau[i][j];

    /* remplissage du tableau qui indique les colonnes */
    for(i=0; i< NB_COLONNE; i++){
        etat->nombre_pions[i] = src->nombre_pions[i];
    }

	return etat;
}

/*
 * Récupération de l'état initial du jeu.
 *
 * @return état initial
 */
Etat * etat_initial( void ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));

	/* remplissage du plateau : */
	int i,j;
	for (i=0; i< NB_LIGNE; i++)
		for ( j=0; j<NB_COLONNE; j++)
			etat->plateau[i][j] = VIDE;

	/* remplissage du tableau qui indique les colonnes : */
	for (i=0; i< NB_COLONNE; i++)
        etat->nombre_pions[i] = 0;

	return etat;
}

/*
 * Méthode d'affichage de l'état actuel du jeu
 *
 * @param etat, etat du jeu actuel
 */
void afficheJeu(Etat * etat) {
	/* affichage du plateau : */

	int i,j;
	printf("   |");
	for ( j = 0; j < NB_COLONNE; j++)
		printf(" %d |", j);
	printf("\n");
	printf("--------------------------------");
	printf("\n");

	for(i=(NB_LIGNE-1); i >= 0; i--) {
		printf(" %d |", i);
		for ( j = 0; j < NB_COLONNE; j++)
			printf(" %c |", etat->plateau[i][j]);
		printf("\n");
		printf("--------------------------------");
		printf("\n");
	}
}

/*
 * Méthode qui permet de récupérer un nouveau coup
 *
 * @param colonne, indication de la colonne sélectionnée
 * @return le nouveau coup proposé
 */
Coup * nouveauCoup( int colonne ) {
	Coup * coup = (Coup *)malloc(sizeof(Coup));

	/* numéro de la colonne: */
	coup->colonne = colonne;

	return coup;
}

/*
 * Méthode qui demande à l'humain quel est son prochain coup
 */
Coup * demanderCoup () {
	/* Demande du numéro de colonne : */
	int colonne;
	printf("\n quelle colonne ? ") ;
	scanf("%d",&colonne);

	return nouveauCoup(colonne);
}

/*
 * Modifier l'état en jouant un coup
 * @param etat, etat actuel du jeu, qui est donc modifié
 * @param coup, coup qui va modifier l'état du jeu
 * @return retourne 0 si le coup n'est pas possible
 */
int jouerCoup( Etat * etat, Coup * coup ) {
    // vérification si la colonne courante est pleine et si elle est dans le tableau
    if(coup->colonne < 0 || coup->colonne >= NB_COLONNE ||
       etat->nombre_pions[coup->colonne] == (NB_LIGNE)){
        return 0;
    }else{
        // sinon, mise à jour de l'état du jeu
        int i = 0;
        int trouve = 0;

        // récupération de l'emplacement vide, et modification
        while(i < NB_LIGNE && !trouve){
            if(etat->plateau[i][coup->colonne] == VIDE){
                etat->plateau[i][coup->colonne] = etat->joueur ? PION_1 : PION_0;
                etat->nombre_pions[coup->colonne] ++;
                etat->joueur = AUTRE_JOUEUR(etat->joueur);
                trouve ++;
            }else{
                i++;
            }
        }
        return 1;
    }
}

/*
 * Méthode qui permet de récupérer les coups possibles d'un état du jeu.
 *
 * @param etat, à partir duquel on récupère les coups possibles
 * @return Retourne une liste de coups possibles à partir d'un etat
 * (tableau de pointeurs de coups se terminant par NULL)
*/
Coup ** coups_possibles( Etat * etat ) {

    printf("creation des coups possibles\n");
	Coup ** coups = (Coup **) malloc((1+LARGEUR_MAX) * sizeof(Coup *) );

    // index qui permet de parcourir le tableau de pointeurs
	int k = 0;

    // pour chaque colonne, on regarde si on a atteint la hauteur max
    // si c'est pas le cas, ajout du nouveau coup
    int i;
    for(i = 0; i < NB_COLONNE; i++){
        if(etat->nombre_pions[i] < (NB_LIGNE)){
            coups[k] = nouveauCoup(i);
            k++;
        }
    }

    printf("fin de la creation des coups possibles\n");

	coups[k] = NULL;
	return coups;
}

/*
    Méthode de test si la partie est finie ou non.
*/
FinDePartie testFin( Etat * etat ) {

    // n étant le nombre de coups joués
    int i, j, k, n = 0;
    // caractère courant
    char courant;

    for(j = 0; j < NB_COLONNE; j++){
        if(etat->nombre_pions[j] > 0){
            for(i = 0; i < NB_LIGNE; i++){
                if(etat->plateau[i][j] != VIDE){
                    n++;

                    // test colonne
                    if(j >= (TAILLE_POUR_GAGNER - 1)){
                        k = 0;

                        courant = etat->plateau[i][j];
                        while( k < TAILLE_POUR_GAGNER && etat->plateau[i][j-k] == courant){
                                k++;
                        }

                        if(k == TAILLE_POUR_GAGNER){
                            return courant == PION_1? ORDI_GAGNE : HUMAIN_GAGNE;
                        }
                    }

                    // test ligne
                    if(i >= (TAILLE_POUR_GAGNER - 1)){
                        k = 0;

                        courant = etat->plateau[i][j];
                        while( k < TAILLE_POUR_GAGNER && etat->plateau[i-k][j] == courant){
                                k++;
                        }

                        if(k == TAILLE_POUR_GAGNER){
                            return courant == PION_1? ORDI_GAGNE : HUMAIN_GAGNE;
                        }
                    }

                    // diagonales

                    // vers la gauche
                    if(i >= (TAILLE_POUR_GAGNER - 1) && j >= (TAILLE_POUR_GAGNER - 1)){
                        k = 0;

                        courant = etat->plateau[i][j];
                        while( k < TAILLE_POUR_GAGNER && etat->plateau[i-k][j-k] == courant){
                                k++;
                        }

                        if(k == TAILLE_POUR_GAGNER){
                            return courant == PION_1? ORDI_GAGNE : HUMAIN_GAGNE;
                        }
                    }

                    // vers la droite
                    if(i+(TAILLE_POUR_GAGNER) < NB_LIGNE && j >= (TAILLE_POUR_GAGNER - 1)){
                        k = 0;

                        courant = etat->plateau[i][j];
                        while( k < TAILLE_POUR_GAGNER && etat->plateau[i+k][j-k] == courant){
                                k++;
                        }

                        if(k == TAILLE_POUR_GAGNER){
                            return courant == PION_1? ORDI_GAGNE : HUMAIN_GAGNE;
                        }
                    }
                }
            }
        }
    }

	// et sinon tester le match nul
	return ( n == (NB_COLONNE * NB_LIGNE) ) ? MATCHNUL : NON;
}

/*
 * Méthode qui contient la boucle de jeu
 *
 * @return 0
 */
void lancerJeu(){
    Coup * coup;
	FinDePartie fin;

	// initialisation
    srand(time(0));
	Etat * etat = etat_initial();

	// Choisir qui commence :
	printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
	scanf("%d", &(etat->joueur) );

	// boucle de jeu
	do {
		if ( etat->joueur == JOUEUR_HUMAIN ) {
		    // tour de l'humain
            printf("\n");
            afficheJeu(etat);
			do {
				coup = demanderCoup();
			} while ( !jouerCoup(etat, coup) );
            printf("\n");
            afficheJeu(etat);
		}else {
		    // tour de l'Ordinateur
			ordijoue_mcts( etat, TEMPS );
		}
		fin = testFin( etat );
	} while ( fin == NON ) ;

	if ( fin == ORDI_GAGNE )
		printf( "\n** Victoire de l'ordinateur, vous avez perdu ! **\n");
	else if ( fin == MATCHNUL )
		printf("\n Match nul !  \n");
	else
		printf( "\n** BRAVO, vous avez battu l'ordinateur. **\n");
}

int main(void) {
    lancerJeu();
    return 0;
}
