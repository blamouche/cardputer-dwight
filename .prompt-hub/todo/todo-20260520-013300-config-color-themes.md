# Task: Sélecteur de thème couleur dans le menu de configuration

Date: 2026-05-20

## Objectif
Ajouter dans le menu de configuration une option pour choisir la couleur de
l'interface : remplacer le vert par bleu, violet ou jaune, plus un thème
"clair" (fond blanc, interface en niveaux de gris).

## Plan
1. `src/core/Theme.h/.cpp` : introduire des presets nommés + API
   (`themeApply(id)`, `themeCount/themeId/themeLabel`). Presets :
   green (défaut), blue, purple, yellow, light (fond blanc, gris).
2. `src/core/ConfigManager.h/.cpp` : ajouter `SystemConfig.colorTheme`
   (clé JSON `color_theme`, défaut "green"), load + save.
3. `src/main.cpp` : appliquer le thème configuré après `gConfig.load()`.
4. `src/apps/config/ConfigApp.*` : restructurer en menu à sections
   (Computers / Appearance) + écran de choix de thème (applique + persiste live).
5. Doc : `README.md` + `sd/dwight-assistant-config.json`.

## Status
- [x] Theme presets + API
- [x] ConfigManager color_theme
- [x] main.cpp apply theme
- [x] ConfigApp sections + appearance picker
- [x] README + sample config
- [x] Build

## Review
- Thèmes ajoutés : green (défaut), blue, purple, yellow, light (fond blanc gris).
- Centralisés dans Theme.cpp via une table de presets RGB → API simple réutilisée
  par le picker et par main.cpp. Aucun magic number dispersé.
- ConfigApp ouvre sur un menu à sections (Computers / Appearance) ; navigation
  back cohérente (edit→Computers, section→Sections, Sections→quitter).
- Application live + persistance SD (`color_theme`). Rétro-compatible : clé
  absente → défaut "green" ; `primary_color` conservé inchangé.
- Build PlatformIO OK (Flash 23.1%). Pas de tests unitaires dans le repo.
- Limite connue : rendu couleurs/contraste à confirmer sur device réel.
- Status : completed.
