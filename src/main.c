/* FICHIER : src/main.c */
/**
 * @file main.c
 * @brief Programme principal du comparateur de structures de données.
 * @author Lilia BEN SALAH et Javerichalle MFINA (tâche L, intégration du prototype)
 *
 * Étapes :
 *   1. charger tous les plugins .so du dossier plugins/ ;
 *   2. lire et mélanger le dictionnaire (graine 42) ;
 *   3. lancer le banc de test, qui écrit resultats/mesures.csv ;
 *   4. relire le CSV et ouvrir la fenêtre des courbes.
 *
 * Utilisation :
 *   ./comparateur                 mesures complètes, puis courbes
 *   ./comparateur -r              mesures rapides (démonstration), puis courbes
 *   ./comparateur -c              courbes seulement (relit le dernier CSV)
 *   ./comparateur -d fichier      utilise un autre dictionnaire
 *   ./comparateur -h              aide
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "plugins.h"
#include "dictionnaire.h"
#include "benchmark.h"
#include "affichage.h"

#define DOSSIER_PLUGINS "plugins"
#define DICO_DEFAUT "/usr/share/dict/french"
#define CHEMIN_CSV "resultats/mesures.csv"
#define NB_PLUGINS_MAX 16
#define NOM_FICHIER_MAX 256   /* taille de d_name sous Linux */
#define CHEMIN_MAX 512
#define GRAINE 42

/* Ce que l'utilisateur a demandé sur la ligne de commande */
typedef struct Options Options;
struct Options {
  const char * dictionnaire;
  int rapide;            /* 1 : peu de tailles et une seule répétition */
  int courbes_seulement; /* 1 : pas de mesures, on relit le CSV existant */
};

/* Affiche l'aide */
/* Complexité : O(1) */
static void afficher_aide(const char * programme) {
  printf("Utilisation : %s [-r] [-c] [-d dictionnaire]\n", programme);
  printf("  -r  mesures rapides (1000, 5000, 10000 mots, 1 repetition)\n");
  printf("  -c  courbes seulement : relit %s sans refaire les mesures\n", CHEMIN_CSV);
  printf("  -d  dictionnaire a utiliser (defaut : %s)\n", DICO_DEFAUT);
}

/* Lit les options ; renvoie 1 si on continue, 0 si l'aide a été affichée (-h), -1 si erreur */
/* Complexité : O(argc) */
static int lire_options(int argc, char * argv[], Options * options) {
  int i;
  options->dictionnaire = DICO_DEFAUT;
  options->rapide = 0;
  options->courbes_seulement = 0;
  for(i = 1; i < argc; ++i) {
    if(strcmp(argv[i], "-r") == 0) {
      options->rapide = 1;
    } else if(strcmp(argv[i], "-c") == 0) {
      options->courbes_seulement = 1;
    } else if(strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
      options->dictionnaire = argv[++i];
    } else if(strcmp(argv[i], "-h") == 0) {
      afficher_aide(argv[0]);
      return 0;
    } else {
      fprintf(stderr, "Option inconnue : %s\n", argv[i]);
      afficher_aide(argv[0]);
      return -1;
    }
  }
  return 1;
}

/* Compare deux noms de fichiers (pour trier les plugins avec qsort) */
/* Complexité : O(longueur des noms) */
static int comparer_noms(const void * a, const void * b) {
  return strcmp((const char *)a, (const char *)b);
}

/* Indique si le nom se termine par ".so" */
/* Complexité : O(longueur du nom) */
static int est_plugin(const char * nom) {
  size_t n = strlen(nom);
  return n > 3 && strcmp(nom + n - 3, ".so") == 0;
}

/* Charge tous les fichiers .so du dossier, dans l'ordre alphabétique */
/* Ajouter une structure = déposer un .so dans le dossier, sans toucher à ce code */
/* Renvoie le nombre de plugins chargés */
/* Complexité : O(k log k), k = nombre de fichiers .so */
static int charger_plugins(const char * dossier, Plugin plugins[], int nb_max) {
  char noms[NB_PLUGINS_MAX][NOM_FICHIER_MAX];
  char chemin[CHEMIN_MAX];
  DIR * rep = NULL;
  struct dirent * entree = NULL;
  int nb_fichiers = 0;
  int nb_charges = 0;
  int i;
  if((rep = opendir(dossier)) == NULL) {
    perror(dossier);
    return 0;
  }
  while((entree = readdir(rep)) != NULL && nb_fichiers < nb_max) {
    if(est_plugin(entree->d_name) && strlen(entree->d_name) < NOM_FICHIER_MAX) {
      strcpy(noms[nb_fichiers], entree->d_name);
      ++nb_fichiers;
    }
  }
  closedir(rep);
  qsort(noms, (size_t)nb_fichiers, sizeof(noms[0]), comparer_noms);
  for(i = 0; i < nb_fichiers; ++i) {
    if(snprintf(chemin, sizeof(chemin), "./%s/%s", dossier, noms[i]) >= (int)sizeof(chemin)) {
      continue;   /* chemin trop long : on ignore ce fichier */
    }
    if(plugin_charger(&plugins[nb_charges], chemin)) {
      printf("Plugin charge : %-8s (%s)\n", plugins[nb_charges].fiche->nom, chemin);
      ++nb_charges;
    }
  }
  return nb_charges;
}

/* Ferme tous les plugins */
/* Complexité : O(nb) */
static void fermer_plugins(Plugin plugins[], int nb) {
  int i;
  for(i = 0; i < nb; ++i) {
    plugin_fermer(&plugins[i]);
  }
}

/* Charge les plugins et le dictionnaire, puis lance les mesures */
/* Renvoie 1 si le CSV a été écrit, 0 sinon */
/* Complexité : celle du banc de test */
static int mesurer(const Options * options) {
  static const size_t tailles_completes[] = {1000, 5000, 10000, 50000, 100000, 0};
  static const size_t tailles_rapides[] = {1000, 5000, 10000};
  Config config;
  Plugin plugins[NB_PLUGINS_MAX];
  Dictionnaire * dico = NULL;
  int nb_plugins = 0;
  int nb_lignes = 0;

  if((nb_plugins = charger_plugins(DOSSIER_PLUGINS, plugins, NB_PLUGINS_MAX)) == 0) {
    fprintf(stderr, "Aucun plugin dans %s/ : lancez d'abord \"make plugins\"\n", DOSSIER_PLUGINS);
    return 0;
  }
  if((dico = Dictionnaire_charger(options->dictionnaire)) == NULL) {
    fermer_plugins(plugins, nb_plugins);
    return 0;
  }
  Dictionnaire_melanger(dico, GRAINE);
  printf("Dictionnaire : %zu mots (%s, melanges avec la graine %d)\n",
         dico->nb_mots, options->dictionnaire, GRAINE);

  if(options->rapide) {
    config.tailles = tailles_rapides;
    config.nb_tailles = 3;
    config.repetitions = 1;
  } else {
    config.tailles = tailles_completes;
    config.nb_tailles = 6;
    config.repetitions = 5;
  }
  config.limite_ms = 2000.0;

  nb_lignes = Benchmark_lancer(plugins, nb_plugins, dico, &config, CHEMIN_CSV);
  if(nb_lignes > 0) {
    printf("%d mesures ecrites dans %s\n", nb_lignes, CHEMIN_CSV);
  }

  Dictionnaire_free(&dico);
  fermer_plugins(plugins, nb_plugins);
  return nb_lignes > 0;
}

/* Relit le CSV et ouvre la fenêtre des courbes */
/* Renvoie 1 si tout s'est bien passé, 0 sinon */
/* Complexité : celle de l'affichage */
static int afficher(void) {
  Releves * releves = NULL;
  int ok = 0;
  if((releves = Releves_charger(CHEMIN_CSV)) == NULL) {
    fprintf(stderr, "Pas de mesures : lancez d'abord \"./comparateur\" ou \"./comparateur -r\"\n");
    return 0;
  }
  printf("Courbes : 1/2/3 operation, T temps/comparaisons, L echelle, Echap pour quitter\n");
  ok = Affichage_courbes(releves);
  Releves_free(&releves);
  return ok;
}

int main(int argc, char * argv[]) {
  Options options;
  int lecture = lire_options(argc, argv, &options);
  if(lecture <= 0) {
    exit(lecture == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
  }
  if(!options.courbes_seulement && !mesurer(&options)) {
    exit(EXIT_FAILURE);
  }
  if(!afficher()) {
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}