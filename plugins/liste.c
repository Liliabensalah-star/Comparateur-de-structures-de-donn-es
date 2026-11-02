/* +---------------------------+ */
/* | FICHIER : plugins/liste.c | */
/* +---------------------------+ */
/**
 * @file liste.c
 * @brief Plugin : liste chaînée de mots.
 * @author Javerichalle MFINA
 */
#include <stdlib.h>
#include <string.h>
#include "structure.h"

/* Un maillon : un mot et l'adresse du maillon suivant */
typedef struct Maillon Maillon;
struct Maillon {
  char * mot;        /* copie du mot (strdup) */
  Maillon * suivant; /* maillon suivant, NULL si dernier */
};

/* La liste : le premier maillon et les compteurs */
typedef struct Liste Liste;
struct Liste {
  Maillon * tete;               /* premier maillon, NULL si liste vide */
  size_t nb_mots;               /* nombre de mots rangés */
  long nb_comparaisons;         /* compteur de strcmp */
  ObservateurEtape observateur; /* fonction d'animation, ou NULL */
  void * contexte;              /* donnée passée à l'observateur */
};

/* Crée une liste vide. Renvoie NULL si la mémoire manque. */
/* Complexité : O(1) */
static void * Liste_creer(void) {
  Liste * res = NULL;
  if((res = (Liste *)malloc(sizeof(Liste))) == NULL) {
    return NULL;
  }
  res->tete = NULL;
  res->nb_mots = 0;
  res->nb_comparaisons = 0;
  res->observateur = NULL;
  res->contexte = NULL;
  return res;
}