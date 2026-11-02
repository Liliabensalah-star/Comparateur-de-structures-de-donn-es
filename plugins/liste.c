
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

/* Indique si le mot est présent dans la liste */
/* Complexité : O(n) */
static Resultat Liste_chercher(void * s, const char * mot) {
  Liste * liste = (Liste *)s;
  Maillon * current = NULL;
  if((current = Liste_trouver(liste, mot)) == NULL) {
    return RES_ABSENT;
  }
  Liste_signaler(liste, ETAPE_TROUVE, current->mot);
  return RES_OK;
}

/* Renvoie le nombre de mots rangés */
/* Complexité : O(1) */
static size_t Liste_taille(void * s) {
  return ((Liste *)s)->nb_mots;
}

/* Renvoie le nombre de comparaisons faites depuis la création */
/* Complexité : O(1) */
static long Liste_operations(void * s) {
  return ((Liste *)s)->nb_comparaisons;
}

/* Libère tous les maillons, leurs mots, puis la liste */
/* Complexité : O(n) */
static void Liste_detruire(void * s) {
  Liste * liste = (Liste *)s;
  Maillon * current = NULL;
  Maillon * suivant = NULL;
  if(liste == NULL) {
    return;
  }
  for(current = liste->tete; current != NULL; current = suivant) {
    suivant = current->suivant;
    free(current->mot);
    free(current);
  }
  free(liste);
}

/* Donne chaque mot et sa place à la fonction f (pour le dessin) */
/* Complexité : O(n) */
static void Liste_parcourir(void * s, VisiteurMot f, void * contexte) {
  Liste * liste = (Liste *)s;
  Maillon * current = NULL;
  int colonne = 0;
  for(current = liste->tete; current != NULL; current = current->suivant) {
    f(current->mot, 0, colonne, contexte);
    ++colonne;
  }
}

/* Branche (ou débranche si f vaut NULL) la fonction d'animation */
/* Complexité : O(1) */
static void Liste_observer(void * s, ObservateurEtape f, void * contexte) {
  Liste * liste = (Liste *)s;
  liste->observateur = f;
  liste->contexte = contexte;
}

/* Suppression d'un mot : prévue dans la tâche N */
/* Complexité : O(1) pour l'instant */
static Resultat Liste_supprimer(void * s, const char * mot) {
  (void)s;
  (void)mot;
  return RES_ABSENT;
}

/* La fiche de la liste, remplie avec nos fonctions */
static const Structure fiche = {
  .nom = "Liste",
  .creer = Liste_creer,
  .ajouter = Liste_ajouter,
  .chercher = Liste_chercher,
  .supprimer = Liste_supprimer,
  .taille = Liste_taille,
  .operations = Liste_operations,
  .detruire = Liste_detruire,
  .parcourir = Liste_parcourir,
  .observer = Liste_observer
};

/* Fonction récupérée par le programme avec dlsym */
const Structure * structure_plugin(void) {
  return &fiche;
}