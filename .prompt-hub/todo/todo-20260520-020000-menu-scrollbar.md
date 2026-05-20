# Tâche : barre de scroll dans le menu général

## Objectif
Ajouter une barre de défilement verticale dans `MenuApp` pour signaler à
l'utilisateur les options situées hors de la fenêtre visible (max 4 items).

## Plan
- [x] Lire MenuApp.cpp / .h
- [x] Réserver une colonne à droite quand la liste déborde
- [x] Dessiner piste + curseur proportionnels (muted/accent)
- [x] Compiler (`pio run`)
- [x] Versionner + changelog

## Review
- Implémenté dans `MenuApp::draw()` : colonne réservée à droite (sbW+gap)
  uniquement si `items.size() > visible`, piste `muted` + curseur `accent`
  proportionnel. Largeur de liste réduite pour ne pas recouvrir le texte.
- `pio run` OK (Flash 23.1%, RAM 10.6%). Version 0.1.1 → 0.1.2.
- Statut : completed (test device réel restant).
