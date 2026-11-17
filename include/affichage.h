/* FICHIER : include/affichage.h */
#ifndef DEF_HEADER_AFFICHAGE
#define DEF_HEADER_AFFICHAGE

#include <stddef.h>
#include <SDL/SDL.h>

#define NOM_MAX 32

/* Une ligne lue dans le CSV : structure,operation,nb_mots,temps_ms,nb_comparaisons */
typedef struct Releve Releve;
struct Releve {
  char structure[NOM_MAX];
  char operation[NOM_MAX];
  long nb_mots;
  double temps_ms;
  long nb_comparaisons;
};

/* Toutes les lignes du CSV, dans un tableau qui grandit */
typedef struct Releves Releves;
struct Releves {
  Releve * tab;
  size_t nb;
  size_t capacite;
};

/* Ce que la fenêtre montre : quelle opération, quelle mesure, quelle échelle */
typedef struct Vue Vue;
struct Vue {
  int operation;   /* 0 : ajout, 1 : recherche, 2 : suppression */
  int comparaisons; /* 0 : temps en ms, 1 : nombre de comparaisons */
  int logarithme;  /* 0 : échelle linéaire, 1 : échelle logarithmique */
};

/* Lit le CSV des mesures ; NULL si le fichier ne s'ouvre pas ou si la mémoire manque */
Releves * Releves_charger(const char * chemin);

/* Libère les relevés et met le pointeur à NULL */
void Releves_free(Releves ** releves);

/* Dessine les courbes de la vue sur l'écran */
void Affichage_dessiner(SDL_Surface * ecran, const Releves * releves, const Vue * vue);

/* Ouvre la fenêtre et affiche les courbes jusqu'à Échap ; 1 si réussi, 0 sinon */
int Affichage_courbes(const Releves * releves);

#endif