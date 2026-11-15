/* FICHIER : include/dictionnaire.h */
#ifndef DEF_HEADER_DICTIONNAIRE
#define DEF_HEADER_DICTIONNAIRE

#include <stddef.h>

#define MOT_LONGUEUR_MAX 100

/* Les mots lus dans un fichier, dans un tableau qui grandit */
typedef struct Dictionnaire Dictionnaire;
struct Dictionnaire {
  char ** mots;
  size_t nb_mots;
  size_t capacite;
};

/* Lit un mot par ligne ; NULL si le fichier ne s'ouvre pas ou si la mémoire manque */
Dictionnaire * Dictionnaire_charger(const char * chemin);

/* Mélange les mots (Fisher-Yates) avec une graine fixe */
void Dictionnaire_melanger(Dictionnaire * dico, unsigned int graine);

/* Libère tous les mots, puis le dictionnaire, et met le pointeur à NULL */
void Dictionnaire_free(Dictionnaire ** dico);

#endif