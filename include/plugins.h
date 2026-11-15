/* FICHIER : include/plugins.h */
/**
 * @file plugins.h
 * @brief Chargement des structures de données compilées en plugins .so.
 * @author Lilia BEN SALAH
 *
 * Un plugin est un fichier .so qui fournit la fonction structure_plugin().
 * Le programme l'ouvre pendant qu'il tourne (dlopen), sans le connaître
 * à la compilation : ajouter une structure = ajouter un .so, rien d'autre.
 */
#ifndef PLUGINS_H
#define PLUGINS_H

#include "structure.h"

/** Un plugin chargé en mémoire */
typedef struct {
  void * poignee;            /**< ce que renvoie dlopen, à rendre à dlclose */
  const Structure * fiche;   /**< la fiche de fonctions de la structure */
} Plugin;

/**
 * Ouvre le fichier .so et récupère sa fiche.
 * Renvoie 1 si tout va bien, 0 sinon (le message d'erreur est affiché).
 * En cas d'échec, plugin->poignee et plugin->fiche valent NULL.
 */
int plugin_charger(Plugin * plugin, const char * chemin);

/** Ferme le plugin (dlclose). Sans effet si le plugin n'est pas chargé. */
void plugin_fermer(Plugin * plugin);

#endif /* PLUGINS_H */