/* FICHIER : src/dictionnaire.c */
/**
 * @file dictionnaire.c
 * @brief Lecture d'un fichier de mots et mélange des mots.
 * @author Javerichalle MFINA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dictionnaire.h"

/* Agrandit le tableau de mots s'il est plein */
/* Renvoie 1 si réussi, 0 si la mémoire manque */
/* Complexité : O(n) */
static int Dictionnaire_update(Dictionnaire * dico) {
  char ** tmp = NULL;
  size_t capacite;
  if(dico->nb_mots < dico->capacite) {
    return 1;
  }
  capacite = dico->nb_mots * 2 + 10;
  if((tmp = (char **)realloc(dico->mots, capacite * sizeof(char *))) == NULL) {
    return 0;
  }
  dico->mots = tmp;
  dico->capacite = capacite;
  return 1;
}

/* Lit un mot par ligne ; NULL si le fichier ne s'ouvre pas ou si la mémoire manque */
/* Complexité : O(n) */
Dictionnaire * Dictionnaire_charger(const char * chemin) {
  Dictionnaire * res = NULL;
  FILE * fich = NULL;
  char buffer[1024];
  if((fich = fopen(chemin, "r")) == NULL) {
    fprintf(stderr, "Dictionnaire_charger : impossible d'ouvrir \"%s\"\n", chemin);
    return NULL;
  }
  if((res = (Dictionnaire *)malloc(sizeof(Dictionnaire))) == NULL) {
    fprintf(stderr, "Dictionnaire_charger : erreur d'allocation\n");
    fclose(fich);
    return NULL;
  }
  res->mots = NULL;
  res->nb_mots = 0;
  res->capacite = 0;
  while(fscanf(fich, "%1023s", buffer) == 1) {
    if(strlen(buffer) > MOT_LONGUEUR_MAX) {
      continue;
    }
    if(!Dictionnaire_update(res) || (res->mots[res->nb_mots] = strdup(buffer)) == NULL) {
      fprintf(stderr, "Dictionnaire_charger : erreur d'allocation\n");
      fclose(fich);
      Dictionnaire_free(&res);
      return NULL;
    }
    ++(res->nb_mots);
  }
  fclose(fich);
  return res;
}

/* Mélange les mots avec une graine fixe */
/* Complexité : O(n) */
void Dictionnaire_melanger(Dictionnaire * dico, unsigned int graine) {
  size_t i;
  size_t j;
  char * tmp = NULL;
  if(dico == NULL || dico->nb_mots < 2) {
    return;
  }
  srand(graine);
  for(i = dico->nb_mots - 1; i > 0; --i) {
    j = (size_t)rand() % (i + 1);
    tmp = dico->mots[i];
    dico->mots[i] = dico->mots[j];
    dico->mots[j] = tmp;
  }
}

/* Libère tous les mots, puis le dictionnaire, et met le pointeur à NULL */
/* Complexité : O(n) */
void Dictionnaire_free(Dictionnaire ** dico) {
  size_t i;
  if(dico == NULL || *dico == NULL) {
    return;
  }
  for(i = 0; i < (*dico)->nb_mots; ++i) {
    free((*dico)->mots[i]);
  }
  free((*dico)->mots);
  free(*dico);
  *dico = NULL;
}