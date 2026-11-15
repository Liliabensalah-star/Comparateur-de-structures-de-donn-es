/* FICHIER : tests/test_dictionnaire.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dictionnaire.h"

static int nb_erreurs = 0;

/* Affiche OK si la condition est vraie, ECHEC sinon */
static void verifier(int condition, const char * message) {
  if(condition) {
    printf("OK    : %s\n", message);
  } else {
    printf("ECHEC : %s\n", message);
    ++nb_erreurs;
  }
}

/* Compte combien de fois le mot apparaît dans le dictionnaire */
static int compter(const Dictionnaire * dico, const char * mot) {
  size_t i;
  int res = 0;
  for(i = 0; i < dico->nb_mots; ++i) {
    if(strcmp(dico->mots[i], mot) == 0) {
      ++res;
    }
  }
  return res;
}

int main() {
  Dictionnaire * dico = NULL;
  Dictionnaire * copie = NULL;
  char long100[MOT_LONGUEUR_MAX + 1];
  size_t i;
  int meme_ordre = 1;

  memset(long100, 'b', MOT_LONGUEUR_MAX);
  long100[MOT_LONGUEUR_MAX] = '\0';

  verifier(Dictionnaire_charger("data/inexistant.txt") == NULL, "fichier inexistant : NULL");

  if((dico = Dictionnaire_charger("data/test.txt")) == NULL) {
    printf("ECHEC : impossible de lire data/test.txt\n");
    exit(EXIT_FAILURE);
  }
  verifier(dico->nb_mots == 6, "6 mots lus (lignes vides et mot trop long ignores)");
  verifier(strcmp(dico->mots[0], "chat") == 0, "1er mot : chat");
  verifier(strcmp(dico->mots[1], "abri") == 0, "ligne vide ignoree : 2e mot abri");
  verifier(strcmp(dico->mots[2], "lion") == 0, "\\r retire : 3e mot lion");
  verifier(strcmp(dico->mots[3], long100) == 0, "mot de 100 caracteres garde");
  verifier(strcmp(dico->mots[4], "été") == 0, "accents gardes : été");
  verifier(strcmp(dico->mots[5], "chat") == 0, "doublon du fichier garde");

  if((copie = Dictionnaire_charger("data/test.txt")) == NULL) {
    printf("ECHEC : impossible de relire data/test.txt\n");
    Dictionnaire_free(&dico);
    exit(EXIT_FAILURE);
  }
  Dictionnaire_melanger(dico, 42);
  Dictionnaire_melanger(copie, 42);
  for(i = 0; i < dico->nb_mots; ++i) {
    if(strcmp(dico->mots[i], copie->mots[i]) != 0) {
      meme_ordre = 0;
    }
  }
  verifier(meme_ordre, "meme graine : meme ordre");
  verifier(dico->nb_mots == 6 && compter(dico, "chat") == 2 && compter(dico, "abri") == 1
           && compter(dico, "lion") == 1 && compter(dico, "été") == 1, "melange : aucun mot perdu");
  printf("Ordre apres melange :");
  for(i = 0; i < dico->nb_mots; ++i) {
    printf(" %.10s", dico->mots[i]);
  }
  printf("\n");

  Dictionnaire_free(&dico);
  Dictionnaire_free(&copie);
  verifier(dico == NULL && copie == NULL, "Dictionnaire_free met le pointeur a NULL");

  printf("Bilan : %d erreur(s)\n", nb_erreurs);
  exit(nb_erreurs == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}