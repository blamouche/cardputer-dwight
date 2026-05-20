# Task — App "Dwight is focus" (timer de focus)

Timestamp : 20260520-021500
Slug : dwight-focus-timer

## Objectif

Ajouter une nouvelle app `dwight_focus` ("Dwight is focus") :
- Définir une durée de focus, affichée en gros en compte à rebours plein écran.
- Dwight lance des menaces (escalade) pour pousser à respecter le temps de focus.
- Si l'utilisateur tente d'abandonner avant la fin (touche retour menu), Dwight
  menace et exige une confirmation.
- À la fin du compte à rebours, Dwight félicite (à contrecœur).

## Contraintes / conventions repérées

- Pattern App : `id/name/subtitle`, `setup/onEnter/loop/draw`, `load/saveConfig`.
- Entrées via `mgr.inputChanged()/inputPressed()` + `M5Cardputer.Keyboard.keysState()`
  (jamais l'API clavier directement — flag one-shot). Flèches = `;` `.` `,` `/`.
- Touche retour menu globale `` ` `` interceptée par AppManager → `onBackRequested()`.
- Dessin sur `mgr.canvas()`, thème via `gTheme`, helpers `dwightDrawAsciiFace`,
  `drawSpeechBubble`, `drawWrappedText`. Écran 240x135.
- Enregistrement dans `main.cpp` + ajout au sample SD + `enabled_apps`.
- Buzzer : `system.buzzer_volume` existe mais inutilisé → bip via `M5.Speaker`,
  gardé par volume>0.

## Plan

1. [ ] Créer `src/apps/focus/FocusApp.h`
2. [ ] Créer `src/apps/focus/FocusApp.cpp` (états : Setup / Running / Confirm-abandon / Done)
3. [ ] Enregistrer dans `src/main.cpp`
4. [ ] Ajouter au sample `sd/dwight-assistant-config.json` (+ enabled_apps)
5. [ ] Documenter dans `README.md`
6. [ ] Versioning : version.md 0.1.2 → 0.1.3, releases.md, memory.md
7. [ ] Valider via `pio run`

## Review

- [x] `src/apps/focus/FocusApp.h` + `.cpp` créés (4 phases, beep buzzer gardé).
- [x] Enregistré dans `src/main.cpp` (include + instance + registerApp).
- [x] Sample SD : `enabled_apps` + bloc `apps.dwight_focus.default_minutes`.
- [x] README : tableau de statut, section app détaillée, schéma SD, layout.
- [x] Versioning : version.md 0.1.4 → 0.1.5 (repo avait avancé), releases.md,
      memory.md.
- [x] `pio run` OK : Flash 23.9% (798857 o), RAM 10.6%. Splash regénéré 0.1.5.

### Décisions / hypothèses
- "Ne pas respecter le temps de focus" interprété comme : tenter de quitter
  avant la fin → interception via `onBackRequested()` + confirmation à 2
  pressions, plus menaces croissantes en continu pendant le décompte.
- Messages en anglais (style Dwight Schrute) pour cohérence avec les autres
  apps dont les bulles sont en anglais.
- Buzzer : `system.buzzer_volume` existait mais inutilisé ; bips ajoutés via
  `M5.Speaker` avec `begin()` paresseux, no-op si volume = 0.

### Limites / suites
- Non testé sur device réel (compilation seule) : lisibilité du gros MM:SS,
  rendu des bulles, son du buzzer.
- Repo non-git → aucun commit/push possible.

### Statut : completed
