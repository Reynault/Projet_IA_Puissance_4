#include "jeu.h"
#include "ia.h"

/*
 * Créer un nouveau noeud en jouant un coup à partir d'un parent
 * utiliser nouveauNoeud(NULL, NULL) pour créer la racine
 */
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
 * Ajouter un enfant à un parent en jouant un coup
 * retourne le pointeur sur l'enfant ajouté
 */
Noeud * ajouterEnfant(Noeud * parent, Coup * coup) {
	Noeud * enfant = nouveauNoeud (parent, coup ) ;
	parent->enfants[parent->nb_enfants] = enfant;
	parent->nb_enfants++;
	return enfant;
}

/*
 * Fonction qui créer les fils d'un noeud
 *
 * @param noeud, dont il faut créer les fils, il faut que le noeud n'ait pas de fils
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
    // Libération coups (tableau alloué qui contient les coups possibles)
    free(coups);
}

/*
 * Méthode de libération des noeuds inutilisés
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

	// créer l'arbre de recherche
	Noeud * racine = nouveauNoeud(NULL, NULL);
    Noeud * courant;
    FinDePartie resultat;

    // initialisation de l'état
	racine->etat = copieEtat(etat);

    // implémentation de l'algorithme MCTS-UCT pour 
    // déterminer le meilleur coup ci-dessous
	int iter = 0;
    int noeudNonExploree;

	do {
        // initiatlisation des valeurs de base
        noeudNonExploree = 0;
        courant = racine;

        // parcours de l'arbre jusqu'à trouver un noeud non parcouru, ou à arriver à la fin du jeu, ou arriver
        // à la fin de l'arbre
        do{
            // création des fils si le noeud n'a pas déjà été parcouru
            if(courant->estParcouru == 0){
                creationFils(courant);
                // le noeud courant est alors parcouru
                courant->estParcouru = 1;
            }

            // récupération du fils prioritaire pour le noeud courant
            // si le noeud courant peut encore mener à des nouvelles configurations (coups possibles > 0)
            if(courant->nb_enfants > 0) {
                courant = getNoeudPrioritaire(courant);

                // si celui-ci n'a pas encore été parcouru on sort de la boucle.
                if (courant->estParcouru == 0)  noeudNonExploree = 1;
            }
        }while( (resultat = testFin(courant->etat)) == NON && noeudNonExploree == 0);

        // si on s'arrête à cause d'un noeud non exploré
        if(noeudNonExploree == 1){
            // lancement d'une marche aléatoire
            resultat = effectuerMarcheAleatoire(courant);
            creationFils(courant);
            courant->estParcouru = 1; // et mise à jour du noeud exploré
        }

        // on remonte les valeurs vers la racine en mettant à jour les noeuds
        if(resultat != NON) {
            remonterValeurVersRacine(courant, resultat);
        }

        // récupération du temps pour vérification
		toc = clock();
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iter ++;
	} while ( temps < tempsmax );

	// Jouer le meilleur premier coup
	jouerCoup(etat, getMeilleurCoup(racine));

	// Penser à libérer la mémoire :
	freeNoeud(racine);
}

/*
 * Fonction qui permet de mettre à jour les valeurs
 * en partant d'un noeud
 * @param noeud à partir duquel on remonte les valeurs
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
 * Fonction qui permet de récupérer le noeud
 * prioritaire parmis les fils du noeud passé en paramètre.
 * 
 * Sachant que si un noeud parmis les fils n'a pas été explorés, on récupère
 * un tableau dans lequel on en prend un aléatoirement
 * 
 * @param noeud, le noeud duquel on veut récupérer le fils prioritaire (noeud qui possède des enfants)
 * @return le noeud le plus prioritaire
 */
Noeud * getNoeudPrioritaire(Noeud * noeud){
    Noeud * prioritaire = noeud;
    Noeud ** enfants = noeud->enfants;
    int nb_enfants = noeud->nb_enfants;
    
    int indices_pas_parcourus[nb_enfants];  // indices des noeuds pas déjà parcourus dans enfants
    int nb_pas_parcourus = 0;
    double b_valeur_courante, max = LONG_MIN;

    // vérification de la présence de noeuds non parcourus
    for(int i = 0; i < nb_enfants; i++){
        // si pas parcouru trouvé
        if(enfants[i]->estParcouru == 0){
            // ajout dans tableau
            indices_pas_parcourus[nb_pas_parcourus] = i;
            nb_pas_parcourus ++;
        }else{
            // sinon, mise à jour de max
            printf("Mise a jour du jeu max\n");
            b_valeur_courante = getBValeur(enfants[i]);
            if(b_valeur_courante > max){
                prioritaire = enfants[i];
                max = b_valeur_courante;
            }
        }
    }
    
    // si il y a des noeuds non parcourus
    if(nb_pas_parcourus > 0){
        // on retourne l'un d'entre eux aléatoirement
        prioritaire = enfants[
            indices_pas_parcourus[rand()%nb_pas_parcourus]
        ];
    }
    
    return prioritaire;
}

/*
 * Fonction qui permet de récupérer le meilleur coup
 * à partir d'un noeud racine. Donc parcours des noeuds
 * enfants, et récupération du coup de celui qui maximise
 * le critère voulu.
 * 
 * Critère courant : maximisation de la moyenne des récompenses obtenus ( nb_victoires / nb_simus )
 * 
 * @param noeud, noeud à partir duquel on cherche le meilleur coup (parmis ceux de ses fils)
 * @return le meilleur coup
 */
Coup * getMeilleurCoup(Noeud * noeud){
    float valeurCourante, max = INT_MIN;
    int nb_enfants = noeud->nb_enfants;
    Coup * meilleurCoup = NULL;
    Noeud * enfant;
    
    // récupération du meilleur coup parmis les enfants du noeud fourni
    for(int i = 0; i < nb_enfants; i++){
        enfant = noeud->enfants[i];
        // les coups possibles sont ceux d'un enfant qui a été parcouru
        if(enfant->estParcouru == 1){
            valeurCourante = (float)(enfant->nb_victoires) / (float)(enfant->nb_simus);
            if(valeurCourante > max){
                max = valeurCourante;
                meilleurCoup = enfant->coup;
            }
        }
    }
    
    // si le meilleur coup n'a pas été trouvé, cela signifie qu'il n'y a pas
    // de fils qui a été exploré, on prend alors un coup aléatoire
    if(meilleurCoup == NULL){
        meilleurCoup = noeud->enfants[rand()%nb_enfants]->coup;
        printf("meilleur coup aleatoire\n");
    }
    
    return meilleurCoup;
}

/*
 * Fonction qui permet d'effectuer la marche aléatoire, elle va parcourir aléatoirement
 * l'arbre en partant d'un noeud. On copie l'état actuel dans un autre état qui va nous servir
 * à parcourir l'arbre sans générer les fils. (juste un état à modifier)
 * 
 * @param noeud, à partir duquel on réalise la marche aléatoire
 * @return fin de partie trouvée
 */
FinDePartie effectuerMarcheAleatoire(Noeud * noeud){
    Etat * etatCourant;
    Coup ** coups;
    FinDePartie estFini;
    int nbCoups;

    // copie du bloc mémoire contenant l'état du noeud
    etatCourant = copieEtat(noeud->etat);

    // tant qu'on arrive pas à la fin
    while((estFini = testFin(etatCourant)) == NON){
        // récupération d'un mouvement aléatoire parmi les coups possibles
        coups = coups_possibles(etatCourant);
        nbCoups = 0;
        while ( coups[nbCoups] != NULL) {
            nbCoups++;
        }
        // modification du noeud courant
        jouerCoup(etatCourant, coups[rand()%nbCoups]);
        // libération de la liste des coups
        free(coups);
    }

    // libération de l'état
    free(etatCourant);

    return estFini;
}

/*
 * Fonction qui calcule la B-valeur du noeud fourni.
 * 
 * Si le noeud fourni n'a pas de parent où n'est pas déjà exploré,
 * on renvoit une b_valeur nulle.
 * 
 * @param noeud, noeud fourni
 * @return la B-valeur
 */
double getBValeur(Noeud * noeud){
    double moyenne_recompense, exploration, b_valeur = 0;

    // si le noeud a déjà été parcouru et qu'il n'est pas la racine
    if(noeud->estParcouru == 1 && noeud->parent != NULL){
        // calcul de la moyenne de la récompense (nb_victoires / nb_simus) (exploitation)
        moyenne_recompense = (double)noeud->nb_victoires / (double)noeud->nb_simus;
        
        // calcul de l'exploration
        exploration = sqrt(log(noeud->parent->nb_simus)/noeud->nb_simus);
        
        // calcul de la b_valeur en utilisant la constante C
        b_valeur = moyenne_recompense + C * exploration;

        // inversion du signe en fonction du joueur
        if(noeud->joueur == JOUEUR_HUMAIN){
            b_valeur = -b_valeur;
        }
    }
    
    return b_valeur;
}
