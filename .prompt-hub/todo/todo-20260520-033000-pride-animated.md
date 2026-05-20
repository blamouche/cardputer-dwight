# Todo — Thème Pride animé (cyclage arc-en-ciel, toute l'UI)

## Objectif
Transformer le thème `pride` en **arc-en-ciel animé** : les couleurs UI
(`foreground`/`accent`/`muted`) cyclent dans la roue des teintes au fil du temps.
Effet sur **toute l'UI**.

## Décision archi (vérifiée)
- Toutes les apps repeignent déjà sur un timer (60–200 ms), pas seulement sur
  input → animer `gTheme` dans la boucle principale suffit, **aucune modif par
  app**. Le visage ASCII de Dwight (rampe background→muted→foreground) cyclera
  aussi → rainbow Dwight, cohérent.
- Fond noir conservé (lisibilité). Texte sur sélection = `background` (noir) →
  accent maintenu lumineux. Status (success/warning/error) gardés fixes pour la
  lisibilité sémantique.

## Étapes
- [ ] `Theme.h` : déclarer `themeTick(uint32_t nowMs)`, MAJ commentaire.
- [ ] `Theme.cpp` : tracker l'index actif ; HSV→RGB565 ; `themeTick` recalcule
      fg/accent/muted depuis une teinte fonction du temps quand pride actif
      (no-op sinon → zéro impact sur les autres thèmes).
- [ ] `main.cpp` : appeler `themeTick(millis())` chaque frame.
- [ ] `README.md` : préciser que `pride` est animé.
- [ ] Bump version (0.1.14 → 0.1.15) + releases.md.
- [ ] `pio run`.

## Review
- Modifs : `Theme.h` (decl themeTick + commentaire), `Theme.cpp` (index actif,
  hsv565, themeTick, includes math/string, commentaire preset), `main.cpp`
  (appel themeTick en tête de loop), `README.md`, `releases.md`, `version.md`
  (0.1.14 → 0.1.15).
- Clé : aucune app modifiée — toutes repeignent déjà sur timer, l'animation
  centrale de gTheme se propage partout.
- Validation : `pio run` SUCCESS (7.45 s). Pas de tests unitaires.
- Outcome : success. Reste : flash + contrôle visuel du cyclage.
