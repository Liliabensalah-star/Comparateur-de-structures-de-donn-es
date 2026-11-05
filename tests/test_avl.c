/* +----------------------------+ */
/* | FICHIER : tests/test_avl.c | */
/* +----------------------------+ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* Ce que parcourir doit donner : ordre alphabétique, profondeur, rang */
static const char * attendu_mot[] = {"abri", "chat", "lion"};
static const int attendu_ligne[] = {1, 0, 1};
static int nb_visites = 0;
static int parcours_ok = 1;

/* Affiche un mot et sa place, et vérifie qu'elle est la bonne */
static void verifier_mot(const char * mot, int ligne, int colonne, void * contexte) {
  (void)contexte;
  printf("  [ligne %d, colonne %d] %s\n", ligne, colonne, mot);
  if(nb_visites >= 3 || colonne != nb_visites
     || strcmp(mot, attendu_mot[nb_visites]) != 0
     || ligne != attendu_ligne[nb_visites]) {
    parcours_ok = 0;
  }
  ++nb_visites;
}

/* Compte les étapes envoyées à l'animation */
static void compter_etape(TypeEtape type, const char * mot, void * contexte) {
  (void)type;
  (void)mot;
  ++(*(int *)contexte);
}

int main() {
  const char * lib_path = "./plugins/avl.so";
  void * dlptr = NULL;
  const Structure * (*plugin)(void) = NULL;
  const Structure * st = NULL;
  void * arbre = NULL;
  int nb_etapes = 0;

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

  if((arbre = st->creer()) == NULL) {
    fprintf(stderr, "Erreur : creation de l'arbre impossible\n");
    dlclose(dlptr);
    exit(EXIT_FAILURE);
  }

  verifier(st->taille(arbre) == 0, "arbre vide au depart");
  verifier(st->chercher(arbre, "chat") == RES_ABSENT, "recherche dans un arbre vide");
  verifier(st->ajouter(arbre, "chat") == RES_OK, "ajout de chat (racine)");
  verifier(st->ajouter(arbre, "abri") == RES_OK, "ajout de abri (a gauche)");
  verifier(st->ajouter(arbre, "lion") == RES_OK, "ajout de lion (a droite)");
  verifier(st->ajouter(arbre, "chat") == RES_ABSENT, "doublon chat refuse");
  verifier(st->taille(arbre) == 3, "3 mots dans l'arbre");
  verifier(st->chercher(arbre, "abri") == RES_OK, "abri trouve");
  verifier(st->chercher(arbre, "ours") == RES_ABSENT, "ours absent");
  /* 0 + 1 + 1 + 1 (doublon) + 2 (abri) + 2 (ours) = 7 */
  verifier(st->operations(arbre) == 7, "7 comparaisons au total");

  st->observer(arbre, compter_etape, &nb_etapes);
  st->chercher(arbre, "lion");
  st->observer(arbre, NULL, NULL);
  /* visite chat, visite lion, trouve lion */
  verifier(nb_etapes == 3, "3 etapes signalees pour chercher lion");

  printf("Contenu de l'arbre :\n");
  st->parcourir(arbre, verifier_mot, NULL);
  verifier(parcours_ok && nb_visites == 3, "parcours : ordre alphabetique, profondeur et rang justes");

  st->detruire(arbre);
  arbre = NULL;
  dlclose(dlptr);
  dlptr = NULL;

  printf("Bilan : %d erreur(s)\n", nb_erreurs);
  exit(nb_erreurs == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
