/* FICHIER : plugins/avl.c */
/**
 * @file avl.c
 * @brief Plugin : arbre AVL de mots.
 * @author Lilia BEN SALAH
 *
 * Tâche F : ajout et recherche (arbre binaire de recherche, sans rotation).
 * Tâche H : les rotations seront ajoutées dans Noeud_inserer.
 */
#include <stdlib.h>
#include <string.h>
#include "structure.h"

/* Un nœud : un mot, sa hauteur et ses deux sous-arbres */
typedef struct Noeud Noeud;
struct Noeud {
  char * mot;
  int hauteur;      /* 1 pour une feuille */
  Noeud * gauche;   /* mots plus petits (ordre de strcmp) */
  Noeud * droite;   /* mots plus grands */
};

/* L'arbre : la racine et les compteurs */
typedef struct Arbre Arbre;
struct Arbre {
  Noeud * racine;
  size_t nb_mots;
  long nb_comparaisons;
  ObservateurEtape observateur;
  void * contexte;
};

/* Crée un arbre vide. Renvoie NULL si la mémoire manque. */
/* Complexité : O(1) */
static void * Arbre_creer(void) {
  Arbre * res = NULL;
  if((res = (Arbre *)malloc(sizeof(Arbre))) == NULL) {
    return NULL;
  }
  res->racine = NULL;
  res->nb_mots = 0;
  res->nb_comparaisons = 0;
  res->observateur = NULL;
  res->contexte = NULL;
  return res;
}

/* Prévient l'observateur d'une étape, s'il y en a un */
/* Complexité : O(1) */
static void Arbre_signaler(Arbre * arbre, TypeEtape type, const char * mot) {
  if(arbre->observateur != NULL) {
    arbre->observateur(type, mot, arbre->contexte);
  }
}

/* Compare le mot cherché au mot d'un nœud, en comptant la comparaison */
/* Renvoie < 0 si mot est avant, 0 si égal, > 0 si mot est après */
/* Complexité : O(1) (hors longueur des mots) */
static int Arbre_comparer(Arbre * arbre, const char * mot, Noeud * noeud) {
  ++(arbre->nb_comparaisons);
  Arbre_signaler(arbre, ETAPE_VISITE, noeud->mot);
  return strcmp(mot, noeud->mot);
}

/* Allocation d'une feuille contenant une copie du mot */
/* Complexité : O(1) */
static Noeud * Noeud_alloc(const char * mot) {
  Noeud * res = NULL;
  if((res = (Noeud *)malloc(sizeof(Noeud))) == NULL) {
    return NULL;
  }
  if((res->mot = strdup(mot)) == NULL) {
    free(res);
    return NULL;
  }
  res->hauteur = 1;
  res->gauche = NULL;
  res->droite = NULL;
  return res;
}

/* Hauteur d'un sous-arbre (0 s'il est vide) */
/* Complexité : O(1) */
static int Noeud_hauteur(Noeud * noeud) {
  return (noeud == NULL) ? 0 : noeud->hauteur;
}

/* Recalcule la hauteur d'un nœud à partir de ses deux enfants */
/* Complexité : O(1) */
static void Noeud_maj_hauteur(Noeud * noeud) {
  int hg = Noeud_hauteur(noeud->gauche);
  int hd = Noeud_hauteur(noeud->droite);
  noeud->hauteur = 1 + (hg > hd ? hg : hd);
}

/* Range le mot dans le sous-arbre et renvoie sa nouvelle racine */
/* *res reçoit RES_OK, RES_ABSENT (doublon) ou RES_ERREUR (mémoire) */
/* Complexité : O(h), h = hauteur de l'arbre */
static Noeud * Noeud_inserer(Arbre * arbre, Noeud * noeud, const char * mot, Resultat * res) {
  int cmp = 0;
  if(noeud == NULL) {
    Noeud * nouveau = NULL;
    if((nouveau = Noeud_alloc(mot)) == NULL) {
      *res = RES_ERREUR;
      return NULL;
    }
    *res = RES_OK;
    Arbre_signaler(arbre, ETAPE_INSERTION, nouveau->mot);
    return nouveau;
  }
  cmp = Arbre_comparer(arbre, mot, noeud);
  if(cmp < 0) {
    noeud->gauche = Noeud_inserer(arbre, noeud->gauche, mot, res);
  } else if(cmp > 0) {
    noeud->droite = Noeud_inserer(arbre, noeud->droite, mot, res);
  } else {
    *res = RES_ABSENT;
    return noeud;
  }
  Noeud_maj_hauteur(noeud);
  /* Tâche H : rééquilibrer ici (rotations) */
  return noeud;
}

/* Ajoute une copie du mot, les doublons sont refusés */
/* Complexité : O(h) */
static Resultat Arbre_ajouter(void * s, const char * mot) {
  Arbre * arbre = (Arbre *)s;
  Resultat res = RES_ERREUR;
  arbre->racine = Noeud_inserer(arbre, arbre->racine, mot, &res);
  if(res == RES_OK) {
    ++(arbre->nb_mots);
  }
  return res;
}

/* Indique si le mot est présent dans l'arbre */
/* Complexité : O(h) */
static Resultat Arbre_chercher(void * s, const char * mot) {
  Arbre * arbre = (Arbre *)s;
  Noeud * current = arbre->racine;
  int cmp = 0;
  while(current != NULL) {
    cmp = Arbre_comparer(arbre, mot, current);
    if(cmp == 0) {
      Arbre_signaler(arbre, ETAPE_TROUVE, current->mot);
      return RES_OK;
    }
    current = (cmp < 0) ? current->gauche : current->droite;
  }
  return RES_ABSENT;
}

/* Renvoie le nombre de mots rangés */
/* Complexité : O(1) */
static size_t Arbre_taille(void * s) {
  return ((Arbre *)s)->nb_mots;
}

/* Renvoie le nombre de comparaisons faites depuis la création */
/* Complexité : O(1) */
static long Arbre_operations(void * s) {
  return ((Arbre *)s)->nb_comparaisons;
}

/* Libère un sous-arbre : les enfants d'abord, puis le nœud */
/* Complexité : O(n) */
static void Noeud_detruire(Noeud * noeud) {
  if(noeud == NULL) {
    return;
  }
  Noeud_detruire(noeud->gauche);
  Noeud_detruire(noeud->droite);
  free(noeud->mot);
  free(noeud);
}

/* Libère tous les nœuds, leurs mots, puis l'arbre */
/* Complexité : O(n) */
static void Arbre_detruire(void * s) {
  Arbre * arbre = (Arbre *)s;
  if(arbre == NULL) {
    return;
  }
  Noeud_detruire(arbre->racine);
  free(arbre);
}

/* Parcours infixe : les mots sortent dans l'ordre alphabétique */
/* ligne = profondeur (racine = 0), colonne = rang alphabétique */
/* Complexité : O(n) */
static void Noeud_parcourir(Noeud * noeud, int profondeur, int * rang, VisiteurMot f, void * contexte) {
  if(noeud == NULL) {
    return;
  }
  Noeud_parcourir(noeud->gauche, profondeur + 1, rang, f, contexte);
  f(noeud->mot, profondeur, *rang, contexte);
  ++(*rang);
  Noeud_parcourir(noeud->droite, profondeur + 1, rang, f, contexte);
}

/* Donne chaque mot et sa place à la fonction f (pour le dessin) */
/* Complexité : O(n) */
static void Arbre_parcourir(void * s, VisiteurMot f, void * contexte) {
  Arbre * arbre = (Arbre *)s;
  int rang = 0;
  Noeud_parcourir(arbre->racine, 0, &rang, f, contexte);
}

/* Branche (ou débranche si f vaut NULL) la fonction d'animation */
/* Complexité : O(1) */
static void Arbre_observer(void * s, ObservateurEtape f, void * contexte) {
  Arbre * arbre = (Arbre *)s;
  arbre->observateur = f;
  arbre->contexte = contexte;
}

/* Suppression d'un mot : prévue dans la tâche O */
/* Complexité : O(1) pour l'instant */
static Resultat Arbre_supprimer(void * s, const char * mot) {
  (void)s;
  (void)mot;
  return RES_ABSENT;
}

/* La fiche de l'AVL, remplie avec nos fonctions */
static const Structure fiche = {
  .nom = "AVL",
  .creer = Arbre_creer,
  .ajouter = Arbre_ajouter,
  .chercher = Arbre_chercher,
  .supprimer = Arbre_supprimer,
  .taille = Arbre_taille,
  .operations = Arbre_operations,
  .detruire = Arbre_detruire,
  .parcourir = Arbre_parcourir,
  .observer = Arbre_observer
};

/* Fonction récupérée par le programme avec dlsym */
const Structure * structure_plugin(void) {
  return &fiche;
}
