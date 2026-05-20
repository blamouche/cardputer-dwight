# Todo — Menu : sélection sur l'app active au retour ESC

## Objectif
Quand on est dans une application et qu'on revient au menu via la touche ESC
(`KEY_BACK_TO_MENU`), le menu doit avoir le curseur positionné sur l'app qui
était active, plutôt que de repartir sur le premier élément.

## Cause
- `AppManager::backToMenu()` bascule vers `_apps[0]` (le menu).
- `MenuApp::onEnter()` force `_selection = 0` à chaque entrée.

## Plan
1. `AppManager` : mémoriser la dernière app active non-home (`_lastApp`) dans
   `backToMenu()` + exposer `lastApp()`.
2. `MenuApp::onEnter()` : initialiser `_selection` sur l'index de `lastApp()`
   dans la liste visible (fallback 0).

## Review
- [x] AppManager.h : champ `_lastApp` + accesseur `lastApp()`.
- [x] AppManager.cpp : `backToMenu()` enregistre `_lastApp = _current`.
- [x] MenuApp.cpp : `onEnter()` positionne la sélection sur `lastApp()`.
- [x] `pio run` SUCCESS.
- Statut : completed.
