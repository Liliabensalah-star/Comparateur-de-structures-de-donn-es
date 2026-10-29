# Spécifications communes — Comparateur de structures de données

**Groupe 8 · ESGI 2A3 · Javerichalle et Lilia**
Tâche C du PERT · Validé le 29/10/2026 · Version 3

Ce document fixe ce qui est commun aux deux membres du groupe. Une fois envoyé sur `main`, il n'est plus modifié sans l'accord des deux.

---

## 1. Arborescence et responsables

```
Comparateur-de-structures-de-donn-es/
├── include/          structure.h, dictionnaire.h, plugins.h, benchmark.h, csv.h, affichage.h
├── src/              main.c, dictionnaire.c, plugins.c, benchmark.c, csv.c,
│                     affichage.c, animation.c, dessin_avl.c
├── plugins/          liste.c, hachage.c, avl.c   (un fichier .c = un fichier .so)
├── data/             petits fichiers de mots pour les tests
├── resultats/        CSV produits par le programme (dans .gitignore)
├── docs/             PERT, spécifications, guide d'installation, rapport
├── Makefile
└── README.md
```

| Fichier | Rôle | Responsable |
|---|---|---|
| `include/structure.h` | Interface commune (le contrat) | Les deux |
| `src/main.c` | Lancer le programme, lire les options | Les deux |
| `src/dictionnaire.c` | Lire les mots d'un fichier | Les deux |
| `plugins/liste.c` | Liste chaînée | Javerichalle |
| `plugins/hachage.c` | Table de hachage | Javerichalle |
| `plugins/avl.c` | Arbre AVL | Lilia |
| `src/plugins.c` | Charger les `.so` (`dlopen`, `dlsym`) | Lilia |
| `src/benchmark.c` | Chronométrer et compter les opérations | Lilia |
| `src/csv.c` | Écrire les résultats en CSV | Lilia |
| `src/affichage.c` | Fenêtre SDL, courbes, dessin liste et table | Javerichalle |
| `src/animation.c` | Animations et mode interactif | Javerichalle |
| `src/dessin_avl.c` | Dessin de l'AVL, compteur d'opérations affiché | Lilia |
| `Makefile` | Compiler le programme et les plugins | Lilia le crée, les deux le complètent |

Dictionnaire complet : `/usr/share/dict/french` (paquet `wfrench`), non copié dans le dépôt.

---

## 2. Interface commune : `include/structure.h`

Voir le fichier `include/structure.h`. Résumé :

| Fonction | Rôle | Renvoie |
|---|---|---|
| `creer` | Crée une structure vide | pointeur, ou `NULL` si mémoire insuffisante |
| `ajouter` | Range une copie du mot (`strdup`) | `RES_OK`, `RES_ABSENT` si déjà présent, `RES_ERREUR` |
| `chercher` | Cherche le mot | `RES_OK` si trouvé, sinon `RES_ABSENT` |
| `supprimer` | Retire le mot et libère sa copie | `RES_OK`, ou `RES_ABSENT` s'il n'était pas là |
| `taille` | Nombre de mots rangés | `size_t` |
| `operations` | Comparaisons de mots (`strcmp`) depuis `creer` | `long` |
| `detruire` | Libère tous les mots et la structure | — |
| `parcourir` | Appelle `f(mot, ligne, colonne, contexte)` pour chaque mot | — |
| `observer` | Enregistre `f`, appelée à chaque étape ; `NULL` = rien | — |

Règles pour l'auteure d'une structure :

- Toutes les fonctions du plugin sont `static`, sauf `structure_plugin()` qui renvoie la fiche.
- Le compteur `operations` augmente de 1 à **chaque `strcmp`**, et uniquement là (le calcul du hachage et les rotations ne comptent pas).
- À côté de chaque `strcmp` : `if (s->observateur) s->observateur(ETAPE_VISITE, mot, s->contexte);`
- `ETAPE_ROTATION` est signalée par l'AVL à chaque rotation, avec le mot du nœud pivot.

Placement pour `parcourir` :

| Structure | `ligne` | `colonne` |
|---|---|---|
| Liste | 0 | position dans la liste |
| Hachage | numéro de case | position dans la case |
| AVL | profondeur (racine = 0) | rang dans l'ordre alphabétique (parcours infixe) |

**Vérifié le 29/10** : `structure.h` compile sans warning (`-Wall -Wextra -std=gnu11`) avec un plugin de liste de test chargé par `dlopen` ; valgrind ne signale aucune fuite.

---

## 3. Compilation

- Options communes : `gcc -Wall -Wextra -std=gnu11 -Iinclude`
  (`gnu11` plutôt que `c11` : `strdup` et `clock_gettime` ne déclenchent pas de warning).
- Plugin : `gcc ... -fPIC -shared plugins/avl.c -o plugins/avl.so`
- Programme principal : lié avec `-ldl` et la SDL.
- Objectif : **zéro warning**.

---

## 4. Règles sur les mots

| Règle | Décision |
|---|---|
| Doublons | Refusés : `ajouter` renvoie `RES_ABSENT`, les comparaisons faites sont comptées |
| Copie | Chaque structure garde sa copie (`strdup`) et la libère dans `supprimer` et `detruire` |
| Comparaison | `strcmp`, sensible aux majuscules (« Paris » ≠ « paris ») |
| Accents | Mots gardés tels quels (UTF-8) |
| Lecture | Un mot par ligne, retour à la ligne retiré, lignes vides ignorées |
| Longueur | 100 caractères maximum ; les mots plus longs sont ignorés |

---

## 5. Protocole de mesure (banc de test)

**Mélange des mots.** Le dictionnaire est trié, donc on mélange les mots avant les mesures (mélange de Fisher-Yates) avec une **graine fixe : `srand(42)`**. Les trois structures reçoivent **exactement le même ordre**. Les mesures sont ainsi comparables entre structures et identiques d'un lancement à l'autre.

**Tailles mesurées** : 1 000, 5 000, 10 000, 50 000, 100 000 mots, puis le dictionnaire entier. La liste chaînée peut s'arrêter à 50 000 si elle devient trop lente.

**Une série de taille N, pour une structure :**

1. `creer` une structure vide.
2. **ajout** : ajouter les N premiers mots mélangés.
3. **recherche** : chercher ces mêmes N mots (tous présents).
4. **suppression** : supprimer ces mêmes N mots (la structure finit vide).
5. `detruire`.

Chaque opération est chronométrée séparément avec `clock_gettime(CLOCK_MONOTONIC)`. Le nombre de comparaisons d'une opération = `operations()` après − `operations()` avant.

**Répétitions.** La série complète est faite **5 fois**, en recréant la structure à chaque fois.
- `temps_ms` = moyenne des 5 temps.
- `nb_comparaisons` = valeur d'une seule répétition (identique les 5 fois, puisque l'ordre des mots est le même).

Le banc de test n'appelle jamais `observer` : aucune étape n'est signalée pendant les mesures.

---

## 6. Format du CSV

Fichier `resultats/mesures.csv`, réécrit à chaque lancement. Écrit par le banc de test (Lilia), lu par les courbes (Javerichalle).

```
structure,operation,nb_mots,temps_ms,nb_comparaisons
Liste,ajout,1000,1.842,499500
AVL,ajout,1000,0.312,9976
Hachage,ajout,1000,0.105,1043
Liste,recherche,1000,1.790,500500
```

| Colonne | Contenu | Format |
|---|---|---|
| `structure` | Nom de la fiche | `Liste`, `AVL`, `Hachage` |
| `operation` | Opération mesurée | `ajout`, `recherche`, `suppression` |
| `nb_mots` | Taille de la série | entier |
| `temps_ms` | Temps moyen sur 5 répétitions | point décimal, 3 décimales |
| `nb_comparaisons` | Différence du compteur `operations` | entier |

Séparateur : virgule. En attendant le vrai banc de test, Javerichalle utilise un faux CSV au même format pour les courbes (tâche K).

---

## 7. Points laissés à chaque auteure

- **Table de hachage** (Javerichalle) : fonction de hachage et taille de la table (fixe ou redimensionnée). Ce choix change beaucoup les résultats et devra être expliqué dans le rapport.
- **Animations** : le mécanisme (`observer`) est dans le contrat dès maintenant ; son utilisation dans l'affichage vient après le prototype, comme prévu dans le PERT.

---

## 8. Méthode de travail

- `structure.h` et ce document sont envoyés sur `main` le 29/10.
- Une branche et une Pull Request par tâche, relue par l'autre.
- Point de 10 minutes par semaine : où j'en suis, ce qui me bloque, ce que j'attends de toi.
