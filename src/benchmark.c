/* FICHIER : src/benchmark.c */
/**
 * @file benchmark.c
 * @brief Banc de test : chronomètre et compte les comparaisons des structures.
 * @author Lilia BEN SALAH
 */
#include <stdio.h>
#include <time.h>
#include "benchmark.h"
#include "csv.h"

/* Heure actuelle en millisecondes (horloge monotone : ne recule jamais) */
/* Complexité : O(1) */
static double maintenant_ms(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (double)t.tv_sec * 1000.0 + (double)t.tv_nsec / 1000000.0;
}

/* Complexité : O(1) */
const char * Benchmark_nom_operation(Operation operation) {
  switch(operation) {
    case OP_AJOUT:       return "ajout";
    case OP_RECHERCHE:   return "recherche";
    case OP_SUPPRESSION: return "suppression";
    default:             return "?";
  }
}

/* Applique une opération aux nb_mots premiers mots, en la chronométrant */
/* *comparaisons reçoit le nombre de comparaisons faites pendant l'opération */
/* Complexité : celle de l'opération, nb_mots fois */
static double mesurer(const Structure * st, void * s, Operation operation, char ** mots, size_t nb_mots, long * comparaisons) {
  Resultat (*fonction)(void *, const char *) = NULL;
  long avant = st->operations(s);
  double debut = 0.0;
  size_t i;
  switch(operation) {
    case OP_AJOUT:     fonction = st->ajouter;   break;
    case OP_RECHERCHE: fonction = st->chercher;  break;
    default:           fonction = st->supprimer; break;
  }
  debut = maintenant_ms();
  for(i = 0; i < nb_mots; ++i) {
    fonction(s, mots[i]);
  }
  *comparaisons = st->operations(s) - avant;
  return maintenant_ms() - debut;
}

/* Une série complète, répétée : creer, ajout, recherche, suppression, detruire */
/* Complexité : repetitions x (coût des trois opérations sur nb_mots mots) */
int Benchmark_serie(const Structure * st, char ** mots, size_t nb_mots, int repetitions, Mesure mesures[NB_OPERATIONS]) {
  double total_ms[NB_OPERATIONS] = {0.0, 0.0, 0.0};
  long comparaisons = 0;
  void * s = NULL;
  int r;
  int op;
  for(op = 0; op < NB_OPERATIONS; ++op) {
    mesures[op].structure = st->nom;
    mesures[op].operation = (Operation)op;
    mesures[op].nb_mots = nb_mots;
    mesures[op].nb_comparaisons = 0;
  }
  for(r = 0; r < repetitions; ++r) {
    if((s = st->creer()) == NULL) {
      fprintf(stderr, "Erreur : impossible de creer la structure %s\n", st->nom);
      return 0;
    }
    for(op = 0; op < NB_OPERATIONS; ++op) {
      total_ms[op] += mesurer(st, s, (Operation)op, mots, nb_mots, &comparaisons);
      if(r == 0) {
        mesures[op].nb_comparaisons = comparaisons;
      }
    }
    st->detruire(s);
    s = NULL;
  }
  for(op = 0; op < NB_OPERATIONS; ++op) {
    mesures[op].temps_ms = total_ms[op] / repetitions;
  }
  return 1;
}

/* Pour chaque structure, mesure les tailles dans l'ordre et écrit le CSV */
/* Complexité : dominée par la structure la plus lente (la liste) */
int Benchmark_lancer(const Plugin plugins[], int nb_plugins, const Dictionnaire * dico, const Config * config, const char * chemin_csv) {
  Mesure mesures[NB_OPERATIONS];
  FILE * csv = NULL;
  int nb_lignes = 0;
  int p;
  int t;
  int op;
  size_t nb_mots;
  double pire_ms;

  if((csv = Csv_ouvrir(chemin_csv)) == NULL) {
    return -1;
  }
  for(p = 0; p < nb_plugins; ++p) {
    const Structure * st = plugins[p].fiche;
    for(t = 0; t < config->nb_tailles; ++t) {
      nb_mots = config->tailles[t];
      if(nb_mots == 0 || nb_mots > dico->nb_mots) {
        nb_mots = dico->nb_mots;   /* 0 ou trop grand : tout le dictionnaire */
      }
      printf("%-8s %7zu mots ... ", st->nom, nb_mots);
      fflush(stdout);
      if(!Benchmark_serie(st, dico->mots, nb_mots, config->repetitions, mesures)) {
        break;
      }
      pire_ms = 0.0;
      for(op = 0; op < NB_OPERATIONS; ++op) {
        Csv_ecrire(csv, &mesures[op]);
        ++nb_lignes;
        if(mesures[op].temps_ms > pire_ms) {
          pire_ms = mesures[op].temps_ms;
        }
      }
      printf("ajout %.3f ms, recherche %.3f ms\n", mesures[OP_AJOUT].temps_ms, mesures[OP_RECHERCHE].temps_ms);
      if(nb_mots == dico->nb_mots) {
        break;   /* tout le dictionnaire est mesuré : inutile d'aller plus loin */
      }
      if(pire_ms > config->limite_ms) {
        printf("%-8s trop lente au-dela de %zu mots : tailles suivantes ignorees\n", st->nom, nb_mots);
        break;
      }
    }
  }
  Csv_fermer(&csv);
  return nb_lignes;
}