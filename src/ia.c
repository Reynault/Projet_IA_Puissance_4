#include "jeu.h"
#include "ia.h"

/*
 * Creer un nouveau noeud en jouant un coup a partir d'un parent
 * utiliser nouveauNoeud(NULL, NULL) pour creer la racine
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
 * Ajouter un enfant a un parent en jouant un coup
 * retourne le pointeur sur l'enfant ajoute
 */
Noeud * ajouterEnfant(Noeud * parent, Coup * coup) {
	Noeud * enfant = nouveauNoeud (parent, coup ) ;
	parent->enfants[parent->nb_enfants] = enfant;
	parent->nb_enfants++;
	return enfant;
}

/*
 * Fonction qui creer les fils d'un noeud
 *
 * @param noeud, dont il faut creer les fils, il faut que le noeud n'ait pas de fils
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
    // Liberation coups (tableau alloue qui contient les coups possibles)
    free(coups);
}

/*
 * Methode de liberation des noeuds inutilises
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

	// creer l'arbre de recherche
	Noeud * racine = nouveauNoeud(NULL, NULL);
    Noeud * courant;
    FinDePartie resultat;

    // initialisation de l'etat
	racine->etat = copieEtat(etat);

    // implementation de l'algorithme MCTS-UCT pour
    // determiner le meilleur coup ci-dessous
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

        // parcours de l'arbre jusqu'a trouver un noeud non parcouru, ou a arriver a la fin du jeu, ou arriver
        // a la fin de l'arbre
        do{
//            printf("est parcouru\n");
            // creation des fils si le noeud n'a pas deja ete parcouru
            if(courant->estParcouru == 0){
                creationFils(courant);
                // le noeud courant est alors parcouru
                courant->estParcouru = 1;
            }

            // recuperation du fils prioritaire pour le noeud courant
            // si le noeud courant peut encore mener a des nouvelles configurations (coups possibles > 0)
            if(courant->nb_enfants > 0) {
//                printf("enfant > 0 donc recup noeud prioritaire\n");
                courant = getNoeudPrioritaire(courant);

                // si celui-ci n'a pas encore ete parcouru on sort de la boucle.
                if (courant->estParcouru == 0) {
//                    printf("noeud non explore\n");
                    noeudNonExploree = 1;
                }
            }
        }while( ((resultat = testFin(courant->etat)) == NON && noeudNonExploree == 0 )|| courant->nb_victoires > 0);

        // si on s'arrete a cause d'un noeud non explore
        if(noeudNonExploree == 1){
            // lancement d'une marche aleatoire
            //printf("marche aleatoire\n");
            if(etat->version == 1) {
                resultat = effectuerMarchePseudoAleatoire(courant);
            }else{
                resultat = effectuerMarcheAleatoire(courant);
            }
            //printf("creation fils\n");
            creationFils(courant);
            courant->estParcouru = 1; // et mise a jour du noeud explore
        }

        // on remonte les valeurs vers la racine en mettant a jour les noeuds
        if(resultat != NON) {
            //printf("remonter valeurs\n");
            remonterValeurVersRacine(courant, resultat);
        }

        // recuperation du temps pour verification
		toc = clock();
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iteration ++;
	} while ( temps < tempsmax );

	// Jouer le meilleur premier coup
    //printf("recup meilleur coup\n");
    Noeud * meilleurNoeud = getMeilleurNoeud(racine , etat->max);
	jouerCoup(etat, meilleurNoeud->coup);

    // affichage des informations concernant le nombre de simulations
    printf("\nNombre de simulations : %d\n", iteration);
    //affichage du pourcentage de victoire de l'ordinateur lors des simulations
    double pv = (double )victOrdi * 100 / (victOrdi + victHumain);
    printf("\nPourcentage de victoire pour l'ordinateur : %.2f %%\n" , pv);
    //affichage du pourcentage de victoire de l'humain lors des simulations
    pv = (double)victHumain * 100 / (victOrdi + victHumain);
    printf("\nPourcentage de victoire pour l'humain : %.2f %%\n" , pv);

    // et l'estimation de la probabilite de victoire
    printf("\nEstimation de la probabilite de victoire pour l'ordinateur : %f\n", (float)(meilleurNoeud->nb_victoires) / (float)(meilleurNoeud->nb_simus));

	// Penser a liberer la memoire :
	freeNoeud(racine);
}

/*
 * Fonction qui permet de mettre a jour les valeurs
 * en partant d'un noeud
 * @param noeud e partir duquel on remonte les valeurs
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
 * Fonction qui permet de recuperer le noeud
 * prioritaire parmis les fils du noeud passe en parametre.
 * 
 * Sachant que si un noeud parmis les fils n'a pas ete explores, on recupere
 * un tableau dans lequel on en prend un aleatoirement
 * 
 * @param noeud, le noeud duquel on veut recuperer le fils prioritaire (noeud qui possede des enfants)
 * @return le noeud le plus prioritaire
 */
Noeud * getNoeudPrioritaire(Noeud * noeud){
    Noeud * prioritaire = noeud;
    Noeud ** enfants = noeud->enfants;
    int nb_enfants = noeud->nb_enfants;
    
    int indices_pas_parcourus[nb_enfants];  // indices des noeuds pas deja parcourus dans enfants
    int nb_pas_parcourus = 0;
    double b_valeur_courante, max = LONG_MIN;

    // verification de la presence de noeuds non parcourus
    for(int i = 0; i < nb_enfants; i++){
        // si pas parcouru trouve
        if(enfants[i]->estParcouru == 0){
            // ajout dans tableau
            indices_pas_parcourus[nb_pas_parcourus] = i;
            nb_pas_parcourus ++;
        }else{
            // sinon, mise a jour de max
            b_valeur_courante = getBValeur(enfants[i]);
            if(b_valeur_courante > max){
                prioritaire = enfants[i];
                max = b_valeur_courante;
            }
        }
    }
    
    // si il y a des noeuds non parcourus
    if(nb_pas_parcourus > 0){
        // on retourne l'un d'entre eux aleatoirement
        prioritaire = enfants[
            indices_pas_parcourus[rand()%nb_pas_parcourus]
        ];
    }
    
    return prioritaire;
}

/*
 * Fonction qui permet de recuperer le meilleur coup
 * a partir d'un noeud racine. Donc parcours des noeuds
 * enfants, et recuperation du coup de celui qui maximise
 * le critere voulu.
 * 
 * Critere courant : maximisation de la moyenne des recompenses obtenus ( nb_victoires / nb_simus )
 * 
 * @param noeud, noeud a partir duquel on cherche le meilleur coup (parmis ceux de ses fils)
 * @param isMax, determine quelle critére le programme utilise (max ou robuste)
 * @return le meilleur coup
 */
Noeud * getMeilleurNoeud(Noeud * noeud , int isMax){
    float valeurCourante, max = INT_MIN;
    int nb_enfants = noeud->nb_enfants;
    Noeud * meilleurNoeud = NULL;
    Noeud * enfant;
    
    // recuperation du meilleur coup parmis les enfants du noeud fourni
    for(int i = 0; i < nb_enfants; i++){
        enfant = noeud->enfants[i];
        // les coups possibles sont ceux d'un enfant qui a ete parcouru
        if(enfant->estParcouru == 1){
            if (isMax == 1) {
                valeurCourante = (float)(enfant->nb_victoires) / (float)(enfant->nb_simus);
                if (valeurCourante > max) {
                    max = valeurCourante;
                    meilleurNoeud = enfant;
                }
            }else{
                if (enfant->nb_simus > max) {
                    max = enfant->nb_simus;
                    meilleurNoeud = enfant;
                }
            }
        }
    }
    
    // si le meilleur coup n'a pas ete trouve, cela signifie qu'il n'y a pas
    // de fils qui a ete explore, on prend alors un coup aleatoire
    if(meilleurNoeud == NULL){
        meilleurNoeud = noeud->enfants[rand()%nb_enfants];
    }
    
    return meilleurNoeud;
}

/*
 * Fonction qui permet d'effectuer la marche aleatoire, elle va parcourir aleatoirement
 * l'arbre en partant d'un noeud. On copie l'etat actuel dans un autre etat qui va nous servir
 * a parcourir l'arbre sans generer les fils. (juste un etat a modifier)
 * 
 * @param noeud, a partir duquel on realise la marche aleatoire
 * @return fin de partie trouvee
 */
FinDePartie effectuerMarcheAleatoire(Noeud * noeud){
    Etat * etatCourant;
    Coup ** coups;
    FinDePartie estFini;
    int nbCoups;

    // copie du bloc memoire contenant l'etat du noeud
    etatCourant = copieEtat(noeud->etat);

    // tant qu'on arrive pas a la fin
    while((estFini = testFin(etatCourant)) == NON){
        // recupuration d'un mouvement aleatoire parmi les coups possibles
        coups = coups_possibles(etatCourant);
        nbCoups = 0;
        while ( coups[nbCoups] != NULL) {
            nbCoups++;
        }
        // modification du noeud courant
        jouerCoup(etatCourant, coups[rand()%nbCoups]);
        // liberation de la liste des coups
        free(coups);
    }

    // liberation de l'etat
    free(etatCourant);

    if (testFin(etatCourant) == ORDI_GAGNE){
        victOrdi++;
    }else{
        victHumain++;
    }


    return estFini;
}

/*
 * fonction effectuerMarcheAleatoire ameliore
 * aleatoire inteligent (conctroler)
 * @param noeud, a partir duquel on realise la marche aleatoire
 * @return fin de partie trouvee
 */


FinDePartie effectuerMarchePseudoAleatoire(Noeud * noeud){
    Etat * etatCourant;
    Coup ** coups;
    FinDePartie estFini;
    int nbCoups;

    // copie du bloc memoire contenant l'etat du noeud
    etatCourant = copieEtat(noeud->etat);

    // tant qu'on arrive pas a la fin
    while((estFini = testFin(etatCourant)) == NON){
        // recuperation d'un mouvement aleatoire parmi les coups possibles
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

        // liberation de la liste des coups
        free(coups);
    }

    // liberation de l'etat
    free(etatCourant);

    if (testFin(etatCourant) == ORDI_GAGNE){
        victOrdi++;
    }else{
        victHumain++;
    }

    return estFini;
}





/*
 * Fonction qui calcule la B-valeur du noeud fourni.
 * 
 * Si le noeud fourni n'a pas de parent ou n'est pas deja explore,
 * on renvoit une b_valeur nulle.
 * 
 * @param noeud, noeud fourni
 * @return la B-valeur
 */
double getBValeur(Noeud * noeud){
    double moyenne_recompense, exploration, b_valeur = 0;

    // si le noeud a deja ete parcouru et qu'il n'est pas la racine
    if(noeud->estParcouru == 1 && noeud->parent != NULL){
        // calcul de la moyenne de la recompense (nb_victoires / nb_simus) (exploitation)
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
