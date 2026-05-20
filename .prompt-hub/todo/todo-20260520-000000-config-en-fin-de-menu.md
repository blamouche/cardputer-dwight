# Tâche : mettre le menu Config en fin de liste

## Objectif
Dans le menu (MenuApp), l'app Config doit toujours apparaître en dernier,
indépendamment de l'ordre de `menu.enabled_apps` dans la config SD.

## Plan
1. Modifier `visibleApps()` dans `src/apps/menu/MenuApp.cpp` pour déplacer
   l'app dont `id() == "config"` à la fin de la liste affichée.
2. Compiler (`pio run`).
3. Mettre à jour version/releases/memory.

## Review
- `visibleApps()` (src/apps/menu/MenuApp.cpp) déplace l'app `config` en fin de
  liste via `std::stable_partition` (ordre des autres apps préservé).
- `<algorithm>` et `<string.h>` déjà inclus → pas d'ajout d'include.
- `pio run` OK (Flash 23.9% / 799837 o). Version 0.1.5 → 0.1.6.
- Outcome : success.
