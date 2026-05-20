# Memory Log

## 2026-05-20 01:33 — agent (Claude Code)
- Action : ajout d'un sélecteur de thème couleur dans le menu de configuration
  (Green/Blue/Purple/Yellow + Light fond blanc niveaux de gris).
- Fichiers modifiés :
  - `src/core/Theme.h`, `src/core/Theme.cpp` — presets nommés + API
    (`themeApply`, `themeCount`, `themeId`, `themeLabel`, `themeIndexOf`).
  - `src/core/ConfigManager.h`, `src/core/ConfigManager.cpp` — champ
    `SystemConfig.colorTheme` (clé JSON `color_theme`, défaut "green").
  - `src/main.cpp` — `themeApply(config.colorTheme)` après chargement config.
  - `src/apps/config/ConfigApp.h/.cpp` — restructuration en menu à sections
    (Computers / Appearance) + écran de choix de thème (live + persisté).
  - `README.md`, `sd/dwight-assistant-config.json` — doc + exemple.
- Validation : `pio run` OK (Flash 23.1%, RAM 10.6%). Pas de tests unitaires
  dans le projet ; validation par compilation.
- Outcome : success.
- Note : dépôt non-git → pas de commit/push possible.
- Next : test sur device réel (flash de firmware.bin) pour valider le rendu
  des couleurs, notamment le contraste du thème Light.

## 2026-05-20 01:45 — agent (Claude Code)
- Action : correction des niveaux de vert du visage de Dwight dans les thèmes.
- Cause : `DwightFace.cpp` codait en dur le vert pour les niveaux 1 (faint) et
  3 (mid) de la rampe de luminance ; seuls 0/2/4 suivaient le thème.
- Fix : rampe entièrement dérivée du thème via interpolation RGB565
  (`lerp565`) entre background → muted → foreground. Touche menu + splash.
- Fichiers : `src/core/DwightFace.cpp`, `src/core/DwightFace.h`, `README.md`.
- Validation : `pio run` OK. Version 0.1.0 → 0.1.1.
- Outcome : success.

## 2026-05-20 02:00 — agent (Claude Code)
- Action : ajout d'une barre de scroll verticale dans le menu général
  (`MenuApp`) pour signaler les options hors de la fenêtre visible.
- Détails : barre affichée uniquement si nb d'apps > fenêtre (4). Piste
  `muted` + curseur `accent` proportionnel (taille = visible/total,
  position = top/maxTop). La largeur de liste est réduite de sbW+gap pour
  ne pas recouvrir le texte ; pas de scrollbar dans le cas liste vide.
- Fichiers : `src/apps/menu/MenuApp.cpp`, `.prompt-hub/releases.md`,
  `.prompt-hub/version.md` (0.1.1 → 0.1.2).
- Validation : `pio run` OK (Flash 23.1%, RAM 10.6%).
- Outcome : success.
- Note : dépôt non-git → pas de commit/push possible.
- Next : test sur device réel pour valider le rendu de la barre.

## 2026-05-20 02:05 — agent (Claude Code)
- Action : flash du firmware 0.1.2 sur le Cardputer.
- Commande : `pio run --target upload --upload-port /dev/cu.usbmodem1101`.
- Résultat : 773024 octets écrits à 0x10000, hash vérifié, hard reset OK.
- Outcome : success.
- Next : validation visuelle de la barre de scroll sur device.

## 2026-05-20 02:15 — agent (Claude Code)
- Action : correction contraste de la barre de scroll du menu en thème light.
- Cause : pouce en couleur `accent` (0x44) sur piste `muted` (0x88) → écart de
  luminance trop faible, invisible sur l'écran TFT en thème light.
- Fix : pouce dessiné en `foreground` (couleur la plus contrastée vs piste et
  fond dans tous les thèmes). `src/apps/menu/MenuApp.cpp`.
- Validation : `pio run` OK, flash device OK. Version 0.1.2 → 0.1.3.
- Outcome : success.

## 2026-05-20 02:15 — agent (Claude Code)
- Action : synchronisation automatique de la version du splash screen avec la
  source de vérité `.prompt-hub/version.md`.
- Cause : `Splash.cpp` affichait `v1.0` codé en dur, jamais mis à jour.
- Fix : pré-script de build `scripts/gen_version.py` qui génère
  `src/core/Version.h` (`#define FIRMWARE_VERSION`) depuis `version.md`
  (idempotent, n'écrit que si changement). `Splash.cpp` affiche
  `"v" FIRMWARE_VERSION`. Ajout du pré-script à `platformio.ini`.
- Fichiers : `scripts/gen_version.py`, `src/core/Version.h`,
  `src/core/Splash.cpp`, `platformio.ini`, `releases.md`, `version.md`.
- Validation : `pio run` OK (Flash 23.9%), log `gen_version: Version.h
  updated to 0.1.4` confirmé. Version 0.1.3 → 0.1.4.
- Outcome : success.
- Note : dépôt non-git → pas de commit/push possible. Désormais tout bump de
  `version.md` se reflète sur le splash au prochain build, sans édition manuelle.

## 2026-05-20 02:15 — agent (Claude Code)
- Action : ajout d'une nouvelle app **Dwight is focus** (`dwight_focus`), un
  minuteur de focus avec compte à rebours plein écran et menaces de Dwight.
- Comportement : 4 phases (Setup / Running / ConfirmAbandon / Done). Setup :
  durée réglable `;`/`.` ±1 min, `,`/`/` ±5 min (1..180, défaut 25), Enter =
  démarrer. Running : gros MM:SS centré (textSize 5) + barre de progression +
  bulle de menaces qui escaladent (5 paliers, toutes les 9 s). Touche retour
  menu pendant Running → `onBackRequested()` renvoie false et passe en
  ConfirmAbandon (menace) ; 2e pression = quitte, toute autre touche = reprend.
  Done : félicitations + gros "DONE", Enter = nouveau timer. Bips via
  `M5.Speaker` gardés par `system.buzzer_volume` (begin paresseux).
- Fichiers : `src/apps/focus/FocusApp.h`, `src/apps/focus/FocusApp.cpp` (new),
  `src/main.cpp` (include + instance + registerApp), `README.md` (tableau de
  statut, section app, schéma SD, project layout), `sd/dwight-assistant-config.json`
  (enabled_apps + bloc apps.dwight_focus), `releases.md`, `version.md`.
- Détail technique : `FocusApp.cpp` doit inclure `core/ConfigManager.h` (et pas
  seulement `AppManager.h`) car `config()->system()` déréférence le type complet.
- Validation : `pio run` OK (Flash 23.9% / 798857 o, RAM 10.6%). Splash regénéré
  en 0.1.5. Pas de tests unitaires → validation par compilation.
- Outcome : success. Version 0.1.4 → 0.1.5.
- Note : dépôt non-git → pas de commit/push. Bip buzzer non testé sur device
  réel (aucun usage Speaker préexistant) ; begin() paresseux pour éviter tout
  effet de bord si volume = 0.
- Next : test sur device réel (flash de firmware.bin) — lisibilité du gros
  compte à rebours, rendu des bulles de menace, et son du buzzer.

## 2026-05-20 — Config app épinglée en fin de menu
- Actor : agent (Claude Code)
- Action : `visibleApps()` (src/apps/menu/MenuApp.cpp) force désormais l'app
  dont `id()=="config"` en fin de liste via `std::stable_partition`, quel que
  soit son ordre dans `menu.enabled_apps`. L'ordre relatif des autres apps est
  préservé.
- Fichiers : `src/apps/menu/MenuApp.cpp`, `.prompt-hub/version.md`,
  `.prompt-hub/releases.md`, `.prompt-hub/todo/todo-20260520-000000-config-en-fin-de-menu.md`.
- Validation : `pio run` OK (Flash 23.9% / 799837 o, RAM 10.6%).
- Outcome : success. Version 0.1.5 → 0.1.6.
- Note : dépôt non-git → pas de commit/push.

## 2026-05-20 10:00 — agent (Claude Code)
- Action : ajout d'une section **Sound** dans l'app **Config** (configuration
  générale) pour autoriser/refuser les bips et régler le volume du buzzer.
- Détail : nouveau champ `SystemConfig.buzzerEnabled` (clé JSON
  `system.buzzer_enabled`, défaut `true`), chargé/sauvé dans `ConfigManager`.
  `ConfigApp` : nouvelle entrée de section "Sound" + `Mode::Sound`,
  `handleSoundKeys()` (`;`/`.` change de ligne, `,`/`/` modifie, Enter bascule),
  `drawSound()` (ligne Beeps On/Off + jauge de volume), `previewBeep()` (begin
  paresseux du M5.Speaker, tone 880 Hz 90 ms à chaque changement). `FocusApp::beep()`
  respecte aussi `buzzerEnabled`.
- Fichiers : `src/core/ConfigManager.h/.cpp`, `src/apps/config/ConfigApp.h/.cpp`,
  `src/apps/focus/FocusApp.cpp`, `sd/dwight-assistant-config.json`,
  `releases.md`, `version.md`, todo.
- Validation : `pio run` OK (Flash 24.0% / 801061 o, RAM 10.6%). Splash regénéré
  en 0.1.7. Pas de tests unitaires → validation par compilation.
- Outcome : success. Version 0.1.6 → 0.1.7.
- Note : dépôt non-git → pas de commit/push. Aperçu sonore non testé sur device
  réel.

## 2026-05-20 10:10 — agent (Claude Code)
- Action : flash du firmware 0.1.7 sur le device via `pio run --target upload`.
- Port : `/dev/cu.usbmodem1101`. 801424 octets écrits, hash vérifié, hard reset OK.
- Outcome : success. Reste à valider visuellement la section Sound et l'aperçu
  sonore sur l'écran/buzzer réels.

## 2026-05-20 10:30 — agent (Claude Code)
- Action : nouvelle app **Dwight answers** (`dwight_answers`) — bibliothèque de
  phrases pré-enregistrées saisies sur l'hôte BLE via une liste déroulante, avec
  ajout/édition/suppression on-device.
- Comportement : Mode::List (`;`/`.` naviguer, `Enter` = taper la phrase via
  BLE sans Entrée finale, `n` nouvelle, `e` éditer, `d` supprimer) et Mode::Edit
  (saisie ASCII 32..126, `Del` efface, `Enter` sauve, `` ` `` annule via
  onBackRequested qui renvoie false). Liste plein écran avec fenêtre de scroll
  (pattern ConfigApp/DwightUnlock), statut BLE ONLINE/WAITING en haut à droite,
  message "Typed." 1,5 s après envoi. Mapping AZERTY réutilisé (toAzerty),
  delay(8) entre caractères.
- Stockage : `apps.dwight_answers.phrases` (array de strings) + device_name,
  manufacturer, battery_level, layout. loadConfig ne vide la liste que si
  l'array est présent → 5 phrases par défaut en dur conservées au 1er run.
- Fichiers : `src/apps/dwight_answers/DwightAnswersApp.h/.cpp` (new),
  `src/main.cpp` (include + instance + registerApp après dwight_unlock),
  `sd/dwight-assistant-config.json` (enabled_apps + bloc apps.dwight_answers),
  `README.md` (tableau statut, JSON commenté, section app détaillée, project
  layout), `releases.md`, `version.md`, todo.
- Détail technique : `DwightAnswersApp.cpp` doit inclure `core/ConfigManager.h`
  (pas seulement AppManager.h) car `config()->save()` déréférence le type
  complet — même piège que FocusApp (erreur de compilation corrigée).
- Validation : `pio run` OK (Flash 24.1% / 807113 o, RAM 10.7%). Pas de tests
  unitaires → validation par compilation. Splash regénéré en 0.1.8.
- Outcome : success. Version 0.1.7 → 0.1.8.
- Note : dépôt non-git → pas de commit/push. Envoi BLE des phrases non testé sur
  device réel.
- Next : flash 0.1.8 + test sur device (navigation liste, édition, envoi sur
  hôte AZERTY).

## 2026-05-20 10:35 — agent (Claude Code)
- Action : flash du firmware 0.1.8 sur le device via `pio run --target upload`.
- Port : `/dev/cu.usbmodem1101`. 807472 octets écrits, hash vérifié, hard reset OK.
- Outcome : success. Reste à valider sur l'écran réel l'app Dwight answers
  (liste déroulante, CRUD n/e/d, envoi BLE des phrases sur l'hôte AZERTY).

## 2026-05-20 10:50 — agent (Claude Code)
- Action : correctif envoi BLE de texte (bug remonté par l'utilisateur :
  l'app Dwight answers déclenchait des raccourcis/clics sur le laptop au lieu
  de saisir le texte).
- Cause racine : `BleKeyboard::write()` envoie press puis release séparés de
  seulement vTaskDelay(3) ; avec `delay(8)` (answers) / `delay(12)` (unlock)
  entre caractères, les rapports HID se télescopaient dans un même intervalle
  de connexion BLE (~15 ms sur macOS) → release perdu → touche/modificateur
  « collé » → l'hôte exécute des accords de raccourcis au lieu de taper.
- Fix : `delay(28)` entre caractères (answers + unlock) + nouvelle primitive
  `BleHid::releaseAll()` (rapport HID tout-à-zéro) appelée en fin de saisie.
- Fichiers : `src/core/BleHid.h/.cpp` (ajout releaseAll), 
  `src/apps/dwight_answers/DwightAnswersApp.cpp`,
  `src/apps/dwight_unlock/DwightUnlockApp.cpp`, `releases.md`, `version.md`.
- Validation : `pio run` OK (Flash 24.2% / 807177 o, RAM 10.7%) + flash device
  sur `/dev/cu.usbmodem1101` (807536 o, hash vérifié). Version 0.1.8 → 0.1.9.
- Leçon : pour la saisie BLE en rafale, espacer > 1 intervalle de connexion
  (~15 ms macOS) et toujours releaseAll() après. delay < ~20 ms = touches
  collées. À garder en tête pour toute future app qui « tape » du texte.
- Next : valider sur le laptop que le texte se saisit correctement maintenant.

## 2026-05-20 11:20 — agent (Claude Code)
- Action : refonte de la traduction de disposition clavier pour la saisie BLE
  (bug remonté : caractères spéciaux mal gérés). Cible confirmée par
  l'utilisateur : macOS French AZERTY ; portée : accents + symboles courants
  (pas d'AltGr).
- Cause : l'ancien `toAzerty` (dupliqué dans keyboard/unlock/answers) ne
  traduisait que a/q,z/w,m, `!`→8, `?`→M et les chiffres. Toute la rangée de
  chiffres non-shiftée (`& é " ' ( § è ! ç à`) et la ponctuation
  (`, ; . : / =`) tombaient à côté → apostrophe, virgule, point, parenthèses,
  accents faux.
- Fix : nouveau module partagé `src/core/AzertyText.h/.cpp` —
  `azerty::mapAscii(c)` (table macOS French complète, dérivée des
  correspondances déjà validées par l'auteur : `!`→8, chiffres→jumeaux shiftés,
  donc rangée non-shiftée reconstruite de façon cohérente) +
  `azerty::typeText(text, azerty)` qui décode l'UTF-8 (accents directs é è à ç
  ù § ° → touches de la rangée de chiffres), espace les caractères (delay 28) et
  fait releaseAll. Les 3 apps (answers, unlock, keyboard) passent par ce module ;
  3 copies de toAzerty supprimées.
- Layout macOS French retenu (positions exprimées en touches US) :
  rangée chiffres unshift = & é " ' ( § è ! ç à ) ; bas de clavier
  US m/,/./ = , ; : / et US-/ = `=` ; US-' = ù/% ; US-` = @/# ; US-`-` = )/° ;
  US-`=` = -/_.
- Limites : Option/AltGr ({ } [ ] | \ € ~ ^) et majuscules accentuées non gérés.
- Fichiers : `src/core/AzertyText.h/.cpp` (new),
  `src/apps/{dwight_answers,dwight_unlock,keyboard}/*.cpp`, `README.md`,
  `releases.md`, `version.md`, `lessons.md`.
- Validation : `pio run` OK (Flash 24.1% / 807029 o, RAM 10.7%) + flash device
  `/dev/cu.usbmodem1101`. Version 0.1.9 → 0.1.10.
- Next : test device — vérifier sur le Mac une phrase type
  "C'est l'été à 9h, ça va ?" (apostrophes, accents, ponctuation). Si certaines
  touches sont fausses, ajuster la table dans AzertyText.cpp (la disposition mac
  exacte reste à confirmer touche par touche).

## 2026-05-20 11:30 — agent (Claude Code)
- Action : re-flash ("burn") du firmware 0.1.10 sur `/dev/cu.usbmodem1101`.
  Le flash précédent avait été buildé alors que version.md valait encore 0.1.9
  (splash v0.1.9) ; rebuild → gen_version 0.1.10 → upload. Hash vérifié, reset OK.
- Outcome : success. Device aligné sur 0.1.10 (code + splash).

## 2026-05-20 02:40 — agent (Claude Code)
- Action : ajout d'un mode « Économie d'énergie » (éco) demandé par
  l'utilisateur, configurable dans l'app Config. Effets quand activé : LED RGB
  éteinte, sons coupés, luminosité écran plafonnée (40/255), écran éteint après
  15 s d'inactivité (réveil à la première touche).
- Implémentation : champ `SystemConfig.powerSaveEnabled` (clé SD
  `power_save_enabled`, défaut false) + helpers `ConfigManager::soundsEnabled()`,
  `ledEnabled()`, `effectiveBrightness()`. Constantes `POWER_SAVE_BRIGHTNESS`
  (40) / `POWER_SAVE_SCREEN_OFF_MS` (15000) dans HardwareConfig.h. Nouvelle
  section « Power » dans ConfigApp (Mode::Power + handle/draw, toggle `,`//`Enter`,
  luminosité réappliquée en direct). main.cpp : `updateLed()` éteint la LED en éco,
  `updatePowerSave()` (timer d'extinction + réveil sur input), luminosité initiale
  via effectiveBrightness(). FocusApp::beep passe par soundsEnabled().
- Fichiers : src/core/{ConfigManager.h/.cpp,HardwareConfig.h}, src/main.cpp,
  src/apps/config/ConfigApp.{h,cpp}, src/apps/focus/FocusApp.cpp, README.md,
  releases.md, version.md (0.1.10 → 0.1.11).
- Validation : `pio run` OK (Flash 24.2% / 808209 o, RAM 10.7%). Pas flashé sur
  device (en attente accord utilisateur). NB : version.md bumpé après ce build,
  donc rebuild nécessaire avant flash pour que le splash affiche 0.1.11.
- Contrainte : le projet n'est PAS un dépôt git (`git rev-parse` échoue) →
  commit/push impossibles. Signalé à l'utilisateur.
- Next : faire valider le mode éco sur device (flash) ; vérifier le réveil écran
  et que le cap de luminosité convient (ajuster POWER_SAVE_BRIGHTNESS si besoin).

## 2026-05-20 02:50 — agent (Claude Code)
- Action : rebuild + flash sur /dev/cu.usbmodem1101 (accord utilisateur).
- Note concurrence : une autre session a ajouté en parallèle l'app
  `dwight_coaching` (main.cpp + releases 0.1.12) et bumpé version.md à 0.1.12
  pendant ma tâche. Le pré-build gen_version a donc généré Version.h=0.1.12 ;
  le firmware flashé embarque MON mode éco (0.1.11) ET le coaching (0.1.12),
  splash 0.1.12. Wrote 811984 o, hash vérifié, reset OK. Outcome : success.
- Next : tester le mode éco sur device (Config > Power) — LED off, silence,
  écran dimmé puis off à 15 s, réveil touche. Ajuster POWER_SAVE_BRIGHTNESS si
  besoin. Pas de git → toujours pas de commit/push.

## 2026-05-20 02:55 — agent (Claude Code)
- Action : ajout de l'app **Dwight's coaching** (`dwight_coaching`) — affiche au
  hasard une phrase de motivation de Dwight ; une touche quelconque tire une
  nouvelle citation (jamais 2x la même de suite), touche retour quitte.
- Rendu : visage ASCII à gauche + bulle de citation à droite (réutilise
  DwightFace + SpeechBubble, comme FocusApp). Aucune dépendance BLE/SD.
- Citations : liste par défaut en dur (12 lignes), surchargée si la config SD
  fournit `apps.dwight_coaching.quotes` (array). Pattern de loadConfig identique
  à dwight_answers ; setup() pose les défauts (avant load) si la liste est vide.
- Aléa : randomSeed(esp_random()) une fois à l'onEnter, random(n) avec évitement
  de la répétition immédiate.
- Fichiers : `src/apps/dwight_coaching/DwightCoachingApp.{h,cpp}` (new),
  `src/main.cpp` (include + instance + registerApp avant config),
  `sd/dwight-assistant-config.json` (enabled_apps + bloc), `README.md` (status,
  JSON commenté, section app, project layout), `releases.md`, `version.md`, todo.
- Concurrence : une session parallèle a publié « Power save » et bumpé version.md
  à 0.1.11 pendant mon build. J'ai pris le numéro suivant 0.1.12 pour le coaching
  et inséré mon entrée changelog au-dessus de 0.1.11.
- Validation : `pio run` OK (Flash 24.3% / 811621 o, RAM 10.7%). Version.h=0.1.12
  (splash synchronisé). Pas de tests unitaires → validation par compilation.
- Outcome : success. Version → 0.1.12. Projet non-git → pas de commit/push.
- Next : test device (déjà flashé par la session éco en 0.1.12) — navigation
  dans l'app, tirage aléatoire, lisibilité des citations longues dans la bulle.

## 2026-05-20 03:05 — agent (Claude Code)
- Action : renommage des 7 apps thématiques avec des libellés anglais courts et
  fun dans l'esprit Dwight Schrute (demande utilisateur, noms qu'il a validés).
  presentation_remote→"Slide Sensei", amphetamine→"Bears Don't Sleep",
  keyboard→"Ghost Writer", dwight_unlock→"Open Sesame", dwight_answers→"FALSE.",
  dwight_focus→"Back To Work", dwight_coaching→"Schrute Wisdom".
- Portée : seul le `name()` de chaque header change (source unique du libellé
  affiché menu + barre de titre). Les `id()` sont INCHANGÉS → clés SD
  `apps.<id>.*` préservées, aucune config utilisateur cassée. Le `_deviceName`
  BLE de presentation_remote (et l'exemple `device_name` du README/JSON) est
  CONSERVÉ pour ne pas casser l'appairage des hôtes déjà jumelés.
- Fichiers : les 7 headers d'app (`name()` + commentaire de tête), `ConfigApp.h`
  (commentaire), `README.md` (table, lien d'ancrage, en-têtes de section, ref
  Config), `releases.md`, `version.md` (0.1.12 → 0.1.13), todo.
- Précision langue : l'utilisateur a d'abord reçu des propositions FR/EN puis a
  demandé « uniquement en anglais » → leçon ajoutée (confirmer la langue d'un
  livrable de contenu avant de proposer).
- Validation : `pio run` OK (SUCCESS en 6.81 s, image esp32s3 générée). Pas de
  tests unitaires → validation par compilation. grep de contrôle : plus aucun
  ancien libellé hors device_name BLE.
- Outcome : success. Version → 0.1.13. Projet non-git → pas de commit/push.
- Next : flash device pour vérifier visuellement les nouveaux libellés au menu.

## 2026-05-20 03:10 — agent (Claude Code)
- Action : rebuild + flash du firmware 0.1.13 sur `/dev/cu.usbmodem1101`.
- Détail : `pio run --target upload` → gen_version a régénéré Version.h en 0.1.13
  (splash synchronisé). 812000 octets écrits à 0x10000, hash vérifié, hard reset OK.
- Outcome : success. Device aligné sur 0.1.13 (code + splash).
- Next : contrôle visuel sur l'écran — les 7 nouveaux libellés au menu (Slide
  Sensei, Bears Don't Sleep, Ghost Writer, Open Sesame, FALSE., Back To Work,
  Schrute Wisdom).

## 2026-05-20 03:15 — agent (Claude Code)
- Action : ajout d'un thème couleur **Pride** dans le sélecteur de l'app Config
  (demande utilisateur). Le système de thème étant entièrement dynamique (picker
  + config itèrent themeCount()/themeId()/themeLabel()), un seul nouveau preset
  suffit, aucune liste codée en dur à toucher.
- Palette : palette plate (couleurs uniques) → couleurs du drapeau arc-en-ciel
  réparties sur les rôles UI : fg=magenta (0xFF3DA6), accent=orange (0xFF7A18),
  muted=violet (0x5A2A8C), success=vert, warning=jaune, error=rouge, fond noir.
- Fichiers : `src/core/Theme.cpp` (1 preset `pride`), `src/core/Theme.h`
  (commentaire liste d'ids), `README.md` (liste des choix + commentaire JSON),
  `releases.md`, `version.md` (0.1.13 → 0.1.14), todo.
- Validation : `pio run` OK (SUCCESS 7.27 s, Flash 24.3% / 811685 o, RAM 10.7%).
  Pas de tests unitaires → validation par compilation.
- Outcome : success. Version → 0.1.14. Projet non-git → pas de commit/push.
- Next : flash device pour vérifier le rendu du thème Pride au menu/splash.

## 2026-05-20 03:20 — agent (Claude Code)
- Action : rebuild + flash 0.1.14 sur `/dev/cu.usbmodem1101` (thème Pride).
- Détail : gen_version a régénéré Version.h en 0.1.14 (splash sync). 812048 o
  écrits à 0x10000, hash vérifié, hard reset OK.
- Outcome : success. Device aligné sur 0.1.14.
- Next : contrôle visuel — Config → Appearance → Pride.

## 2026-05-20 03:30 — agent (Claude Code)
- Action : thème **Pride** transformé en **arc-en-ciel animé** (demande user :
  "plus arc-en-ciel ? couleurs dynamiques ?" → choix : animé + toute l'UI).
- Approche : animation centrale de gTheme via `themeTick(millis())` appelée en
  tête de `loop()` (main.cpp). Découverte clé : TOUTES les apps repeignent déjà
  sur un timer (menu 100/amphet 100/config 120/answers 80/coaching 200/keyboard
  80/unlock 60/focus 200 ms) → animer gTheme suffit, AUCUNE modif par app.
- themeTick : no-op sauf si le thème actif est `pride` (index suivi via
  gActiveIdx posé dans themeApply/Defaults). Quand pride : fg/accent/muted
  cyclent via hsv565() sur un tour de roue toutes les ~6 s (kPridePeriodMs).
  Fond noir + statuts (success/warning/error) fixes pour la lisibilité. Le
  visage Dwight (rampe bg→muted→fg) cycle donc aussi.
- Fichiers : `Theme.h`, `Theme.cpp` (hsv565 + themeTick + includes math/string),
  `main.cpp`, `README.md`, `releases.md`, `version.md` (0.1.14 → 0.1.15), todo.
- Validation : `pio run` SUCCESS (7.45 s). Pas de tests unitaires.
- Outcome : success. Version → 0.1.15. Projet non-git → pas de commit/push.
- Next : flash + contrôle visuel du cyclage sur device.

## 2026-05-20 03:33 — agent (Claude Code)
- Action : flash 0.1.15 sur `/dev/cu.usbmodem1101` (thème Pride animé).
- Détail : 813072 o écrits à 0x10000, hash vérifié, hard reset OK. Version.h
  régénéré en 0.1.15 par gen_version.
- Outcome : success. Device aligné sur 0.1.15.
- Next : contrôle visuel du cyclage rainbow (Config → Appearance → Pride) ;
  ajuster kPridePeriodMs si le rythme ne convient pas.

## 2026-05-20 03:22 — agent (Claude Code)
- Action : nouvelle app **Beet Farm WiFi** (`beet_farm_wifi`) — point d'accès
  WiFi ouvert (`Schrute_Farms_Free_WiFi`) + portail captif servant une page HTML
  custom esprit Dwight avec QR code + bouton PLAY vers une vidéo The Office
  (https://www.youtube.com/watch?v=AJmaVPfyudQ).
- Décision clé (validée user via questions) : un portail captif n'a PAS d'accès
  Internet → l'iframe YouTube est impossible. Choix : QR code généré localement
  (hors-ligne, SVG inline via lib ricmoo/QRCode v4 ECC_MEDIUM) + bouton PLAY ;
  la vidéo s'ouvre sur la data du visiteur. Réseau ouvert, SSID figé par défaut.
- Archi : libs intégrées au framework Arduino-ESP32 uniquement (WiFi.h /
  WebServer.h / DNSServer.h), pas d'async. DNSServer "*" → IP AP (déclenche le
  portail captif) ; WebServer synchrone servi dans loop() (handleClient +
  processNextRequest) ; réponse HTML 200 sur /generate_204, /hotspot-detect.html,
  /ncsi.txt, /connecttest.txt, / et onNotFound. WiFi coupé proprement à onExit().
- Découverte : ConfigManager.cpp:164-170 auto-append les apps non listées dans
  enabled_apps → l'app apparaît au menu par simple registerApp ; ajout quand
  même propre dans sd/dwight-assistant-config.json (enabled_apps + nœud apps).
- Fichiers : `platformio.ini` (+ricmoo/QRCode), `src/apps/beet_farm_wifi/`
  (BeetFarmWifiApp.h/.cpp + PortalPage.h), `src/main.cpp` (include+instance+
  registerApp), `sd/dwight-assistant-config.json`, `README.md`, `releases.md`,
  `version.md` (0.1.15 → 0.1.16), todo.
- Validation : `pio run` SUCCESS (23.9 s). Flash 36.8% (1.23 MB / 3.34 MB),
  RAM 17.4%. Lib QRCode + WebServer compilées OK. Pas de tests unitaires.
- Outcome : success. Version → 0.1.16. Projet non-git → pas de commit/push.
- Next : flash sur device + test : rejoindre le WiFi avec un téléphone, vérifier
  l'ouverture auto du portail, le QR/bouton PLAY, le compteur de clients. Veiller
  à la coexistence BLE/WiFi si une app BLE a été lancée avant.

## 2026-05-20 03:30 — agent (Claude Code)
- Action : flash 0.1.16 sur `/dev/cu.usbmodem1101` (app Beet Farm WiFi).
- Détail : 1228976 o écrits à 0x10000 (791892 compressés), hash vérifié, hard
  reset OK. `pio run -t upload` SUCCESS (16.9 s).
- Outcome : success. Device aligné sur 0.1.16.
- Next : test réseau réel — rejoindre le WiFi `Schrute_Farms_Free_WiFi` avec un
  téléphone, vérifier l'ouverture auto du portail captif + QR/bouton PLAY +
  compteur de clients.

## 2026-05-20 03:35 — agent (Claude Code)
- Action : publication du projet sur **GitHub** (`blamouche/cardputer-dwight`,
  **public**) + mise en place CI **GitHub Actions** pour tags/releases avec .bin.
- Détail : `git init` (projet jusque-là non-git), commit initial, `gh repo create
  --public --push`. Workflow `.github/workflows/release.yml` : build PlatformIO
  sur push/PR vers main, et sur tag `v*` → compile puis `softprops/action-gh-release`
  publie une release avec `firmware.bin` (image mono-fichier flash @0x0) attaché +
  une copie nommée `cardputer-dwight-vX.Y.Z.bin`, notes d'install incluses.
- `.gitignore` : retrait de `!firmware.bin` (le binaire vient désormais de la CI).
- Version bumpée 0.1.16 → 0.1.17. Tag `v0.1.17` poussé → run CI 26135852019
  SUCCESS (3m34s). Release v0.1.17 OK avec 2 assets (1 294 432 o).
- Outcome : success. Repo + CI + release opérationnels.
- Next (prochaines releases) : bump `.prompt-hub/version.md` → commit → push →
  `git tag -a vX.Y.Z && git push origin vX.Y.Z` ; la CI publie la release+.bin.
