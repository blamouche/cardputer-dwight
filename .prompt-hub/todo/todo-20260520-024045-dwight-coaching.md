# Tâche : app "Dwight's coaching"

## Objectif
Ajouter une nouvelle app `dwight_coaching` qui affiche, en mode aléatoire, une
phrase de motivation/coaching de Dwight (style The Office).

## Contraintes / conventions
- Aucune dépendance BLE/SD requise (app locale, comme `dwight_focus`).
- Rendu : visage ASCII de Dwight à gauche + bulle de citation à droite
  (réutiliser `DwightFace` + `SpeechBubble`, comme FocusApp).
- Citations par défaut en dur + override possible via
  `apps.dwight_coaching.quotes` (array), pattern de `dwight_answers`.
- Une touche (Enter/espace/flèche) = nouvelle citation aléatoire (jamais 2x la
  même de suite). Touche retour = quitte (comportement par défaut).
- Enregistrer l'app dans `main.cpp` avant `config`.
- Mettre à jour `enabled_apps` + bloc `apps.dwight_coaching` dans le JSON SD.
- Mettre à jour README, releases.md, version.md (bump 0.1.10 -> 0.1.11).

## Étapes
- [x] Créer `src/apps/dwight_coaching/DwightCoachingApp.{h,cpp}`
- [x] Enregistrer dans `src/main.cpp`
- [x] Mettre à jour `sd/dwight-assistant-config.json`
- [x] Mettre à jour README.md
- [x] Bump version + changelog
- [x] `pio run` (validation par compilation)

## Review
- App créée selon le pattern FocusApp (local, sans BLE) : visage ASCII à gauche
  + bulle de citation à droite, hint de bas d'écran.
- Citations par défaut (12) en dur, surchargeables via
  `apps.dwight_coaching.quotes`. `setup()` pose les défauts si la liste est vide
  (appelé avant `loadConfig`), `loadConfig` remplace si l'array est présent.
- Aléa : `randomSeed(esp_random())` une fois, `random(n)` avec évitement de la
  répétition immédiate. Une touche = citation suivante, retour = quitte.
- Concurrence : la version 0.1.11 a été prise par une feature « Power save »
  publiée par une session parallèle pendant le build → app livrée en **0.1.12**.
- Validation : `pio run` SUCCESS (Flash 24.3% / 811621 o, RAM 10.7%), Version.h
  régénéré à 0.1.12 (splash synchronisé). Pas de tests unitaires dans le projet.
- Statut : **completed**. Projet non-git → pas de commit/push.
- Limites/suivi : rendu device non vérifié par moi (le firmware 0.1.12 a été
  flashé par la session éco) — à valider visuellement : lisibilité des citations
  longues dans la bulle, tirage aléatoire.
