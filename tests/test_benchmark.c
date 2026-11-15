/* +----------------------------------+ */
/* | FICHIER : tests/test_benchmark.c | */
/* +----------------------------------+ */
/*
 * Utilisation :
 *   ./test_benchmark [dictionnaire]            petit test rapide (vérifications)
 *   ./test_benchmark [dictionnaire] complet    vraies mesures -> resultats/mesures.csv
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "benchmark.h"

#define NB_PLUGINS 3
#define DICO_DEFAUT "/usr/share/dict/french"

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

/* Compte les lignes d'un fichier texte (-1 s'il ne s'ouvre pas) */
static int compter_lignes(const char * chemin, char * premiere, size_t taille) {
  FILE * f = NULL;
  char ligne[256];
  int n = 0;
  if((f = fopen(chemin, "r")) == NULL) {
    return -1;
  }
  while(fgets(ligne, sizeof(ligne), f) != NULL) {
    if(n == 0) {
      strncpy(premiere, ligne, taille - 1);
      premiere[taille - 1] = '\0';
    }
    ++n;
  }
  fclose(f);
  return n;
}

/* Vérifications sur une petite série de 1000 mots */
static void test_serie(const Plugin plugins[], const Dictionnaire * dico) {
  Mesure liste[NB_OPERATIONS];
  Mesure avl[NB_OPERATIONS];
  Mesure hachage[NB_OPERATIONS];
  Mesure encore[NB_OPERATIONS];

  verifier(Benchmark_serie(plugins[0].fiche, dico->mots, 1000, 2, liste), "serie Liste 1000 mots");
  verifier(Benchmark_serie(plugins[1].fiche, dico->mots, 1000, 2, avl), "serie AVL 1000 mots");
  verifier(Benchmark_serie(plugins[2].fiche, dico->mots, 1000, 2, hachage), "serie Hachage 1000 mots");

  /* Liste, ajout en tête avec recherche du doublon : 0 + 1 + ... + 999 */
  verifier(liste[OP_AJOUT].nb_comparaisons == 499500, "Liste ajout 1000 : 499500 comparaisons (calcul a la main)");
  /* Recherche dans l'ordre d'ajout : le 1er mot est au fond, 1000 + 999 + ... + 1 */
  verifier(liste[OP_RECHERCHE].nb_comparaisons == 500500, "Liste recherche 1000 : 500500 comparaisons");
  verifier(avl[OP_RECHERCHE].nb_comparaisons < 1000 * 12, "AVL recherche : moins de 12 comparaisons par mot (log2 1000 = 10)");
  verifier(hachage[OP_RECHERCHE].nb_comparaisons < avl[OP_RECHERCHE].nb_comparaisons, "Hachage : moins de comparaisons que l'AVL");
  verifier(liste[OP_AJOUT].temps_ms >= 0.0 && avl[OP_AJOUT].temps_ms >= 0.0, "temps positifs");

  /* Même graine, même ordre : le nombre de comparaisons ne change pas */
  Benchmark_serie(plugins[1].fiche, dico->mots, 1000, 1, encore);
  verifier(encore[OP_AJOUT].nb_comparaisons == avl[OP_AJOUT].nb_comparaisons, "mesure reproductible (memes comparaisons)");
  verifier(strcmp(Benchmark_nom_operation(OP_SUPPRESSION), "suppression") == 0, "nom d'operation pour le CSV");
}

/* Petite campagne complète : vérifie le fichier CSV produit */
static void test_csv(const Plugin plugins[], const Dictionnaire * dico) {
  const size_t tailles[] = {100, 500};
  Config config = {tailles, 2, 2, 1000.0};
  const char * chemin = "resultats/test_mesures.csv";
  char entete[128] = "";
  int nb_lignes = 0;

  nb_lignes = Benchmark_lancer(plugins, NB_PLUGINS, dico, &config, chemin);
  /* 3 structures x 2 tailles x 3 opérations */
  verifier(nb_lignes == 18, "18 mesures ecrites (3 structures x 2 tailles x 3 operations)");
  verifier(compter_lignes(chemin, entete, sizeof(entete)) == 19, "CSV : 1 ligne d'en-tete + 18 lignes");
  verifier(strcmp(entete, "structure,operation,nb_mots,temps_ms,nb_comparaisons\n") == 0, "CSV : en-tete conforme aux specifications");
  remove(chemin);
}

/* Le vrai protocole des spécifications (partie 5) */
static void campagne_complete(const Plugin plugins[], const Dictionnaire * dico) {
  const size_t tailles[] = {1000, 5000, 10000, 50000, 100000, 0};
  Config config = {tailles, 6, 5, 2000.0};
  int nb_lignes = Benchmark_lancer(plugins, NB_PLUGINS, dico, &config, "resultats/mesures.csv");
  printf("%d mesures ecrites dans resultats/mesures.csv\n", nb_lignes);
  if(nb_lignes <= 0) {
    ++nb_erreurs;
  }
}

int main(int argc, char * argv[]) {
  const char * chemins[NB_PLUGINS] = {"./plugins/liste.so", "./plugins/avl.so", "./plugins/hachage.so"};
  const char * chemin_dico = (argc > 1) ? argv[1] : DICO_DEFAUT;
  int complet = (argc > 2 && strcmp(argv[2], "complet") == 0);
  Plugin plugins[NB_PLUGINS];
  Dictionnaire * dico = NULL;
  int nb_charges = 0;
  int i;

  for(i = 0; i < NB_PLUGINS; ++i) {
    nb_charges += plugin_charger(&plugins[i], chemins[i]);
  }
  if(nb_charges < NB_PLUGINS || (dico = Dictionnaire_charger(chemin_dico)) == NULL) {
    fprintf(stderr, "Erreur : plugins ou dictionnaire introuvables\n");
    for(i = 0; i < NB_PLUGINS; ++i) {
      plugin_fermer(&plugins[i]);
    }
    exit(EXIT_FAILURE);
  }
  Dictionnaire_melanger(dico, 42);
  printf("Dictionnaire : %zu mots (melanges, graine 42)\n", dico->nb_mots);

  if(complet) {
    campagne_complete(plugins, dico);
  } else {
    test_serie(plugins, dico);
    test_csv(plugins, dico);
  }

  Dictionnaire_free(&dico);
  for(i = 0; i < NB_PLUGINS; ++i) {
    plugin_fermer(&plugins[i]);
  }
  printf("Bilan : %d erreur(s)\n", nb_erreurs);
  exit(nb_erreurs == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}