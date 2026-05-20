# Todo — App "Dwight productivity answers"

Date : 2026-05-20
Slug : dwight-answers

## Objectif
Nouvelle app permettant de paramétrer des phrases ("answers") et de les saisir
sur l'ordinateur connecté en Bluetooth en sélectionnant l'élément voulu dans une
liste déroulante. Options pour ajouter / éditer / supprimer des phrases.

## Hypothèses (les plus sûres)
- id app : `dwight_answers`, nom : "Dwight answers", sous-titre : "BLE".
- Stockage : liste de chaînes sous `apps.dwight_answers.phrases` (+ device_name,
  manufacturer, battery_level, layout AZERTY comme les autres apps BLE).
- Liste plein écran (pas de portrait) pour maximiser la place — pattern ConfigApp
  Computers + DwightUnlock pour l'envoi BLE.
- Navigation liste : `;`/`.` déplacer, `Enter` = taper la phrase via BLE,
  `n` = nouvelle, `e` = éditer la sélection, `d` = supprimer la sélection.
- Édition : champ texte (ASCII 32..126), `Enter` = sauver, `` ` `` = annuler.
- Envoi : pas de Return auto (l'utilisateur relit avant d'envoyer). Mapping
  AZERTY réutilisé comme KeyboardApp/DwightUnlock.
- Quelques phrases par défaut en dur (1er run sans SD) + seed dans le JSON SD.

## Étapes
1. [x] Explorer l'archi (App, AppManager, BleHid, ConfigManager, Unlock, Config, main).
2. [x] Créer `src/apps/dwight_answers/DwightAnswersApp.h`.
3. [x] Créer `src/apps/dwight_answers/DwightAnswersApp.cpp`.
4. [x] Enregistrer dans `src/main.cpp` (include + instance + registerApp).
5. [x] Ajouter au seed `sd/dwight-assistant-config.json` (enabled_apps + bloc apps).
6. [x] Mettre à jour `README.md` (tableau de statut, section app, schéma config).
7. [x] Bump version `0.1.7` -> `0.1.8` + `releases.md`.
8. [x] `pio run` pour valider la compilation.
9. [x] Log mémoire + section review.

## Review
- App livrée et compilée (`pio run` SUCCESS, Flash 24.1% / 807113 o, RAM 10.7%).
- Liste déroulante de phrases + envoi BLE sur l'hôte (Enter) + gestion CRUD
  on-device (n/e/d) conformes à la demande.
- Piège évité : include de `core/ConfigManager.h` requis pour `config()->save()`
  (type complet) — corrigé après 1re erreur de compilation.
- Choix : pas de Return auto après la phrase (relecture avant envoi) ; phrases
  ASCII uniquement (le clavier Cardputer n'a pas les accents) ; liste plein
  écran plutôt que portrait Dwight pour la lisibilité des phrases longues.
- Statut : completed. Limite : envoi BLE non testé sur device réel ; pas de
  commit (dépôt non-git).
