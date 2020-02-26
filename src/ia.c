#include "jeu.h"
#include "ia.h"

/*
 * Cr�er un nouveau noeud en jouant un coup � partir d'un parent
 * utiliser nouveauNoeud(NULL, NULL) pour cr�er la racine
 */

int victHumain , victOrdi;

Noeud * nouveauNoeud (Noeud * parent, Coup * coup ) {
	Noeud * noeud = (Noeud *)malloc(sizeof(Noeud));

	if ( parent != NULL && coup != NULL ) {
		noeud->etat = copieEtat ( parent->etat );
		jouerCoup ( noeud->etat, coup );
		noeud->coup = coup;
		noeud->joueur = AUTRE_JOUEUR(parent->joueur);
	} else {
		noeud->etat = NULL;
		noeud->coup = NULL;
		noeud->joueur = 0;
	}
	noeud->parent = parent;
	noeud->nb_enfants = 0;

	// POUR MCTS:
	noeud->nb_victoires = 0;
	noeud->nb_simus = 0;
    noeud->estParcouru = 0;

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
 * Fonction qui cr�er les fils d'un noeud
 *
 * @param noeud, dont il faut cr�er les fils, il faut que le noeud n'ait pas de fils
 */
int creationFils(Noeud * noeud){
    Coup ** coups;
    coups = coups_possibles(noeud->etat);
    // Parcours de tous les coups possibles et ajout de chaque
    int k = 0;
    while ( coups[k] != NULL) {
        ajouterEnfant(noeud, coups[k]);
        k++;
    }
    // Lib�ration coups (tableau allou� qui contient les coups possibles)
    free(coups);
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
	int temps = 0;

	// cr�er l'arbre de recherche
	Noeud * racine = nouveauNoeud(NULL, NULL);
    Noeud * courant;
    FinDePartie resultat;

    // initialisation de l'�tat
	racine->etat = copieEtat(etat);

    // impl�mentation de l'algorithme MCTS-UCT pour 
    // d�terminer le meilleur coup ci-dessous
	int iteration = 0;
    int noeudNonExploree;
    victHumain = 0;
    victOrdi = 0;

//    printf("parcours arbre\n");
	do {
//        printf("parcours noeud\n");
        // initiatlisation des valeurs de base
        noeudNonExploree = 0;
        courant = racine;

        // parcours de l'arbre jusqu'� trouver un noeud non parcouru, ou � arriver � la fin du jeu, ou arriver
        // � la fin de l'arbre
        do{
//            printf("est parcouru\n");
            // cr�ation des fils si le noeud n'a pas d�j� �t� parcouru
            if(courant->estParcouru == 0){
                creationFils(courant);
                // le noeud courant est alors parcouru
                courant->estParcouru = 1;
            }

            // r�cup�ration du fils prioritaire pour le noeud courant
            // si le noeud courant peut encore mener � des nouvelles configurations (coups possibles > 0)
            if(courant->nb_enfants > 0) {
//                printf("enfant > 0 donc recup noeud prioritaire\n");
                courant = getNoeudPrioritaire(courant);

                // si celui-ci n'a pas encore �t� parcouru on sort de la boucle.
                if (courant->estParcouru == 0) {
//                    printf("noeud non explore\n");
                    noeudNonExploree = 1;
                }
            }
        }while( ((resultat = testFin(courant->etat)) == NON && noeudNonExploree == 0 )|| courant->nb_victoires > 0);

        // si on s'arr�te � cause d'un noeud non explor�
        if(noeudNonExploree == 1){
            // lancement d'une marche al�atoire
            //printf("marche aleatoire\n");
            if(etat->version == 1) {
                resultat = effectuerMarchePseudoAleatoire(courant);
            }else{
                resultat = effectuerMarcheAleatoire(courant);
            }
            //printf("creation fils\n");
            creationFils(courant);
            courant->estParcouru = 1; // et mise � jour du noeud explor�
        }

        // on remonte les valeurs vers la racine en mettant � jour les noeuds
        if(resultat != NON) {
            //printf("remonter valeurs\n");
            remonterValeurVersRacine(courant, resultat);
        }

        // r�cup�ration du temps pour v�rification
		toc = clock();
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iteration ++;
	} while ( temps < tempsmax );

	// Jouer le meilleur premier coup
    //printf("recup meilleur coup\n");
    Noeud * meilleurNoeud = getMeilleurNoeud(racine);
	jouerCoup(etat, meilleurNoeud->coup);

    // affichage des informations concernant le nombre de simulations
    printf("\nNombre de simulations : %d\n", iteration);
    //affichage du pourcentage de victoire de l'ordinateur lors des simulations
    double pv = (double )victOrdi * 100 / (victOrdi + victHumain);
    printf("\nPourcentage de victoire pour l'ordinateur : %.2f %%\n" , pv);
    //affichage du pourcentage de victoire de l'humain lors des simulations
    pv = (double)victHumain * 100 / (victOrdi + victHumain);
    printf("\nPourcentage de victoire pour l'humain : %.2f %%\n" , pv);

    // et l'estimation de la probabilit� de victoire
    printf("\nEstimation de la probabilite de victoire pour l'ordinateur : %f\n", (float)(meilleurNoeud->nb_victoires) / (float)(meilleurNoeud->nb_simus));

	// Penser � lib�rer la m�moire :
	freeNoeud(racine);
}

/*
 * Fonction qui permet de mettre � jour les valeurs
 * en partant d'un noeud
 * @param noeud � partir duquel on remonte les valeurs
 */
void remonterValeurVersRacine(Noeud * noeud, FinDePartie resultat){
    Noeud * courant = noeud;
    while(courant != NULL){
        courant->nb_simus++;
        if(resultat == ORDI_GAGNE){
            courant->nb_victoires++;
        }
        courant = courant->parent;
    }
}

/*
 * Fonction qui permet de r�cup�rer le noeud
 * prioritaire parmis les fils du noeud pass� en param�tre.
 * 
 * Sachant que si un noeud parmis les fils n'a pas �t� explor�s, on r�cup�re
 * un tableau dans lequel on en prend un al�atoirement
 * 
 * @param noeud, le noeud duquel on veut r�cup�rer le fils prioritaire (noeud qui poss�de des enfants)
 * @return le noeud le plus prioritaire
 */
Noeud * getNoeudPrioritaire(Noeud * noeud){
    Noeud * prioritaire = noeud;
    Noeud ** enfants = noeud->enfants;
    int nb_enfants = noeud->nb_enfants;
    
    int indices_pas_parcourus[nb_enfants];  // indices des noeuds pas d�j� parcourus dans enfants
    int nb_pas_parcourus = 0;
    double b_valeur_courante, max = LONG_MIN;

    // v�rification de la pr�sence de noeuds non parcourus
    for(int i = 0; i < nb_enfants; i++){
        // si pas parcouru trouv�
        if(enfants[i]->estParcouru == 0){
            // ajout dans tableau
            indices_pas_parcourus[nb_pas_parcourus] = i;
            nb_pas_parcourus ++;
        }else{
            // sinon, mise � jour de max
            b_valeur_courante = getBValeur(enfants[i]);
            if(b_valeur_courante > max){
                prioritaire = enfants[i];
                max = b_valeur_courante;
            }
        }
    }
    
    // si il y a des noeuds non parcourus
    if(nb_pas_parcourus > 0){
        // on retourne l'un d'entre eux al�atoirement
        prioritaire = enfants[
            indices_pas_parcourus[rand()%nb_pas_parcourus]
        ];
    }
    
    return prioritaire;
}

/*
 * Fonction qui permet de r�cup�rer le meilleur coup
 * � partir d'un noeud racine. Donc parcours des noeuds
 * enfants, et r�cup�ration du coup de celui qui maximise
 * le crit�re voulu.
 * 
 * Crit�re courant : maximisation de la moyenne des r�compenses obtenus ( nb_victoires / nb_simus )
 * 
 * @param noeud, noeud � partir duquel on cherche le meilleur coup (parmis ceux de ses fils)
 * @return le meilleur coup
 */
Noeud * getMeilleurNoeud(Noeud * noeud){
    float valeurCourante, max = INT_MIN;
    int nb_enfants = noeud->nb_enfants;
    Noeud * meilleurNoeud = NULL;
    Noeud * enfant;
    
    // r�cup�ration du meilleur coup parmis les enfants du noeud fourni
    for(int i = 0; i < nb_enfants; i++){
        enfant = noeud->enfants[i];
        // les coups possibles sont ceux d'un enfant qui a �t� parcouru
        if(enfant->estParcouru == 1){
            valeurCourante = (float)(enfant->nb_victoires) / (float)(enfant->nb_simus);
            if(valeurCourante > max){
                max = valeurCourante;
                meilleurNoeud = enfant;
            }
        }
    }
    
    // si le meilleur coup n'a pas �t� trouv�, cela signifie qu'il n'y a pas
    // de fils qui a �t� explor�, on prend alors un coup al�atoire
    if(meilleurNoeud == NULL){
        meilleurNoeud = noeud->enfants[rand()%nb_enfants];
    }
    
    return meilleurNoeud;
}

/*
 * Fonction qui permet d'effectuer la marche al�atoire, elle va parcourir al�atoirement
 * l'arbre en partant d'un noeud. On copie l'�tat actuel dans un autre �tat qui va nous servir
 * � parcourir l'arbre sans g�n�rer les fils. (juste un �tat � modifier)
 * 
 * @param noeud, � partir duquel on r�alise la marche al�atoire
 * @return fin de partie trouv�e
 */
FinDePartie effectuerMarcheAleatoire(Noeud * noeud){
    Etat * etatCourant;
    Coup ** coups;
    FinDePartie estFini;
    int nbCoups;

    // copie du bloc m�moire contenant l'�tat du noeud
    etatCourant = copieEtat(noeud->etat);

    // tant qu'on arrive pas � la fin
    while((estFini = testFin(etatCourant)) == NON){
        // r�cup�ration d'un mouvement al�atoire parmi les coups possibles
        coups = coups_possibles(etatCourant);
        nbCoups = 0;
        while ( coups[nbCoups] != NULL) {
            nbCoups++;
        }
        // modification du noeud courant
        jouerCoup(etatCourant, coups[rand()%nbCoups]);
        // lib�ration de la liste des coups
        free(coups);
    }

    // lib�ration de l'�tat
    free(etatCourant);

    if (testFin(etatCourant) == ORDI_GAGNE){
        victOrdi++;
    }else{
        victHumain++;
    }


    return estFini;
}



FinDePartie effectuerMarchePseudoAleatoire(Noeud * noeud){
    Etat * etatCourant;
    Coup ** coups;
    FinDePartie estFini;
    int nbCoups;

    // copie du bloc m�moire contenant l'�tat du noeud
    etatCourant = copieEtat(noeud->etat);

    // tant qu'on arrive pas � la fin
    while((estFini = testFin(etatCourant)) == NON){
        // r�cup�ration d'un mouvement al�atoire parmi les coups possibles
        coups = coups_possibles(etatCourant);
        nbCoups = 0;
        while ( coups[nbCoups] != NULL) {
            nbCoups++;
        }
        // modification du noeud courant

        int test = 0;
        int estGagnant = 0;

        //printf("nbcoups = %d \n", nbCoups);
        do {
            Etat * e = copieEtat(etatCourant);
            jouerCoup(e, coups[test]);
            if (testFin(e) == ORDI_GAGNE && e->joueur == JOUEUR_ORDI){
                estGagnant = 1;
                break;
            }
            test++;
        }while (test < nbCoups);
        if (estGagnant == 0){
            jouerCoup(etatCourant, coups[rand()%nbCoups]);
        } else{
            jouerCoup(etatCourant, coups[test]);
        }

        //printf("no\n");

        // lib�ration de la liste des coups
        free(coups);
    }

    // lib�ration de l'�tat
    free(etatCourant);

     //printf("before test \n");
    if (testFin(etatCourant) == ORDI_GAGNE){
        victOrdi++;
    }else{
        victHumain++;
    }

   //printf("tested \n");

    return estFini;
}





/*
 * Fonction qui calcule la B-valeur du noeud fourni.
 * 
 * Si le noeud fourni n'a pas de parent o� n'est pas d�j� explor�,
 * on renvoit une b_valeur nulle.
 * 
 * @param noeud, noeud fourni
 * @return la B-valeur
 */
double getBValeur(Noeud * noeud){
    double moyenne_recompense, exploration, b_valeur = 0;

    // si le noeud a d�j� �t� parcouru et qu'il n'est pas la racine
    if(noeud->estParcouru == 1 && noeud->parent != NULL){
        // calcul de la moyenne de la r�compense (nb_victoires / nb_simus) (exploitation)
        moyenne_recompense = (double)noeud->nb_victoires / (double)noeud->nb_simus;
        
        // calcul de l'exploration
        exploration = sqrt(log(noeud->parent->nb_simus)/noeud->nb_simus);

        // inversion du signe en fonction du joueur
        if(noeud->joueur == JOUEUR_HUMAIN){
            moyenne_recompense = -moyenne_recompense;
        }

        // calcul de la b_valeur en utilisant la constante C
        b_valeur = moyenne_recompense + C * exploration;
    }
    
    return b_valeur;
}
