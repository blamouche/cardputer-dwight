# Todo — Synchroniser la version du splash screen

Date : 2026-05-20 02:15:00

## Objectif
À chaque mise à jour du numéro de version, la version affichée sur le splash
screen doit être mise à jour. Aujourd'hui `Splash.cpp` affiche `v1.0` en dur.

## Approche (solution élegante, source unique)
- `.prompt-hub/version.md` reste la source de vérité unique.
- Un pré-script de build génère `src/core/Version.h` (`#define FIRMWARE_VERSION`)
  à partir de `version.md` → le splash reflète automatiquement la version.

## Étapes
- [ ] Créer `scripts/gen_version.py` (pré-script, idempotent).
- [ ] L'ajouter à `extra_scripts` dans `platformio.ini`.
- [ ] Générer/commit `src/core/Version.h` initial.
- [ ] Modifier `Splash.cpp` : `#include "Version.h"` + `print("v" FIRMWARE_VERSION)`.
- [ ] `pio run` pour valider la compilation et la génération.
- [ ] Bump version 0.1.2 → 0.1.3, releases.md, memory.md.

## Review
- Fait. Source unique = `.prompt-hub/version.md` → `scripts/gen_version.py`
  (pré-build) génère `src/core/Version.h`. `Splash.cpp` affiche
  `"v" FIRMWARE_VERSION`.
- Découverte : la version était déjà 0.1.3 (correctif scrollbar existant) →
  ce changement bumpé en 0.1.4, pas piggyback sur 0.1.3.
- Validé : `pio run` OK, log `gen_version: Version.h updated to 0.1.4`.
- Statut : completed.
