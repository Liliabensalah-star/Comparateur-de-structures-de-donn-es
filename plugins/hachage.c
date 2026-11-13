/* FICHIER : plugins/hachage.c */
/**
 * @file hachage.c
 * @brief Plugin : table de hachage de mots (chaînage, taille qui double).
 * @author Javerichalle MFINA
 */
#include <stdlib.h>
#include <string.h>
#include "structure.h"

#define CAPACITE_INITIALE 1024

/* Un maillon : un mot et l'adresse du maillon suivant dans la même case */
typedef struct Maillon Maillon;
struct Maillon {
  char * mot;
  Maillon * suivant;
};

/* La table : un tableau de cases, chaque case est une liste chaînée */
typedef struct Table Table;
struct Table {
  Maillon ** cases;
  size_t capacite;
  size_t nb_mots;
  long nb_comparaisons;
  ObservateurEtape observateur;
  void * contexte;
};

/* Fonction de hachage du cours (TypeDataString_hash) */
/* Complexité : O(m), m = longueur du mot */
static unsigned long hashString(const char * mot) {
  unsigned long h = 0;
  const char * current = NULL;
  for(current = mot; *current != '\0'; ++current) {
    h = 31 * h + *current + 129;
  }
  return h;
}

/* Crée une table vide. Renvoie NULL si la mémoire manque. */
/* Complexité : O(1) */
static void * Table_creer(void) {
  Table * res = NULL;
  if((res = (Table *)malloc(sizeof(Table))) == NULL) {
    return NULL;
  }
  if((res->cases = (Maillon **)calloc(CAPACITE_INITIALE, sizeof(Maillon *))) == NULL) {
    free(res);
    return NULL;
  }
  res->capacite = CAPACITE_INITIALE;
  res->nb_mots = 0;
  res->nb_comparaisons = 0;
  res->observateur = NULL;
  res->contexte = NULL;
  return res;
}

/* Prévient l'observateur d'une étape, s'il y en a un */
/* Complexité : O(1) */
static void Table_signaler(Table * table, TypeEtape type, const char * mot) {
  if(table->observateur != NULL) {
    table->observateur(type, mot, table->contexte);
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

/* Numéro de la case où doit se trouver le mot */
/* Complexité : O(m) */
static size_t Table_indice(const Table * table, const char * mot) {
  return hashString(mot) % table->capacite;
}

/* Cherche le mot dans sa case en comptant chaque comparaison */
/* Complexité : O(1) en moyenne (O(n) au pire) */
static Maillon * Table_trouver(Table * table, const char * mot) {
  Maillon * current = NULL;
  for(current = table->cases[Table_indice(table, mot)]; current != NULL; current = current->suivant) {
    ++(table->nb_comparaisons);
    Table_signaler(table, ETAPE_VISITE, current->mot);
    if(strcmp(current->mot, mot) == 0) {
      return current;
    }
  }
  return NULL;
}

/* Double le nombre de cases et y redistribue les maillons existants */
/* Complexité : O(n) */
static int Table_agrandir(Table * table) {
  size_t nouvelle_capacite = table->capacite * 2;
  Maillon ** nouvelles_cases = NULL;
  Maillon * current = NULL;
  Maillon * suivant = NULL;
  size_t i;
  size_t indice;
  if((nouvelles_cases = (Maillon **)calloc(nouvelle_capacite, sizeof(Maillon *))) == NULL) {
    return 0;
  }
  for(i = 0; i < table->capacite; ++i) {
    for(current = table->cases[i]; current != NULL; current = suivant) {
      suivant = current->suivant;
      indice = hashString(current->mot) % nouvelle_capacite;
      current->suivant = nouvelles_cases[indice];
      nouvelles_cases[indice] = current;
    }
  }
  free(table->cases);
  table->cases = nouvelles_cases;
  table->capacite = nouvelle_capacite;
  return 1;
}

/* Ajoute une copie du mot dans sa case, les doublons sont refusés */
/* Complexité : O(1) en moyenne */
static Resultat Table_ajouter(void * s, const char * mot) {
  Table * table = (Table *)s;
  Maillon * nouveau = NULL;
  size_t indice;
  if(Table_trouver(table, mot) != NULL) {
    return RES_ABSENT;
  }
  if(table->nb_mots >= table->capacite) {
    Table_agrandir(table);
  }
  indice = Table_indice(table, mot);
  if((nouveau = Maillon_alloc(mot, table->cases[indice])) == NULL) {
    return RES_ERREUR;
  }
  table->cases[indice] = nouveau;
  ++(table->nb_mots);
  Table_signaler(table, ETAPE_INSERTION, nouveau->mot);
  return RES_OK;
}

/* Indique si le mot est présent dans la table */
/* Complexité : O(1) en moyenne */
static Resultat Table_chercher(void * s, const char * mot) {
  Table * table = (Table *)s;
  Maillon * current = NULL;
  if((current = Table_trouver(table, mot)) == NULL) {
    return RES_ABSENT;
  }
  Table_signaler(table, ETAPE_TROUVE, current->mot);
  return RES_OK;
}

/* Renvoie le nombre de mots rangés */
/* Complexité : O(1) */
static size_t Table_taille(void * s) {
  return ((Table *)s)->nb_mots;
}

/* Renvoie le nombre de comparaisons faites depuis la création */
/* Complexité : O(1) */
static long Table_operations(void * s) {
  return ((Table *)s)->nb_comparaisons;
}

/* Libère tous les maillons de toutes les cases, puis la table */
/* Complexité : O(n + capacité) */
static void Table_detruire(void * s) {
  Table * table = (Table *)s;
  Maillon * current = NULL;
  Maillon * suivant = NULL;
  size_t i;
  if(table == NULL) {
    return;
  }
  for(i = 0; i < table->capacite; ++i) {
    for(current = table->cases[i]; current != NULL; current = suivant) {
      suivant = current->suivant;
      free(current->mot);
      free(current);
    }
  }
  free(table->cases);
  free(table);
}

/* Donne chaque mot et sa place à la fonction f (pour le dessin) */
/* Complexité : O(n + capacité) */
static void Table_parcourir(void * s, VisiteurMot f, void * contexte) {
  Table * table = (Table *)s;
  Maillon * current = NULL;
  size_t i;
  int colonne;
  for(i = 0; i < table->capacite; ++i) {
    colonne = 0;
    for(current = table->cases[i]; current != NULL; current = current->suivant) {
      f(current->mot, (int)i, colonne, contexte);
      ++colonne;
    }
  }
}

/* Branche (ou débranche si f vaut NULL) la fonction d'animation */
/* Complexité : O(1) */
static void Table_observer(void * s, ObservateurEtape f, void * contexte) {
  Table * table = (Table *)s;
  table->observateur = f;
  table->contexte = contexte;
}

/* Suppression d'un mot : prévue dans la tâche N */
/* Complexité : O(1) pour l'instant */
static Resultat Table_supprimer(void * s, const char * mot) {
  (void)s;
  (void)mot;
  return RES_ABSENT;
}

/* La fiche de la table de hachage, remplie avec nos fonctions */
static const Structure fiche = {
  .nom = "Hachage",
  .creer = Table_creer,
  .ajouter = Table_ajouter,
  .chercher = Table_chercher,
  .supprimer = Table_supprimer,
  .taille = Table_taille,
  .operations = Table_operations,
  .detruire = Table_detruire,
  .parcourir = Table_parcourir,
  .observer = Table_observer
};

/* Fonction récupérée par le programme avec dlsym */
const Structure * structure_plugin(void) {
  return &fiche;
}