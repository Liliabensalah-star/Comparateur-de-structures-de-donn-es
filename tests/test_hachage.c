/* FICHIER : tests/test_hachage.c */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "structure.h"

#define NB_MOTS_GRAND_TEST 5000

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
  printf("  [case %d, position %d] %s\n", ligne, colonne, mot);
}

/* Compte les mots et retient la plus grande case vue (donnée à parcourir) */
static void compter_mot(const char * mot, int ligne, int colonne, void * contexte) {
  int * compteurs = (int *)contexte;
  (void)mot;
  (void)colonne;
  ++compteurs[0];
  if(ligne > compteurs[1]) {
    compteurs[1] = ligne;
  }
}

/* Petit test : 3 mots, un doublon, deux recherches */
static void test_petit(const Structure * st) {
  void * table = NULL;
  if((table = st->creer()) == NULL) {
    verifier(0, "creation de la table");
    return;
  }
  verifier(st->taille(table) == 0, "table vide au depart");
  verifier(st->chercher(table, "chat") == RES_ABSENT, "recherche dans une table vide");
  verifier(st->ajouter(table, "chat") == RES_OK, "ajout de chat");
  verifier(st->ajouter(table, "abri") == RES_OK, "ajout de abri");
  verifier(st->ajouter(table, "chat") == RES_ABSENT, "doublon chat refuse");
  verifier(st->ajouter(table, "lion") == RES_OK, "ajout de lion");
  verifier(st->taille(table) == 3, "3 mots dans la table");
  verifier(st->chercher(table, "chat") == RES_OK, "chat trouve");
  verifier(st->chercher(table, "ours") == RES_ABSENT, "ours absent");
  verifier(st->operations(table) < 11, "moins de comparaisons que la liste (11)");
  printf("  comparaisons : %ld\n", st->operations(table));
  printf("Contenu de la table :\n");
  st->parcourir(table, afficher_mot, NULL);
  st->detruire(table);
}

/* Grand test : assez de mots pour que la table double plusieurs fois */
static void test_grand(const Structure * st) {
  void * table = NULL;
  char mot[32];
  int i;
  int trouves = 0;
  int compteurs[2] = {0, 0};
  if((table = st->creer()) == NULL) {
    verifier(0, "creation de la grande table");
    return;
  }
  for(i = 0; i < NB_MOTS_GRAND_TEST; ++i) {
    sprintf(mot, "mot%d", i);
    st->ajouter(table, mot);
  }
  verifier(st->taille(table) == NB_MOTS_GRAND_TEST, "5000 mots ranges");
  for(i = 0; i < NB_MOTS_GRAND_TEST; ++i) {
    sprintf(mot, "mot%d", i);
    if(st->chercher(table, mot) == RES_OK) {
      ++trouves;
    }
  }
  verifier(trouves == NB_MOTS_GRAND_TEST, "les 5000 mots retrouves apres agrandissement");
  verifier(st->ajouter(table, "mot42") == RES_ABSENT, "doublon refuse dans la grande table");
  st->parcourir(table, compter_mot, compteurs);
  verifier(compteurs[0] == NB_MOTS_GRAND_TEST, "parcourir donne les 5000 mots");
  verifier(compteurs[1] >= 1024, "la table a grandi (plus de 1024 cases)");
  printf("  comparaisons pour 5000 ajouts + 5000 recherches : %ld\n", st->operations(table));
  st->detruire(table);
}

int main() {
  const char * lib_path = "./plugins/hachage.so";
  void * dlptr = NULL;
  const Structure * (*plugin)(void) = NULL;
  const Structure * st = NULL;

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

  test_petit(st);
  test_grand(st);

  dlclose(dlptr);
  dlptr = NULL;

  printf("Bilan : %d erreur(s)\n", nb_erreurs);
  exit(nb_erreurs == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}