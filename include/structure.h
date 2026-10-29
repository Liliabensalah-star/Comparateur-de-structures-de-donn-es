/**
 * @file structure.h
 * @brief Interface commune à toutes les structures de données du comparateur.
 *
 * Chaque structure (liste chaînée, AVL, table de hachage...) est compilée
 * en plugin .so et remplit la même « fiche » de fonctions (type Structure).
 * Le banc de test et l'affichage n'utilisent que cette fiche : ils
 * fonctionnent avec n'importe quelle structure sans connaître son intérieur.
 *
 * Contrat figé le 29/10/2026 (Groupe 8 : Javerichalle et Lilia).
 * Toute modification doit être validée par les deux.
 *
 * Compilation : gcc -Wall -Wextra -std=gnu11
 */
#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <stddef.h>

/** Résultat possible d'une opération. */
typedef enum {
    RES_ERREUR = -1, /**< problème (mémoire insuffisante...) */
    RES_ABSENT = 0,  /**< mot non trouvé, ou déjà présent : rien n'a changé */
    RES_OK     = 1   /**< opération réussie */
} Resultat;

/** Ce que la structure est en train de faire (utilisé pour les animations). */
typedef enum {
    ETAPE_VISITE,      /**< la structure compare le mot cherché à ce mot */
    ETAPE_TROUVE,      /**< le mot cherché est trouvé */
    ETAPE_INSERTION,   /**< le mot vient d'être rangé */
    ETAPE_SUPPRESSION, /**< le mot vient d'être retiré */
    ETAPE_ROTATION     /**< AVL : rotation autour de ce mot */
} TypeEtape;

/**
 * Fonction appelée par parcourir() pour chaque mot rangé.
 * ligne et colonne donnent la place du mot pour le dessin :
 *   - Liste   : ligne = 0,          colonne = position dans la liste
 *   - Hachage : ligne = n° de case, colonne = position dans la case
 *   - AVL     : ligne = profondeur, colonne = rang dans l'ordre alphabétique
 */
typedef void (*VisiteurMot)(const char *mot, int ligne, int colonne, void *contexte);

/** Fonction appelée à chaque étape d'une opération (animations). */
typedef void (*ObservateurEtape)(TypeEtape type, const char *mot, void *contexte);

/** La fiche que chaque structure remplit. */
typedef struct {
    const char *nom;                                 /**< "Liste", "AVL", "Hachage" */
    void     *(*creer)(void);                        /**< structure vide, ou NULL */
    Resultat  (*ajouter)(void *s, const char *mot);  /**< range une copie du mot */
    Resultat  (*chercher)(void *s, const char *mot); /**< le mot est-il présent ? */
    Resultat  (*supprimer)(void *s, const char *mot);/**< retire le mot */
    size_t    (*taille)(void *s);                    /**< nombre de mots rangés */
    long      (*operations)(void *s);                /**< comparaisons depuis creer */
    void      (*detruire)(void *s);                  /**< libère toute la mémoire */
    void      (*parcourir)(void *s, VisiteurMot f, void *contexte);
    void      (*observer)(void *s, ObservateurEtape f, void *contexte); /**< f = NULL : aucune étape */
} Structure;

/** Fonction que chaque plugin .so doit fournir (récupérée avec dlsym). */
const Structure *structure_plugin(void);

#endif /* STRUCTURE_H */
