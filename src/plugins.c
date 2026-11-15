/* FICHIER : src/plugins.c */
/**
 * @file plugins.c
 * @brief Chargement des plugins .so (dlopen, dlsym, dlclose).
 * @author Lilia BEN SALAH
 */
#include <stdio.h>
#include <dlfcn.h>
#include "plugins.h"

/* Vérifie qu'aucune fonction de la fiche n'est absente */
/* Complexité : O(1) */
static int fiche_complete(const Structure * fiche) {
  return fiche->nom != NULL
      && fiche->creer != NULL
      && fiche->ajouter != NULL
      && fiche->chercher != NULL
      && fiche->supprimer != NULL
      && fiche->taille != NULL
      && fiche->operations != NULL
      && fiche->detruire != NULL
      && fiche->parcourir != NULL
      && fiche->observer != NULL;
}

/* Ouvre le .so, récupère structure_plugin() et vérifie la fiche */
/* Complexité : O(1) (hors temps de chargement du fichier) */
int plugin_charger(Plugin * plugin, const char * chemin) {
  const Structure * (*fonction)(void) = NULL;
  plugin->poignee = NULL;
  plugin->fiche = NULL;

  if((plugin->poignee = dlopen(chemin, RTLD_NOW)) == NULL) {
    fprintf(stderr, "Erreur d'ouverture du plugin \"%s\"\n%s\n", chemin, dlerror());
    return 0;
  }
  dlerror(); /* efface une éventuelle ancienne erreur */
  if((fonction = (const Structure * (*)(void))dlsym(plugin->poignee, "structure_plugin")) == NULL) {
    fprintf(stderr, "Erreur : \"%s\" ne fournit pas structure_plugin\n%s\n", chemin, dlerror());
    plugin_fermer(plugin);
    return 0;
  }
  if((plugin->fiche = fonction()) == NULL || !fiche_complete(plugin->fiche)) {
    fprintf(stderr, "Erreur : la fiche du plugin \"%s\" est incomplete\n", chemin);
    plugin_fermer(plugin);
    return 0;
  }
  return 1;
}

/* Ferme le plugin et remet ses champs à NULL */
/* Complexité : O(1) */
void plugin_fermer(Plugin * plugin) {
  if(plugin->poignee != NULL) {
    dlclose(plugin->poignee);
  }
  plugin->poignee = NULL;
  plugin->fiche = NULL;
}