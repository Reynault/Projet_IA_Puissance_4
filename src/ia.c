#include "jeu.h"
#include "ia.h"

/*
 * Cr�er un nouveau noeud en jouant un coup � partir d'un parent
 * utiliser nouveauNoeud(NULL, NULL) pour cr�er la racine
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
	racine->etat = copieEtat(etat);

    // impl�mentation de l'algorithme MCTS-UCT pour 
    // d�terminer le meilleur coup ci-dessous
    Noeud * courant;
    Noeud * fils;
	int iter = 0;

	do {
        // on part du noeud racine
        courant = racine;
        
        // tant qu'on arrive pas � la fin de l'arbre
        do{
            // le noeud courant est alors parcouru
            courant->estParcouru = 1;
            // cr�ation des fils
            creationFils(courant);
            // r�cup�ration du fils prioritaire pour le noeud courant
            fils = getNoeudPrioritaire(courant);
            // si celui-ci n'a pas encore �t� parcouru
            if(!fils->estParcouru){
                // marche al�atoire
                courant = effectuerMarcheAleatoire(courant);
            }else{
                // sinon, le fils devient le noeud courant
                courant = fils;
            }
        }while(!testFin(courant->etat));
        
        // r�cup�ration du temps pour v�rification
		toc = clock();
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
		iter ++;
	} while ( temps < tempsmax );

	/* fin de l'algorithme  */

	// Jouer le meilleur premier coup
	jouerCoup(etat, getMeilleurCoup(racine));

	// Penser � lib�rer la m�moire :
	freeNoeud(racine);
}

/*
 * Fonction qui cr�er les fils d'un noeud
 * 
 * @param noeud, dont il faut cr�er les fils
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
    
    // Lib�ration coups (tableau allou� qui contient les coups possibles)
    free(coups);
}

/*
 * Fonction qui permet de r�cup�rer le noeud
 * prioritaire parmis les fils du noeud pass� en param�tre.
 * 
 * Sachant que si un noeud parmis les fils n'a pas �t� explor�s, on r�cup�re
 * un tableau dans lequel on en prend un al�atoirement
 * 
 * @param noeud, le noeud duquel on veut r�cup�rer le fils prioritaire
 * @return le noeud le plus prioritaire
 */
Noeud * getNoeudPrioritaire(Noeud * noeud){
    Noeud * prioritaire;
    
    Noeud ** enfants = noeud->enfants;
    int nb_enfants = noeud->nb_enfants;
    
    int indices_pas_parcourus[nb_enfants];  // indices des noeuds pas d�j� parcourus dans enfants
    int nb_pas_parcourus = 0;
    
    int b_valeur_courante, max = INT_MIN;
    
    // v�rification de la pr�sence de noeuds non parcourus
    for(int i = 0; i < nb_enfants; i++){
        if(!enfants[i]->estParcouru){
            indices_pas_parcourus[nb_pas_parcourus] = i;
            nb_pas_parcourus ++;
        }
    }
    
    // si il y a des noeuds non parcourus
    if(nb_pas_parcourus > 0){
        
        // on retourne l'un d'entre eux al�atoirement
        prioritaire = enfants[
            indices_pas_parcourus[rand()%nb_pas_parcourus]
        ];
        
    }else{
        
        // sinon, parcours et r�cup�ration de la meilleur B-Valeur
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
Coup * getMeilleurCoup(Noeud * noeud){
    int max, valeurCourante;
    int nb_enfants = noeud->nb_enfants;
    Coup * meilleurCoup;
    Noeud * enfant;
    
    // r�cup�ration du meilleur coup parmis les enfants du noeud fourni
    for(int i = 0; i < nb_enfants; i++){
        enfant = noeud->enfants[i];
        // les coups possibles sont ceux d'un enfant qui a �t� parcouru
        if(enfant->estParcouru){
            valeurCourante = enfant->nb_victoires/enfant->nb_simus;
            if(valeurCourante > max){
                max = valeurCourante;
                meilleurCoup = noeud->coup;
            }
        }
    }
    
    // si le meilleur coup n'a pas �t� trouv�, cela signifie qu'il n'y a pas
    // de fils qui a �t� explor�, on prend alors un coup al�atoire
    if(meilleurCoup == NULL){
        meilleurCoup = noeud->enfants[
            rand()%nb_enfants
        ]->coup;
    }
    
    return meilleurCoup;
}

/*
 * Fonction qui permet d'effectuer la marche al�atoire, elle va parcourir al�atoirement
 * l'arbre en partant d'un noeud. Elle va alors, g�n�rer les fils pour chaque noeud
 * puis en prendre un al�atoirement, tout en mettant � jour les noeuds jusqu'� arriver
 * � l'�tat tel qu'on est � la fin du jeu.
 * 
 * @param noeud, � partir duquel on r�alise la marche al�atoire
 * @return le noeud de la fin de la marche al�atoire
 */
Noeud * effectuerMarcheAleatoire(Noeud * noeud){
    int iteration = 0; // nombre d'it�ration utilis� pour faire le chemin arri�re
    Noeud * courant = noeud;
    Noeud * parent;
    
    // tant qu'on arrive pas � la fin
    while(!testFin(courant->etat)){
        // noeud mis � parcouru
        // g�n�ration des noeuds enfants
        // choix d'un noeud al�atoire
        iteration ++;
    }
        
    // quand on arrive au noeud final, il faut r�percuter les valeurs
    // vers le haut (jusqu'au noeud fourni en param�tre)
    
    // tant qu'on est pas revenu au noeud d'origine
    while(iteration > 0){
        // mise � jour du noeud courant (nb_simus et nb_victoires)
        // puis on monte dans l'arbre
        iteration --;
    }
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
float getBValeur(Noeud * noeud){
    int moyenne_recompense, exploration;
    int b_valeur = 0;
    
    // si le noeud a d�j� �t� parcouru et qu'il n'est pas la racine
    if(noeud->estParcouru && noeud->parent != NULL){
        // calcul de la moyenne de la r�compense (nb_victoires / nb_simus) (exploitation)
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
