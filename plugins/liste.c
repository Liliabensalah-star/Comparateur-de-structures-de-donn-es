/**
 * @file liste.c
 * @brief Plugin : liste chaînée de mots.
 * @author Javerichalle MFINA
 */
#include <stdlib.h>     /* malloc, free */
#include <string.h>     /* strcmp, strdup */
#include "structure.h"  /* le contrat commun */

/** Un maillon : un mot et l'adresse du maillon suivant. */
typedef struct Maillon {
    char *mot;                /**< copie du mot (strdup) */
    struct Maillon *suivant;  /**< maillon suivant, NULL si dernier */
} Maillon;

/** La liste : le premier maillon et les compteurs. */
typedef struct {
    Maillon *tete;                 /**< premier maillon, NULL si liste vide */
    size_t nb_mots;                /**< nombre de mots rangés */
    long nb_comparaisons;          /**< compteur de strcmp */
    ObservateurEtape observateur;  /**< fonction d'animation, ou NULL */
    void *contexte;                /**< donnée passée à l'observateur */
} Liste;

/** Crée une liste vide. Renvoie NULL si la mémoire manque. */
static void *liste_creer(void)
{
    Liste *l = malloc(sizeof(Liste)); /* on réserve la place d'UNE structure Liste */
    if (l == NULL) {                  /* malloc a échoué : plus de mémoire */
        return NULL;                  /* le contrat dit : "structure vide, ou NULL" */
    }
    l->tete = NULL;                   /* liste vide : pas de premier maillon */
    l->nb_mots = 0;                   /* aucun mot rangé */
    l->nb_comparaisons = 0;           /* aucun strcmp fait pour l'instant */
    l->observateur = NULL;            /* pas d'animation tant qu'on n'appelle pas observer */
    l->contexte = NULL;               /* rien à transmettre à l'observateur */
    return l;                         /* on renvoie l'adresse de la nouvelle liste */
}