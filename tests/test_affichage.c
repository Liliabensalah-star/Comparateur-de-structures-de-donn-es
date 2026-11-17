/* FICHIER : tests/test_affichage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "affichage.h"

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

int main(int argc, char * argv[]) {
  Releves * releves = NULL;

  verifier(Releves_charger("data/inexistant.csv") == NULL, "fichier inexistant : NULL");

  if((releves = Releves_charger("data/mesures_exemple.csv")) == NULL) {
    printf("ECHEC : impossible de lire data/mesures_exemple.csv\n");
    exit(EXIT_FAILURE);
  }
  verifier(releves->nb == 48, "48 lignes lues (en-tete ignore)");
  verifier(strcmp(releves->tab[0].structure, "Liste") == 0, "1re ligne : structure Liste");
  verifier(strcmp(releves->tab[0].operation, "ajout") == 0, "1re ligne : operation ajout");
  verifier(releves->tab[0].nb_mots == 1000, "1re ligne : 1000 mots");
  verifier(releves->tab[0].temps_ms > 1.79 && releves->tab[0].temps_ms < 1.81, "1re ligne : temps 1.798 ms");
  verifier(releves->tab[0].nb_comparaisons == 499500, "1re ligne : 499500 comparaisons");
  if(argc > 1 && strcmp(argv[1], "--fenetre") == 0) {
    Affichage_courbes(releves);
  }
  Releves_free(&releves);
  verifier(releves == NULL, "Releves_free met le pointeur a NULL");
  printf("Bilan : %d erreur(s)\n", nb_erreurs);
  exit(nb_erreurs == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}