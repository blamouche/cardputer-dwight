# Lessons

## Saisie BLE en rafale : espacer les caractères + releaseAll()
- Contexte : 2026-05-20, app Dwight answers — l'envoi de phrases déclenchait des
  raccourcis/clics sur l'hôte macOS au lieu de taper le texte.
- Règle : pour taper du texte via `BleHid::type()` en boucle, **espacer chaque
  caractère d'au moins ~25-30 ms** (> 1 intervalle de connexion BLE, ~15 ms sur
  macOS) et appeler **`BleHid::releaseAll()` en fin de saisie**. En dessous, les
  rapports HID press/release se télescopent, un release est perdu, une touche ou
  un modificateur reste « collé » et l'hôte exécute des accords de raccourcis.
- Ne jamais utiliser `delay(<20)` entre deux `type()` consécutifs.
- `BleKeyboard::write()` ne sépare press/release que de `vTaskDelay(3)` : ce
  n'est pas suffisant à lui seul, d'où le delay applicatif.

## Traduction clavier BLE : cible = macOS French AZERTY, table dans AzertyText
- Le HID envoie des positions de touches US ; l'hôte applique sa disposition.
  Pour faire apparaître un caractère sur un Mac French AZERTY, envoyer l'octet
  US-ASCII dont la position+shift US = position+shift du caractère sur mac FR.
- Toute la traduction est centralisée dans `src/core/AzertyText.{h,cpp}`
  (`mapAscii` + `typeText`). Ne PAS recréer de `toAzerty` local dans une app :
  router par ce module. `typeText` gère aussi l'UTF-8 (accents) + le pacing BLE.
- Accents directs gérés via UTF-8 : é è à ç ù § ° (rangée de chiffres). Les
  caractères Option/AltGr ({ } [ ] | \ € ~ ^) et majuscules accentuées NE sont
  PAS gérables avec la lib (write() = position US + un seul Shift gauche).
- La disposition mac exacte reste à valider touche par touche sur device ; en
  cas d'erreur, corriger la table dans AzertyText.cpp, pas dans les apps.

## Confirmer la langue d'un livrable de contenu avant de le produire
- Contexte : 2026-05-20, propositions de noms d'apps. J'ai d'abord livré un mix
  FR/EN ; l'utilisateur a dû corriger en demandant « uniquement en anglais ».
- Règle : quand le livrable EST du texte exposé à l'utilisateur final (noms
  d'apps, libellés UI, citations, copy), la langue de ce texte est un choix à
  part entière, distinct de la langue de conversation (FR par défaut ici).
  En cas de doute, demander la langue cible AVANT de générer, ou s'aligner sur
  la langue déjà employée dans le produit (les libellés du firmware sont en
  anglais). Ne pas supposer que « réponds en français » implique « contenu en
  français ».

## Inclure ConfigManager.h quand on appelle config()->save()
- `AppManager.h` ne forward-déclare que `class ConfigManager`. Toute app qui
  déréférence `config()` (ex. `config()->save()`, `config()->system()`) doit
  inclure `core/ConfigManager.h`, sinon erreur « invalid use of incomplete type ».
