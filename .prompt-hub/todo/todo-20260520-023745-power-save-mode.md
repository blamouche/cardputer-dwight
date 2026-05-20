# Tâche : Option « Économie d'énergie » dans la config

Timestamp : 20260520-023745

## Objectif
Ajouter dans l'app Config une option « Économie d'énergie » qui, quand elle est
activée :
- désactive la LED RGB embarquée,
- coupe les sons (beeps),
- limite la luminosité de l'écran,
- éteint l'écran après 15 secondes d'inactivité (réveil à la première touche).

## Plan
1. `SystemConfig` : champ `bool powerSaveEnabled` + accès dérivés
   `soundsEnabled()`, `effectiveBrightness()` dans `ConfigManager`.
2. `ConfigManager.cpp` : défaut `false`, (dé)sérialisation `power_save_enabled`,
   définition `effectiveBrightness()` (cap = `POWER_SAVE_BRIGHTNESS`).
3. `HardwareConfig.h` : constantes `POWER_SAVE_BRIGHTNESS`,
   `POWER_SAVE_SCREEN_OFF_MS`.
4. `FocusApp::beep` et `ConfigApp::previewBeep` : passer par `soundsEnabled()`.
5. `ConfigApp` : nouvelle section « Power » (Mode::Power) avec un toggle,
   application live de la luminosité au basculement.
6. `main.cpp` : `updateLed()` éteint la LED si power save ; `updatePowerSave()`
   gère le timer d'extinction d'écran + réveil ; luminosité initiale via
   `effectiveBrightness()`.
7. Build `pio run`, doc (README/CLAUDE memory/lessons), version 0.1.10 → 0.1.11,
   releases.

## Notes
- Pas de dépôt git (`git rev-parse` échoue) → commit/push impossibles ; je le
  signale à l'utilisateur, le reste des règles de versioning/changelog est tenu.

## Review — terminé
- Tous les points du plan implémentés. `pio run` OK (Flash 24.2% / 808209 o,
  RAM 10.7%), aucune erreur ni warning nouveau.
- Comportement : section « Power » dans Config, toggle persisté
  (`system.power_save_enabled`). Éco → LED off, sons coupés (Focus + preview
  config), luminosité plafonnée à 40, écran off après 15 s, réveil à la touche.
- Non flashé sur device (en attente accord). Rebuild requis avant flash pour
  splash 0.1.11 (version bumpée après le build).
- Pas de dépôt git → commit/push non effectués (impossible).
- Statut : completed (hors commit/flash device).
