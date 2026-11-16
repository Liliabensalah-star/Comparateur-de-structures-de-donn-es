/* FICHIER : include/benchmark.h */
/**
 * @file benchmark.h
 * @brief Banc de test : chronomètre et compte les comparaisons des structures.
 * @author Lilia BEN SALAH
 *
 * Protocole (docs/specifications.md, partie 5) : pour une taille N et une
 * structure, on crée la structure, on ajoute les N premiers mots mélangés,
 * on les cherche tous, on les supprime tous, puis on détruit la structure.
 * La série est répétée plusieurs fois ; le temps gardé est la moyenne.
 */
#ifndef DEF_HEADER_BENCHMARK
#define DEF_HEADER_BENCHMARK

#include <stdio.h>
#include "structure.h"
#include "plugins.h"
#include "dictionnaire.h"

/* Les trois opérations mesurées, dans l'ordre d'une série */
typedef enum {
  OP_AJOUT,
  OP_RECHERCHE,
  OP_SUPPRESSION,
  NB_OPERATIONS
} Operation;

/* Une ligne de résultat (une ligne du CSV) */
typedef struct Mesure Mesure;
struct Mesure {
  const char * structure;   /* nom de la fiche : "Liste", "AVL", "Hachage" */
  Operation operation;
  size_t nb_mots;
  double temps_ms;          /* moyenne sur les répétitions */
  long nb_comparaisons;     /* d'une seule répétition (identique à chaque fois) */
};

/* Réglages d'une campagne de mesures */
typedef struct Config Config;
struct Config {
  const size_t * tailles;   /* tailles à mesurer, dans l'ordre croissant ; 0 = tout le dictionnaire */
  int nb_tailles;
  int repetitions;          /* nombre de séries pour faire la moyenne */
  double limite_ms;         /* au-delà, on ne mesure pas les tailles suivantes pour cette structure */
};

/* Nom d'une opération pour le CSV : "ajout", "recherche", "suppression" */
const char * Benchmark_nom_operation(Operation operation);

/* Mesure une structure pour une taille : remplit mesures[OP_AJOUT..OP_SUPPRESSION] */
/* Renvoie 1 si tout va bien, 0 si la structure n'a pas pu être créée */
int Benchmark_serie(const Structure * st, char ** mots, size_t nb_mots, int repetitions, Mesure mesures[NB_OPERATIONS]);

/* Mesure toutes les structures pour toutes les tailles et écrit le CSV */
/* Renvoie le nombre de lignes écrites, ou -1 si le CSV ne s'ouvre pas */
int Benchmark_lancer(const Plugin plugins[], int nb_plugins, const Dictionnaire * dico, const Config * config, const char * chemin_csv);

#endif