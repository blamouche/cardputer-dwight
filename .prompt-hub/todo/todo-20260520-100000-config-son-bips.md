# Todo — Config son : autoriser les bips et régler le volume

Date : 2026-05-20
Slug : config-son-bips

## Objectif
Ajouter dans l'app **Config** (configuration générale) une section **Sound**
permettant :
- d'autoriser ou non les bips sonores (toggle on/off) ;
- de régler le volume du buzzer.

Le volume existe déjà (`system.buzzer_volume`, 0..255). On ajoute un drapeau
explicite `buzzer_enabled` pour le toggle on/off, indépendant du niveau.

## Étapes
- [x] `SystemConfig` : ajouter `bool buzzerEnabled`.
- [x] `ConfigManager` : défaut `true`, clé JSON `buzzer_enabled`, load + writeDefault.
- [x] `ConfigApp` : section "Sound", `Mode::Sound`, navigation, draw, aperçu sonore.
- [x] `FocusApp::beep()` : respecter `buzzerEnabled` en plus du volume.
- [x] `sd/dwight-assistant-config.json` : ajouter `buzzer_enabled`.
- [x] Build `pio run`.
- [x] Version, releases, memory.

## Review
- Statut : **completed**. Version 0.1.6 → 0.1.7.
- Build `pio run` OK (Flash 24.0% / 801061 o, RAM 10.6%).
- Section **Sound** ajoutée à l'app Config : ligne « Beeps On/Off » + jauge
  de « Volume », aperçu sonore (tone 880 Hz) à chaque changement, persistance SD.
- Le toggle `buzzer_enabled` coupe les bips indépendamment du volume ; respecté
  par `FocusApp::beep()` et `ConfigApp::previewBeep()`.
- Limite : aperçu sonore non testé sur device réel ; dépôt non-git → pas de commit.
