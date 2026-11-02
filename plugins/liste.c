
/* FICHIER : plugins/liste.c */
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
  char * mot;      
  Maillon * suivant; 
};

/* La liste : le premier maillon et les compteurs */
typedef struct Liste Liste;
struct Liste {
  Maillon * tete;              
  size_t nb_mots;           
  long nb_comparaisons;        
  ObservateurEtape observateur; 
  void * contexte;              
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

/* Prévient l'observateur d'une étape, s'il y en a un */
/* Complexité : O(1) */
static void Liste_signaler(Liste * liste, TypeEtape type, const char * mot) {
  if(liste->observateur != NULL) {
    liste->observateur(type, mot, liste->contexte);
  }
}

/* Allocation d'un maillon contenant une copie du mot */
/* Complexité : O(1) */
static Maillon * Maillon_alloc(const char * mot, Maillon * suivant) {
  Maillon * res = NULL;
  if((res = (Maillon *)malloc(sizeof(Maillon))) == NULL) {
    return NULL;
  }
  if((res->mot = strdup(mot)) == NULL) {
    free(res);
    return NULL;
  }
  res->suivant = suivant;
  return res;
}

/* Cherche le mot en comptant chaque comparaison */
/* Renvoie le maillon trouvé, ou NULL */
/* Complexité : O(n) */
static Maillon * Liste_trouver(Liste * liste, const char * mot) {
  Maillon * current = NULL;
  for(current = liste->tete; current != NULL; current = current->suivant) {
    ++(liste->nb_comparaisons);
    Liste_signaler(liste, ETAPE_VISITE, current->mot);
    if(strcmp(current->mot, mot) == 0) {
      return current;
    }
  }
  return NULL;
}

/* Ajoute une copie du mot en tête, les doublons sont refusés */
/* Complexité : O(n) */
static Resultat Liste_ajouter(void * s, const char * mot) {
  Liste * liste = (Liste *)s;
  Maillon * nouveau = NULL;
  if(Liste_trouver(liste, mot) != NULL) {
    return RES_ABSENT;
  }
  if((nouveau = Maillon_alloc(mot, liste->tete)) == NULL) {
    return RES_ERREUR;
  }
  liste->tete = nouveau;
  ++(liste->nb_mots);
  Liste_signaler(liste, ETAPE_INSERTION, nouveau->mot);
  return RES_OK;
}