/* FICHIER : src/csv.c */
/**
 * @file csv.c
 * @brief Écriture des mesures du banc de test au format CSV.
 * @author Lilia BEN SALAH
 */
#include <stdio.h>
#include "csv.h"

/* Crée le fichier et écrit l'en-tête des colonnes */
/* Complexité : O(1) */
FILE * Csv_ouvrir(const char * chemin) {
  FILE * fichier = NULL;
  if((fichier = fopen(chemin, "w")) == NULL) {
    perror(chemin);
    return NULL;
  }
  fprintf(fichier, "structure,operation,nb_mots,temps_ms,nb_comparaisons\n");
  return fichier;
}

/* Écrit une mesure : temps avec 3 décimales et un point décimal */
/* Complexité : O(1) */
void Csv_ecrire(FILE * fichier, const Mesure * mesure) {
  fprintf(fichier, "%s,%s,%zu,%.3f,%ld\n",
          mesure->structure,
          Benchmark_nom_operation(mesure->operation),
          mesure->nb_mots,
          mesure->temps_ms,
          mesure->nb_comparaisons);
}

/* Ferme le fichier */
/* Complexité : O(1) */
void Csv_fermer(FILE ** fichier) {
  if(*fichier != NULL) {
    fclose(*fichier);
  }
  *fichier = NULL;
}