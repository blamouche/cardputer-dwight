# Todo — Nouvelle app « Beet Farm WiFi » (portail captif Dwight)

Créé : 2026-05-20 03:22:08
App id : `beet_farm_wifi`

## Objectif
Ajouter une app qui crée un point d'accès WiFi ouvert (`Schrute_Farms_Free_WiFi`) avec
un portail captif : tout appareil qui se connecte voit s'ouvrir une page HTML custom
dans l'esprit Dwight Schrute, proposant la vidéo The Office US
(https://www.youtube.com/watch?v=AJmaVPfyudQ) via un QR code + un bouton PLAY (le portail
n'ayant pas d'accès Internet, l'iframe YouTube est impossible).

## Étapes
- [ ] Ajouter `ricmoo/QRCode` à `platformio.ini` (lib_deps).
- [ ] Créer `src/apps/beet_farm_wifi/PortalPage.h` (template HTML PROGMEM, marqueurs
      `%QR%` / `%YT_URL%`).
- [ ] Créer `src/apps/beet_farm_wifi/BeetFarmWifiApp.h`.
- [ ] Créer `src/apps/beet_farm_wifi/BeetFarmWifiApp.cpp` (SoftAP + DNSServer + WebServer
      + génération QR SVG inline + rendu écran Cardputer + loadConfig/saveConfig).
- [ ] Enregistrer l'app dans `src/main.cpp` (include + instance + registerApp).
- [ ] Ajouter `beet_farm_wifi` dans `sd/dwight-assistant-config.json`
      (enabled_apps + nœud apps).
- [ ] Compiler (`pio run`).
- [ ] Bookkeeping : version 0.1.15 → 0.1.16, releases.md, memory.md, README.md.

## Décisions
- Réseau ouvert, SSID `Schrute_Farms_Free_WiFi`, IP AP 192.168.4.1.
- Libs réseau intégrées au framework (WiFi.h / WebServer.h / DNSServer.h), pas d'async.
- QR rendu hors-ligne en SVG inline (lib ricmoo/QRCode, version 4, ECC MEDIUM).
- WiFi coupé à `onExit()` (conso + éviter conflit radio avec le BLE des autres apps).
- Pas de dépôt git → pas de commit/push (règles versioning prompt-hub appliquées au
  fichier de version uniquement).

## Review

**Statut : completed.** Toutes les étapes faites.

- `platformio.ini` : ajout de `ricmoo/QRCode` (les libs WiFi/WebServer/DNSServer
  proviennent du framework, rien à ajouter).
- Créés : `src/apps/beet_farm_wifi/{PortalPage.h, BeetFarmWifiApp.h, BeetFarmWifiApp.cpp}`.
- `src/main.cpp` : include + instance `gBeetFarmWifiApp` + `registerApp` (avant Config).
- `sd/dwight-assistant-config.json` : `beet_farm_wifi` ajouté à `enabled_apps` (avant
  `config`) + nœud `apps.beet_farm_wifi` (ssid / youtube_url / channel).
- Docs : `README.md` (table des apps, section dédiée, snippet config, arbo) +
  `releases.md` (0.1.16) + `version.md` (0.1.16) + `memory.md`.

**Validation** : `pio run` SUCCESS (23.9 s). Flash 36.8 % (1.23 MB / 3.34 MB),
RAM 17.4 %. Lib QRCode + WebServer compilées sans erreur. `firmware.bin` régénéré.

**Limites connues / à vérifier au test device** :
- Coexistence BLE+WiFi : si une app BLE a été ouverte avant, le BLE reste actif ;
  démarrer le SoftAP par-dessus est supporté sur l'ESP32-S3 mais peut être instable.
- Détection du portail captif : on renvoie du HTML 200 sur les URLs de sonde ; le
  comportement exact du popup varie selon l'OS/version (iOS/macOS fiables, Android
  affiche en général une notif « Se connecter au réseau »).
- Vidéo non lisible sur le portail (pas d'Internet) — par conception : QR + bouton
  PLAY pour ouvrir sur la data du visiteur.

**Next** : flash + test réseau réel (rejoindre le WiFi, popup portail, QR/PLAY,
compteur de clients).
