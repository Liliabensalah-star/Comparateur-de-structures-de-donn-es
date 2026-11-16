/* FICHIER : include/csv.h */
/**
 * @file csv.h
 * @brief Écriture des mesures du banc de test au format CSV.
 * @author Lilia BEN SALAH
 *
 * Format (docs/specifications.md, partie 6) :
 *   structure,operation,nb_mots,temps_ms,nb_comparaisons
 *   AVL,ajout,1000,0.312,9976
 */
#ifndef DEF_HEADER_CSV
#define DEF_HEADER_CSV

#include <stdio.h>
#include "benchmark.h"

/* Crée (ou réécrit) le fichier et y écrit la ligne d'en-tête ; NULL en cas d'erreur */
FILE * Csv_ouvrir(const char * chemin);

/* Écrit une mesure sur une ligne */
void Csv_ecrire(FILE * fichier, const Mesure * mesure);

/* Ferme le fichier et met le pointeur à NULL */
void Csv_fermer(FILE ** fichier);

#endif