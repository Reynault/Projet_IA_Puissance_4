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
	racine->etat = copieEtat(etat);

    // implémentation de l'algorithme MCTS-UCT pour 
    // déterminer le meilleur coup ci-dessous
    Noeud * courant;
    Noeud * fils;
	int iter = 0;

	do {
        // on part du noeud racine
        courant = racine;
        
        // tant qu'on arrive pas à la fin de l'arbre
        do{
            // le noeud courant est alors parcouru
            courant->estParcouru = 1;
            // création des fils
            creationFils(courant);
            // récupération du fils prioritaire pour le noeud courant
            fils = getNoeudPrioritaire(courant);
            // si celui-ci n'a pas encore été parcouru
            if(!fils->estParcouru){
                // marche aléatoire
                courant = effectuerMarcheAleatoire(courant);
            }else{
                // sinon, le fils devient le noeud courant
                courant = fils;
            }
        }while(!testFin(courant->etat));
        
        // récupération du temps pour vérification
		toc = clock();
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iter ++;
	} while ( temps < tempsmax );

	/* fin de l'algorithme  */

	// Jouer le meilleur premier coup
	jouerCoup(etat, getMeilleurCoup(racine));

	// Penser à libérer la mémoire :
	freeNoeud(racine);
}

/*
 * Fonction qui créer les fils d'un noeud
 * 
 * @param noeud, dont il faut créer les fils
 */
int creationFils(Noeud * noeud){
	Coup ** coups;
    coups = coups_possibles(noeud->etat);
    
    // Parcours de tous les coups possibles et ajout de chaque
    Noeud * enfant;
    int k = 0;
    while ( coups[k] != NULL) {
        enfant = ajouterEnfant(noeud, coups[k]);
        k++;
    }
    
    // Libération coups (tableau alloué qui contient les coups possibles)
    free(coups);
}

/*
 * Fonction qui permet de récupérer le noeud
 * prioritaire parmis les fils du noeud passé en paramètre.
 * 
 * Sachant que si un noeud parmis les fils n'a pas été explorés, on récupère
 * un tableau dans lequel on en prend un aléatoirement
 * 
 * @param noeud, le noeud duquel on veut récupérer le fils prioritaire
 * @return le noeud le plus prioritaire
 */
Noeud * getNoeudPrioritaire(Noeud * noeud){
    Noeud * prioritaire;
    
    Noeud ** enfants = noeud->enfants;
    int nb_enfants = noeud->nb_enfants;
    
    int indices_pas_parcourus[nb_enfants];  // indices des noeuds pas déjà parcourus dans enfants
    int nb_pas_parcourus = 0;
    
    int b_valeur_courante, max = INT_MIN;
    
    // vérification de la présence de noeuds non parcourus
    for(int i = 0; i < nb_enfants; i++){
        if(!enfants[i]->estParcouru){
            indices_pas_parcourus[nb_pas_parcourus] = i;
            nb_pas_parcourus ++;
        }
    }
    
    // si il y a des noeuds non parcourus
    if(nb_pas_parcourus > 0){
        
        // on retourne l'un d'entre eux aléatoirement
        prioritaire = enfants[
            indices_pas_parcourus[rand()%nb_pas_parcourus]
        ];
        
    }else{
        
        // sinon, parcours et récupération de la meilleur B-Valeur
        for(int i = 0; i < nb_enfants; i++){
            b_valeur_courante = getBValeur(enfants[i]);
            if(b_valeur_courante > max){
                prioritaire = enfants[i];
                max = b_valeur_courante;
            }
        }
        
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
    int max, valeurCourante;
    int nb_enfants = noeud->nb_enfants;
    Coup * meilleurCoup;
    Noeud * enfant;
    
    // récupération du meilleur coup parmis les enfants du noeud fourni
    for(int i = 0; i < nb_enfants; i++){
        enfant = noeud->enfants[i];
        // les coups possibles sont ceux d'un enfant qui a été parcouru
        if(enfant->estParcouru){
            valeurCourante = enfant->nb_victoires/enfant->nb_simus;
            if(valeurCourante > max){
                max = valeurCourante;
                meilleurCoup = noeud->coup;
            }
        }
    }
    
    // si le meilleur coup n'a pas été trouvé, cela signifie qu'il n'y a pas
    // de fils qui a été exploré, on prend alors un coup aléatoire
    if(meilleurCoup == NULL){
        meilleurCoup = noeud->enfants[
            rand()%nb_enfants
        ]->coup;
    }
    
    return meilleurCoup;
}

/*
 * Fonction qui permet d'effectuer la marche aléatoire, elle va parcourir aléatoirement
 * l'arbre en partant d'un noeud. Elle va alors, générer les fils pour chaque noeud
 * puis en prendre un aléatoirement, tout en mettant à jour les noeuds jusqu'à arriver
 * à l'état tel qu'on est à la fin du jeu.
 * 
 * @param noeud, à partir duquel on réalise la marche aléatoire
 * @return le noeud de la fin de la marche aléatoire
 */
Noeud * effectuerMarcheAleatoire(Noeud * noeud){
    int iteration = 0; // nombre d'itération utilisé pour faire le chemin arrière
    Noeud * courant = noeud;
    Noeud * parent;
    
    // tant qu'on arrive pas à la fin
    while(!testFin(courant->etat)){
        // noeud mis à parcouru
        // génération des noeuds enfants
        // choix d'un noeud aléatoire
        iteration ++;
    }
        
    // quand on arrive au noeud final, il faut répercuter les valeurs
    // vers le haut (jusqu'au noeud fourni en paramètre)
    
    // tant qu'on est pas revenu au noeud d'origine
    while(iteration > 0){
        // mise à jour du noeud courant (nb_simus et nb_victoires)
        // puis on monte dans l'arbre
        iteration --;
    }
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
float getBValeur(Noeud * noeud){
    int moyenne_recompense, exploration;
    int b_valeur = 0;
    
    // si le noeud a déjà été parcouru et qu'il n'est pas la racine
    if(noeud->estParcouru && noeud->parent != NULL){
        // calcul de la moyenne de la récompense (nb_victoires / nb_simus) (exploitation)
        moyenne_recompense = noeud->nb_victoires / noeud->nb_simus;
        
        // calcul de l'exploration
        exploration = sqrt(log(noeud->parent->nb_simus)/noeud->nb_simus);
        
        // calcul de la b_valeur en utilisant la constante C
        b_valeur = moyenne_recompense + C * exploration;
        
        // inversion du signe en fonction du joueur
        if(noeud->joueur == 0){
            b_valeur = -b_valeur;
        }
    }
    
    return b_valeur;
}
