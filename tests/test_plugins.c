/* +--------------------------------+ */
/* | FICHIER : tests/test_plugins.c | */
/* +--------------------------------+ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plugins.h"

#define NB_PLUGINS 3

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

/* Utilise une structure chargée : 2 ajouts (dont un doublon), 1 recherche */
static int utiliser(const Structure * st) {
  void * s = NULL;
  int ok = 0;
  if((s = st->creer()) == NULL) {
    return 0;
  }
  ok = st->ajouter(s, "chat") == RES_OK
    && st->ajouter(s, "chat") == RES_ABSENT
    && st->chercher(s, "chat") == RES_OK
    && st->taille(s) == 1;
  st->detruire(s);
  return ok;
}

int main() {
  const char * chemins[NB_PLUGINS] = {"./plugins/liste.so", "./plugins/avl.so", "./plugins/hachage.so"};
  const char * noms[NB_PLUGINS] = {"Liste", "AVL", "Hachage"};
  Plugin plugins[NB_PLUGINS];
  Plugin absent;
  char message[128];
  int i;

  /* Les 3 plugins se chargent et se comportent pareil (même fiche) */
  for(i = 0; i < NB_PLUGINS; ++i) {
    sprintf(message, "chargement de %s", chemins[i]);
    verifier(plugin_charger(&plugins[i], chemins[i]), message);
    if(plugins[i].fiche == NULL) {
      continue;
    }
    sprintf(message, "nom de la fiche : %s", noms[i]);
    verifier(strcmp(plugins[i].fiche->nom, noms[i]) == 0, message);
    sprintf(message, "%s : ajout, doublon et recherche via la fiche", noms[i]);
    verifier(utiliser(plugins[i].fiche), message);
  }

  /* Un fichier qui n'existe pas : erreur propre, pas de crash */
  printf("(le message d'erreur suivant est attendu)\n");
  verifier(plugin_charger(&absent, "./plugins/nexistepas.so") == 0, "plugin absent refuse");
  verifier(absent.poignee == NULL && absent.fiche == NULL, "plugin absent : champs remis a NULL");

  for(i = 0; i < NB_PLUGINS; ++i) {
    plugin_fermer(&plugins[i]);
  }
  plugin_fermer(&absent); /* sans effet, ne doit pas planter */

  printf("Bilan : %d erreur(s)\n", nb_erreurs);
  exit(nb_erreurs == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}