/* +------------------------------+ */
/* | FICHIER : tests/test_liste.c | */
/* +------------------------------+ */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "structure.h"

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

/* Affiche un mot et sa place (donnée à parcourir) */
static void afficher_mot(const char * mot, int ligne, int colonne, void * contexte) {
  (void)contexte;
  printf("  [ligne %d, colonne %d] %s\n", ligne, colonne, mot);
}

int main() {
  const char * lib_path = "./plugins/liste.so";
  void * dlptr = NULL;
  const Structure * (*plugin)(void) = NULL;
  const Structure * st = NULL;
  void * liste = NULL;

  if((dlptr = dlopen(lib_path, RTLD_NOW)) == NULL) {
    fprintf(stderr, "Erreur d'ouverture du plugin \"%s\"\n%s\n", lib_path, dlerror());
    exit(EXIT_FAILURE);
  }
  if((plugin = (const Structure * (*)(void))dlsym(dlptr, "structure_plugin")) == NULL) {
    fprintf(stderr, "Erreur de lecture de \"structure_plugin\"\n%s\n", dlerror());
    dlclose(dlptr);
    exit(EXIT_FAILURE);
  }
  st = plugin();
  printf("Plugin charge : %s\n", st->nom);

  if((liste = st->creer()) == NULL) {
    fprintf(stderr, "Erreur : creation de la liste impossible\n");
    dlclose(dlptr);
    exit(EXIT_FAILURE);
  }

  verifier(st->taille(liste) == 0, "liste vide au depart");
  verifier(st->ajouter(liste, "chat") == RES_OK, "ajout de chat");
  verifier(st->ajouter(liste, "abri") == RES_OK, "ajout de abri");
  verifier(st->ajouter(liste, "chat") == RES_ABSENT, "doublon chat refuse");
  verifier(st->ajouter(liste, "lion") == RES_OK, "ajout de lion");
  verifier(st->taille(liste) == 3, "3 mots dans la liste");
  verifier(st->chercher(liste, "chat") == RES_OK, "chat trouve");
  verifier(st->chercher(liste, "ours") == RES_ABSENT, "ours absent");
  verifier(st->operations(liste) == 11, "11 comparaisons au total");

  printf("Contenu de la liste :\n");
  st->parcourir(liste, afficher_mot, NULL);

  verifier(st->supprimer(liste, "ours") == RES_ABSENT, "supprimer ours (absent) : RES_ABSENT");
  verifier(st->supprimer(liste, "abri") == RES_OK, "supprimer abri (milieu)");
  verifier(st->taille(liste) == 2, "2 mots apres suppression");
  verifier(st->chercher(liste, "abri") == RES_ABSENT, "abri n'est plus trouve");
  verifier(st->supprimer(liste, "abri") == RES_ABSENT, "supprimer abri une 2e fois : RES_ABSENT");
  verifier(st->supprimer(liste, "lion") == RES_OK, "supprimer lion (tete)");
  verifier(st->supprimer(liste, "chat") == RES_OK, "supprimer chat (dernier mot)");
  verifier(st->taille(liste) == 0, "liste vide apres 3 suppressions");
  verifier(st->supprimer(liste, "chat") == RES_ABSENT, "supprimer dans une liste vide : RES_ABSENT");
  verifier(st->ajouter(liste, "abri") == RES_OK, "on peut ajouter de nouveau apres suppression");

  st->detruire(liste);
  liste = NULL;
  dlclose(dlptr);
  dlptr = NULL;

  printf("Bilan : %d erreur(s)\n", nb_erreurs);
  exit(nb_erreurs == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}