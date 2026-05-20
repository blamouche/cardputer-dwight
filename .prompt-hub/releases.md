# Changelog

## 0.1.18 — 2026-05-20

### Fixes
- **Menu** : au retour au menu depuis une application via la touche ESC, le
  curseur se positionne désormais sur la ligne de l'application qui était active
  (au lieu de repartir systématiquement sur le premier élément). `AppManager`
  mémorise la dernière app non-home (`lastApp()`) et `MenuApp::onEnter()`
  préselectionne sa ligne dans la liste visible (fallback : premier item).

## 0.1.17 — 2026-05-20

### Build / CI
- Publication du projet sur GitHub (`cardputer-dwight`, public) et mise en place
  de l'**intégration continue GitHub Actions** (`.github/workflows/release.yml`) :
  - build PlatformIO de vérification sur chaque push/PR vers `main` ;
  - sur un **tag `v*`**, compilation du firmware puis création automatique d'une
    **release GitHub** avec le `firmware.bin` mono-fichier (image à flasher à
    l'offset `0x0`) attaché, accompagné d'instructions d'installation
    (`esptool.py` / outils web).
- `.gitignore` : le binaire de build n'est plus versionné (il provient désormais
  de la CI, source de vérité reproductible).

## 0.1.16 — 2026-05-20

### Features
- Nouvelle app **Beet Farm WiFi** (`beet_farm_wifi`) : transforme le Cardputer en
  **point d'accès WiFi ouvert** (`Schrute_Farms_Free_WiFi`) avec **portail captif**.
  Dès qu'un appareil rejoint le réseau, son navigateur ouvre automatiquement une
  **page HTML custom dans l'esprit Dwight Schrute** (« FALSE. You did not find free
  WiFi. I found you. »), qui propose la vidéo *The Office US*
  (https://www.youtube.com/watch?v=AJmaVPfyudQ).
- Le portail n'ayant **pas d'accès Internet**, l'iframe YouTube est impossible : la
  page sert donc un **QR code généré localement** (hors-ligne, en SVG inline) + un
  **bouton PLAY**, pour ouvrir la vidéo sur la **connexion data** du visiteur.
- Mécanique : `SoftAP` (réseau ouvert) + `DNSServer` (toutes les requêtes DNS
  redirigées vers l'IP de l'AP, déclenche la détection de portail captif) + `WebServer`
  synchrone servi dans `loop()` (réponse HTML 200 sur les URLs de sonde des OS
  `/generate_204`, `/hotspot-detect.html`, `/ncsi.txt`, … et le reste). Uniquement des
  libs intégrées au framework Arduino-ESP32, pas de dépendance async.
- Écran Cardputer : portrait ASCII de Dwight + bulle affichant l'état de l'AP, le SSID,
  l'IP (192.168.4.1) et le **nombre de clients connectés** en temps réel. La touche
  retour menu coupe proprement le WiFi (`onExit`).
- QR code généré via la lib **`ricmoo/QRCode`** (nouvelle dépendance, version 4,
  ECC MEDIUM). Réglages surchargeables via la config SD `apps.beet_farm_wifi`
  (`ssid`, `youtube_url`, `channel`).

### Docs
- `README.md` et `sd/dwight-assistant-config.json` mis à jour (table des apps + entrée
  de config `beet_farm_wifi`).

## 0.1.15 — 2026-05-20

### Features
- Le thème **Pride** devient un **arc-en-ciel animé** : ses couleurs « chrome »
  (texte, sélection, éléments atténués) **cyclent en continu** dans la roue des
  teintes (un tour complet toutes les ~6 s). L'effet s'applique à **toute l'UI**
  (menu, splash, toutes les apps, y compris le portrait de Dwight), sans modifier
  chaque app : l'animation est pilotée centralement par `themeTick(millis())`
  dans la boucle principale, et toutes les vues repeignent déjà sur un timer.
  Le fond reste noir et les couleurs de statut (success/warning/error) restent
  fixes pour la lisibilité. `themeTick` est un **no-op** pour tous les autres
  thèmes (aucun impact sur green/blue/purple/yellow/light).
- Conversion **HSV→RGB565** ajoutée dans `Theme.cpp` pour le cyclage de teinte.

## 0.1.14 — 2026-05-20

### Features
- Nouveau thème couleur **Pride** dans l'app **Config** (section Appearance) et
  via `system.color_theme` (id `pride`). La palette étant à couleurs uniques,
  les teintes du drapeau arc-en-ciel sont **réparties sur les rôles de l'UI** :
  texte en **magenta**, sélection en **orange**, éléments atténués en **violet**,
  et statuts en **vert / jaune / rouge**, sur fond noir. Sélectionnable en direct
  et persisté sur la carte SD comme les autres thèmes.

## 0.1.13 — 2026-05-20

### Changes
- **Renommage des apps** avec des noms anglais courts et fun, dans l'esprit
  catchphrase de Dwight Schrute (libellé affiché au menu et dans la barre de
  titre) :
  - `presentation_remote` : *Dwight presentation control* → **Slide Sensei**
  - `amphetamine` : *Dwight under amphetamine* → **Bears Don't Sleep**
  - `keyboard` : *Dwight keyboard* → **Ghost Writer**
  - `dwight_unlock` : *Dwight unlock* → **Open Sesame**
  - `dwight_answers` : *Dwight answers* → **FALSE.**
  - `dwight_focus` : *Dwight is focus* → **Back To Work**
  - `dwight_coaching` : *Dwight's coaching* → **Schrute Wisdom**
- Les **identifiants d'app** (`id()`) sont inchangés : les clés de config SD
  `apps.<id>.*` restent valides, aucune configuration utilisateur n'est cassée.
- Le **nom Bluetooth** de `presentation_remote` (`device_name`, défaut
  *Dwight presentation control*) est **conservé** pour ne pas casser l'appairage
  des hôtes déjà jumelés.

### Docs
- `README.md` mis à jour : table des apps, lien d'ancrage et en-têtes de section.

## 0.1.12 — 2026-05-20

### Features
- Nouvelle app **Dwight's coaching** (`dwight_coaching`) : affiche **au hasard**
  une phrase de motivation/coaching de Dwight (style The Office), avec son
  portrait ASCII à gauche et la citation dans une bulle à droite. **Une touche
  quelconque** (Enter, espace, flèche…) tire une **nouvelle citation aléatoire**
  (jamais deux fois la même de suite) ; la touche retour quitte l'app. Liste de
  citations par défaut intégrée, **surchargée** si la config SD fournit
  `apps.dwight_coaching.quotes` (tableau de chaînes). Aucune dépendance BLE/SD.

## 0.1.11 — 2026-05-20

### Features
- **Mode économie d'énergie** dans l'app **Config** (nouvelle section **Power**).
  Quand il est activé, il : **éteint la LED RGB** embarquée, **coupe tous les
  sons** (beeps), **plafonne la luminosité** de l'écran (cap à 40/255) et
  **éteint l'écran après 15 s d'inactivité** (n'importe quelle touche le
  réveille). Réglage persisté sur la carte SD via `system.power_save_enabled`.
  Activation/désactivation avec `,` / `/` ou `Enter` ; effet appliqué en direct.
- Constantes `POWER_SAVE_BRIGHTNESS` (40) et `POWER_SAVE_SCREEN_OFF_MS` (15000)
  dans `HardwareConfig.h`. Accès dérivés `ConfigManager::soundsEnabled()`,
  `ledEnabled()` et `effectiveBrightness()` pour que l'éco mode soit respecté de
  façon cohérente (son, LED, luminosité). L'app **Focus** passe désormais par
  `soundsEnabled()` pour ses bips.

## 0.1.10 — 2026-05-20

### Fixes
- **Caractères spéciaux correctement saisis** sur hôte **macOS French AZERTY**
  (apps **Dwight answers**, **Dwight unlock**, **Dwight keyboard**). L'ancienne
  table ne traduisait qu'une poignée de positions ; toute la rangée de chiffres
  non-shiftée (`& é " ' ( § è ! ç à`) et la ponctuation (`, ; . : / =`) étaient
  donc fausses. Nouvelle table complète, cohérente avec les correspondances déjà
  validées, plus le **décodage UTF-8** des accents directs (`é è à ç ù § °`).
- Traduction de disposition centralisée dans un module partagé
  `core/AzertyText` (`mapAscii` + `typeText` avec décodage UTF-8, espacement BLE
  sûr et `releaseAll`), supprimant trois copies divergentes de `toAzerty`.

### Limites connues
- Caractères nécessitant **Option/AltGr** (`{ } [ ] | \ € ~ ^`) et **majuscules
  accentuées** (É È À…) non gérés (laissés tels quels / ignorés). `@` et `#`,
  eux, sont gérés (touche directe sur mac French).

## 0.1.9 — 2026-05-20

### Fixes
- **Envoi BLE de texte fiabilisé** (apps **Dwight answers** et **Dwight unlock**).
  Les caractères étaient envoyés trop vite (`delay(8)`/`delay(12)`) : les
  rapports HID press/release se télescopaient dans un même intervalle de
  connexion BLE (~15 ms sur macOS), un « release » pouvait être perdu et une
  touche/un modificateur restait « collé » — l'hôte déclenchait alors des
  raccourcis (menus contextuels, clics, etc.) au lieu de saisir le texte.
  Correctif : espacement porté à `delay(28)` entre caractères + nouvel appel
  `BleHid::releaseAll()` en fin de saisie pour garantir un rapport propre.

## 0.1.8 — 2026-05-20

### Features
- Nouvelle app **Dwight answers** (`dwight_answers`) : une bibliothèque de
  **phrases pré-enregistrées** que l'on saisit sur l'ordinateur connecté en
  Bluetooth en les **sélectionnant dans une liste déroulante**. On choisit une
  phrase avec `;`/`.` et `Enter` la tape caractère par caractère sur l'hôte
  (sans Entrée finale, pour pouvoir relire avant d'envoyer). Les phrases se
  **gèrent directement sur l'appareil** : `n` = nouvelle, `e` = éditer la
  sélection, `d` = supprimer ; l'éditeur saisit du texte ASCII, `Enter` sauve,
  `` ` `` annule. Liste persistée dans la config SD sous
  `apps.dwight_answers.phrases`, avec mapping clavier AZERTY/US (`layout`) comme
  les autres apps BLE. Quelques phrases par défaut au premier lancement.

## 0.1.7 — 2026-05-20

### Features
- App **Config** : nouvelle section **Sound** dans la configuration générale.
  Elle permet d'**autoriser ou non les bips sonores** (interrupteur On/Off) et
  de **régler le volume** du buzzer via une jauge, avec un aperçu sonore à
  chaque changement. Navigation : `;`/`.` change de ligne, `,`/`/` modifie la
  valeur, `Enter` bascule les bips. Persisté dans la config SD via la nouvelle
  clé `system.buzzer_enabled` (défaut `true`). L'app **Dwight is focus**
  respecte désormais cet interrupteur en plus du volume.

## 0.1.6 — 2026-05-20

### Changes
- Le menu principal épingle désormais l'app **Config** en bas de la liste, quel
  que soit son ordre dans `menu.enabled_apps`. L'ordre des autres apps est
  conservé.

## 0.1.5 — 2026-05-20

### Features
- Nouvelle app **Dwight is focus** (`dwight_focus`) : un minuteur de focus. On
  choisit une durée (réglable par pas de 1 / 5 min, défaut 25), puis un
  **compte à rebours plein écran en gros chiffres** s'égrène pendant que Dwight
  lance des **menaces croissantes** pour qu'on respecte le temps de focus. Une
  tentative d'abandon (touche retour menu) est interceptée : Dwight menace et
  exige une seconde pression pour confirmer la reddition ; toute autre touche
  renvoie au travail. À zéro, Dwight félicite (à contrecœur). Bips de fin et
  grognements via le buzzer, respectant `system.buzzer_volume` (0 = muet).
  Config `apps.dwight_focus.default_minutes`. Aucune dépendance BLE/SD.

## 0.1.4 — 2026-05-20

### Features
- Splash de démarrage : la version affichée (`vX.Y.Z`) est désormais dérivée
  automatiquement de la source unique `.prompt-hub/version.md`, au lieu d'un
  `v1.0` codé en dur. Un pré-script de build (`scripts/gen_version.py`) génère
  `src/core/Version.h` (`FIRMWARE_VERSION`) à chaque build, donc tout bump de
  version se reflète sur le splash sans édition manuelle.

## 0.1.3 — 2026-05-20

### Fixes
- Barre de scroll du menu : en thème **Light**, le pouce (couleur `accent`,
  gris foncé) sur la piste (`muted`, gris moyen) manquait de contraste et
  restait peu visible sur l'écran TFT. Le pouce est désormais dessiné en
  `foreground`, qui reste fortement contrasté avec la piste et le fond dans
  tous les thèmes.

## 0.1.2 — 2026-05-20

### Features
- Menu général : ajout d'une barre de défilement verticale à droite de la
  liste. Elle n'apparaît que lorsque le nombre d'apps dépasse la fenêtre
  visible (4 lignes), avec une piste (couleur `muted`) et un curseur
  (couleur `accent`) dont la taille et la position reflètent la portion de
  liste affichée. Signale ainsi les options accessibles par défilement. La
  largeur de la liste est réduite pour laisser place à la barre sans
  recouvrir le texte.

## 0.1.1 — 2026-05-20

### Fixes
- Visage ASCII de Dwight : la rampe de luminance (5 niveaux) était partiellement
  codée en dur en vert (niveaux faint/mid), donc le portrait gardait des teintes
  vertes sur les thèmes blue/purple/yellow/light. La rampe est désormais
  entièrement dérivée du thème (`background` → `muted` → `foreground`) par
  interpolation RGB565. Affecte le menu et le splash de démarrage.

## 0.1.0 — 2026-05-20

### Features
- Sélecteur de thème couleur dans l'app Config. L'écran s'ouvre désormais sur
  un menu à sections (Computers / Appearance). La section Appearance permet de
  choisir la couleur de l'interface : **Green** (défaut), **Blue**, **Purple**,
  **Yellow**, et **Light** (fond blanc, interface en niveaux de gris).
- Le thème choisi est appliqué en direct et persisté dans la config SD via la
  nouvelle clé `system.color_theme`, donc conservé au redémarrage.
- Presets de thème centralisés dans `src/core/Theme.cpp` avec une petite API
  (`themeApply`, `themeCount`, `themeId`, `themeLabel`).

### Docs
- README et `sd/dwight-assistant-config.json` mis à jour avec `color_theme`.

> Note : ce dépôt n'est pas un dépôt git, donc aucun commit/push n'a été
> effectué (règles de versioning prompt-hub appliquées au fichier de version).
