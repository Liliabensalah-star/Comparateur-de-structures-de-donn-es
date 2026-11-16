/* FICHIER : src/affichage.c */
/**
 * @file affichage.c
 * @brief Fenêtre SDL et courbes des mesures (lecture du CSV).
 * @author Javerichalle MFINA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include "affichage.h"

#define LARGEUR 900
#define HAUTEUR 600
#define MARGE_GAUCHE 90
#define MARGE_DROITE 170
#define MARGE_HAUT 60
#define MARGE_BAS 70
#define NB_GRADUATIONS 5
#define NB_STRUCTURES_MAX 8
#define POINTS_MAX 64

static const char * NOMS_OPERATIONS[3] = {"ajout", "recherche", "suppression"};

/* Les limites des valeurs dessinées, après passage à l'échelle */
typedef struct Bornes Bornes;
struct Bornes {
  double xmin;
  double xmax;
  double ymin;
  double ymax;
};

/* Agrandit le tableau de relevés s'il est plein */
/* Renvoie 1 si réussi, 0 si la mémoire manque */
/* Complexité : O(n) */
static int Releves_update(Releves * releves) {
  Releve * tmp = NULL;
  size_t capacite;
  if(releves->nb < releves->capacite) {
    return 1;
  }
  capacite = releves->nb * 2 + 10;
  if((tmp = (Releve *)realloc(releves->tab, capacite * sizeof(Releve))) == NULL) {
    return 0;
  }
  releves->tab = tmp;
  releves->capacite = capacite;
  return 1;
}

/* Lit le CSV des mesures ; NULL si le fichier ne s'ouvre pas ou si la mémoire manque */
/* Complexité : O(n) */
Releves * Releves_charger(const char * chemin) {
  Releves * res = NULL;
  FILE * fich = NULL;
  Releve ligne;
  if((fich = fopen(chemin, "r")) == NULL) {
    fprintf(stderr, "Releves_charger : impossible d'ouvrir \"%s\"\n", chemin);
    return NULL;
  }
  if((res = (Releves *)malloc(sizeof(Releves))) == NULL) {
    fprintf(stderr, "Releves_charger : erreur d'allocation\n");
    fclose(fich);
    return NULL;
  }
  res->tab = NULL;
  res->nb = 0;
  res->capacite = 0;
  if(fscanf(fich, "%*[^\n]") == EOF) {
    fclose(fich);
    return res;
  }
  while(fscanf(fich, " %31[^,],%31[^,],%ld,%lf,%ld", ligne.structure, ligne.operation,
                &ligne.nb_mots, &ligne.temps_ms, &ligne.nb_comparaisons) == 5) {
    if(!Releves_update(res)) {
      fprintf(stderr, "Releves_charger : erreur d'allocation\n");
      fclose(fich);
      Releves_free(&res);
      return NULL;
    }
    res->tab[res->nb] = ligne;
    ++(res->nb);
  }
  fclose(fich);
  return res;
}

/* Libère les relevés et met le pointeur à NULL */
/* Complexité : O(1) */
void Releves_free(Releves ** releves) {
  if(releves == NULL || *releves == NULL) {
    return;
  }
  free((*releves)->tab);
  free(*releves);
  *releves = NULL;
}

/* Couleur d'une structure : la même partout dans le projet */
/* Complexité : O(1) */
static void couleur_structure(const char * nom, Uint8 * r, Uint8 * g, Uint8 * b) {
  if(strcmp(nom, "Liste") == 0) {
    *r = 29; *g = 95; *b = 191;
  } else if(strcmp(nom, "AVL") == 0) {
    *r = 154; *g = 59; *b = 139;
  } else if(strcmp(nom, "Hachage") == 0) {
    *r = 15; *g = 138; *b = 95;
  } else {
    *r = 110; *g = 110; *b = 120;
  }
}

/* Valeur dessinée en hauteur pour un relevé : temps ou comparaisons */
/* Complexité : O(1) */
static double Releve_valeur(const Releve * releve, const Vue * vue) {
  if(vue->comparaisons) {
    return (double)releve->nb_comparaisons;
  }
  return releve->temps_ms;
}

/* Indique si le relevé fait partie de la vue et peut être dessiné */
/* Complexité : O(1) */
static int Releve_visible(const Releve * releve, const Vue * vue) {
  if(strcmp(releve->operation, NOMS_OPERATIONS[vue->operation]) != 0) {
    return 0;
  }
  if(vue->logarithme && (releve->nb_mots <= 0 || Releve_valeur(releve, vue) <= 0)) {
    return 0;
  }
  return 1;
}

/* Passe une valeur à l'échelle choisie (logarithme en base 10 ou non) */
/* Complexité : O(1) */
static double Vue_echelle(double valeur, const Vue * vue) {
  if(vue->logarithme) {
    return log10(valeur);
  }
  return valeur;
}

/* Arrondit vers le haut à une valeur ronde : 1, 2 ou 5 fois une puissance de 10 */
/* Complexité : O(1) */
static double arrondi_rond(double valeur) {
  double puissance;
  double rapport;
  if(valeur <= 0) {
    return 1;
  }
  puissance = pow(10, floor(log10(valeur)));
  rapport = valeur / puissance;
  if(rapport <= 1) return puissance;
  if(rapport <= 2) return 2 * puissance;
  if(rapport <= 5) return 5 * puissance;
  return 10 * puissance;
}

/* Calcule les limites des points visibles ; renvoie 0 s'il n'y en a aucun */
/* Complexité : O(n) */
static int Bornes_calculer(Bornes * bornes, const Releves * releves, const Vue * vue) {
  size_t i;
  double x;
  double y;
  int trouve = 0;
  for(i = 0; i < releves->nb; ++i) {
    if(!Releve_visible(releves->tab + i, vue)) {
      continue;
    }
    x = Vue_echelle((double)releves->tab[i].nb_mots, vue);
    y = Vue_echelle(Releve_valeur(releves->tab + i, vue), vue);
    if(!trouve || x < bornes->xmin) bornes->xmin = x;
    if(!trouve || x > bornes->xmax) bornes->xmax = x;
    if(!trouve || y < bornes->ymin) bornes->ymin = y;
    if(!trouve || y > bornes->ymax) bornes->ymax = y;
    trouve = 1;
  }
  if(!trouve) {
    return 0;
  }
  if(vue->logarithme) {
    bornes->xmin = floor(bornes->xmin);
    bornes->xmax = ceil(bornes->xmax);
    bornes->ymin = floor(bornes->ymin);
    bornes->ymax = ceil(bornes->ymax);
  } else {
    bornes->xmin = 0;
    bornes->ymin = 0;
    bornes->xmax = arrondi_rond(bornes->xmax);
    bornes->ymax = arrondi_rond(bornes->ymax);
  }
  if(bornes->xmax <= bornes->xmin) bornes->xmax = bornes->xmin + 1;
  if(bornes->ymax <= bornes->ymin) bornes->ymax = bornes->ymin + 1;
  return 1;
}

/* Position à l'écran d'une valeur, en largeur puis en hauteur */
/* Complexité : O(1) */
static Sint16 ecran_x(double x, const Bornes * bornes) {
  return (Sint16)(MARGE_GAUCHE + (x - bornes->xmin) / (bornes->xmax - bornes->xmin)
                  * (LARGEUR - MARGE_GAUCHE - MARGE_DROITE));
}

static Sint16 ecran_y(double y, const Bornes * bornes) {
  return (Sint16)(HAUTEUR - MARGE_BAS - (y - bornes->ymin) / (bornes->ymax - bornes->ymin)
                  * (HAUTEUR - MARGE_HAUT - MARGE_BAS));
}

/* Écrit un nombre en court : 300k, 1.5M, 12, 0.25 */
/* Complexité : O(1) */
static void formater(char * texte, size_t taille, double valeur) {
  if(valeur >= 1000000) {
    snprintf(texte, taille, "%gM", valeur / 1000000);
  } else if(valeur >= 1000) {
    snprintf(texte, taille, "%gk", valeur / 1000);
  } else {
    snprintf(texte, taille, "%g", valeur);
  }
}

/* Dessine les axes, la grille et les graduations */
/* En échelle log, une graduation par puissance de 10 */
/* Complexité : O(1) */
static void dessiner_axes(SDL_Surface * ecran, const Bornes * bornes, const Vue * vue) {
  char texte[32];
  int k;
  int nx = vue->logarithme ? (int)(bornes->xmax - bornes->xmin) : NB_GRADUATIONS;
  int ny = vue->logarithme ? (int)(bornes->ymax - bornes->ymin) : NB_GRADUATIONS;
  double valeur;
  Sint16 position;
  for(k = 0; k <= nx; ++k) {
    valeur = bornes->xmin + k * (bornes->xmax - bornes->xmin) / nx;
    position = ecran_x(valeur, bornes);
    lineRGBA(ecran, position, MARGE_HAUT, position, HAUTEUR - MARGE_BAS, 225, 228, 235, 255);
    formater(texte, sizeof(texte), vue->logarithme ? pow(10, valeur) : valeur);
    stringRGBA(ecran, position - 12, HAUTEUR - MARGE_BAS + 10, texte, 60, 64, 75, 255);
  }
  for(k = 0; k <= ny; ++k) {
    valeur = bornes->ymin + k * (bornes->ymax - bornes->ymin) / ny;
    position = ecran_y(valeur, bornes);
    lineRGBA(ecran, MARGE_GAUCHE, position, LARGEUR - MARGE_DROITE, position, 225, 228, 235, 255);
    formater(texte, sizeof(texte), vue->logarithme ? pow(10, valeur) : valeur);
    stringRGBA(ecran, 10, position - 4, texte, 60, 64, 75, 255);
  }
  lineRGBA(ecran, MARGE_GAUCHE, HAUTEUR - MARGE_BAS, LARGEUR - MARGE_DROITE, HAUTEUR - MARGE_BAS, 30, 30, 40, 255);
  lineRGBA(ecran, MARGE_GAUCHE, MARGE_HAUT, MARGE_GAUCHE, HAUTEUR - MARGE_BAS, 30, 30, 40, 255);
  stringRGBA(ecran, LARGEUR - MARGE_DROITE - 110, HAUTEUR - MARGE_BAS + 28, "nombre de mots", 30, 30, 40, 255);
  stringRGBA(ecran, 10, MARGE_HAUT - 20, vue->comparaisons ? "comparaisons" : "temps (ms)", 30, 30, 40, 255);
}

/* Compare deux relevés par nombre de mots (pour qsort) */
/* Complexité : O(1) */
static int comparer_nb_mots(const void * a, const void * b) {
  const Releve * m1 = *(const Releve * const *)a;
  const Releve * m2 = *(const Releve * const *)b;
  if(m1->nb_mots < m2->nb_mots) return -1;
  if(m1->nb_mots > m2->nb_mots) return 1;
  return 0;
}

/* Dessine la courbe d'une structure : points reliés par ordre de taille */
/* Complexité : O(n log n) (tri des points avec qsort) */
static void dessiner_courbe(SDL_Surface * ecran, const Releves * releves, const Vue * vue,
                            const Bornes * bornes, const char * nom) {
  const Releve * points[POINTS_MAX];
  int nb = 0;
  int i;
  size_t k;
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Sint16 x;
  Sint16 y;
  for(k = 0; k < releves->nb && nb < POINTS_MAX; ++k) {
    if(strcmp(releves->tab[k].structure, nom) == 0 && Releve_visible(releves->tab + k, vue)) {
      points[nb] = releves->tab + k;
      ++nb;
    }
  }
  qsort(points, (size_t)nb, sizeof(const Releve *), comparer_nb_mots);
  couleur_structure(nom, &r, &g, &b);
  for(i = 0; i < nb; ++i) {
    x = ecran_x(Vue_echelle((double)points[i]->nb_mots, vue), bornes);
    y = ecran_y(Vue_echelle(Releve_valeur(points[i], vue), vue), bornes);
    if(i > 0) {
      lineRGBA(ecran, ecran_x(Vue_echelle((double)points[i - 1]->nb_mots, vue), bornes),
               ecran_y(Vue_echelle(Releve_valeur(points[i - 1], vue), vue), bornes), x, y, r, g, b, 255);
    }
    filledCircleRGBA(ecran, x, y, 4, r, g, b, 255);
  }
}

/* Dessine les courbes de la vue sur l'écran */
/* Complexité : O(n log n), n = nombre de relevés */
void Affichage_dessiner(SDL_Surface * ecran, const Releves * releves, const Vue * vue) {
  const char * noms[NB_STRUCTURES_MAX];
  int nb_noms = 0;
  int i;
  size_t k;
  Bornes bornes = {0, 0, 0, 0};
  char titre[96];
  Uint8 r;
  Uint8 g;
  Uint8 b;
  SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 250, 250, 252));
  snprintf(titre, sizeof(titre), "%s - %s - echelle %s", NOMS_OPERATIONS[vue->operation],
           vue->comparaisons ? "comparaisons" : "temps (ms)", vue->logarithme ? "log" : "lineaire");
  stringRGBA(ecran, MARGE_GAUCHE, 20, titre, 30, 30, 40, 255);
  stringRGBA(ecran, 10, HAUTEUR - 18, "1/2/3 : operation   T : temps/comparaisons   L : echelle   Echap : quitter",
             90, 95, 110, 255);
  if(!Bornes_calculer(&bornes, releves, vue)) {
    stringRGBA(ecran, MARGE_GAUCHE, HAUTEUR / 2, "Aucune mesure pour cette operation", 30, 30, 40, 255);
    SDL_Flip(ecran);
    return;
  }
  dessiner_axes(ecran, &bornes, vue);
  for(k = 0; k < releves->nb; ++k) {
    for(i = 0; i < nb_noms && strcmp(noms[i], releves->tab[k].structure) != 0; ++i) {
    }
    if(i == nb_noms && nb_noms < NB_STRUCTURES_MAX) {
      noms[nb_noms] = releves->tab[k].structure;
      ++nb_noms;
    }
  }
  for(i = 0; i < nb_noms; ++i) {
    dessiner_courbe(ecran, releves, vue, &bornes, noms[i]);
    couleur_structure(noms[i], &r, &g, &b);
    boxRGBA(ecran, LARGEUR - MARGE_DROITE + 20, MARGE_HAUT + i * 24, LARGEUR - MARGE_DROITE + 32,
            MARGE_HAUT + i * 24 + 12, r, g, b, 255);
    stringRGBA(ecran, LARGEUR - MARGE_DROITE + 40, MARGE_HAUT + i * 24 + 2, noms[i], 30, 30, 40, 255);
  }
  SDL_Flip(ecran);
}

/* Ouvre la fenêtre et affiche les courbes jusqu'à Échap ; 1 si réussi, 0 sinon */
/* Complexité : O(n log n) par dessin */
int Affichage_courbes(const Releves * releves) {
  SDL_Surface * ecran = NULL;
  SDL_Event event;
  Vue vue = {0, 0, 0};
  int actif = 1;
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Erreur dans SDL_Init : %s\n", SDL_GetError());
    return 0;
  }
  if((ecran = SDL_SetVideoMode(LARGEUR, HAUTEUR, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)) == NULL) {
    fprintf(stderr, "Erreur dans SDL_SetVideoMode : %s\n", SDL_GetError());
    SDL_Quit();
    return 0;
  }
  SDL_WM_SetCaption("Comparateur de structures - courbes", NULL);
  Affichage_dessiner(ecran, releves, &vue);
  while(actif) {
    SDL_WaitEvent(&event);
    switch(event.type) {
      case SDL_QUIT : actif = 0; break;
      case SDL_KEYDOWN :
        switch(event.key.keysym.sym) {
          case SDLK_ESCAPE : actif = 0; break;
          case SDLK_1 : case SDLK_KP1 : vue.operation = 0; break;
          case SDLK_2 : case SDLK_KP2 : vue.operation = 1; break;
          case SDLK_3 : case SDLK_KP3 : vue.operation = 2; break;
          case SDLK_t : vue.comparaisons = !vue.comparaisons; break;
          case SDLK_l : vue.logarithme = !vue.logarithme; break;
          default : break;
        }
        if(actif) {
          Affichage_dessiner(ecran, releves, &vue);
        }
        break;
      default : break;
    }
  }
  SDL_Quit();
  return 1;
}