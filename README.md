# Dwight Assistant

A multi-application desktop assistant firmware for the **M5Stack Cardputer**
and **Cardputer ADV** (ESP32-S3). Boots into a launcher menu, runs one app at
a time, and stores every preference in a single JSON file on the micro-SD card
so reflashing never loses your settings.

## Status

| App                          | ID                    | State    | Description                                          |
|------------------------------|-----------------------|----------|------------------------------------------------------|
| Slide Sensei                 | `presentation_remote` | ✅ Ready | BLE HID keyboard for slide-deck navigation.          |
| Bears Don't Sleep            | `amphetamine`         | ✅ Ready | Keeps the host awake by tapping arrow keys over BLE. |
| Ghost Writer                 | `keyboard`            | ✅ Ready | Use the Cardputer's QWERTY as a live BLE keyboard.   |
| Open Sesame                  | `dwight_unlock`       | ✅ Ready | Types a saved session password over BLE to unlock a host. |
| FALSE.                       | `dwight_answers`      | ✅ Ready | Pick a saved phrase from a list and type it onto the host over BLE. |
| Back To Work                 | `dwight_focus`        | ✅ Ready | Full-screen focus countdown; Dwight throws escalating threats. |
| Schrute Wisdom               | `dwight_coaching`     | ✅ Ready | Shows a random Dwight motivational quote; any key for the next one. |
| Beet Farm WiFi               | `beet_farm_wifi`      | ✅ Ready | Open WiFi access point + captive portal: joiners get a Dwight-themed page with a QR code / PLAY button to an *Office* video. |
| Config                       | `config`              | ✅ Ready | On-device settings: unlock computers, UI color theme, sound, and power-save (eco) mode. |
| _More coming_                | _tbd_                 | ⏳       | Clock, notes, todo, weather, IR, ...                 |

The framework is built so adding a new app is a small, isolated change — see
[Adding a new app](#adding-a-new-app) below.

## Look and feel

- **CLI / terminal theme** — black background, soft phosphor-green text. The
  whole palette lives in [src/core/Theme.h/.cpp](src/core/Theme.cpp); change it
  in one place and every screen follows.
- **Selectable color themes** — pick the interface color from the Config app
  (Appearance section) or via `system.color_theme` in the SD config. Choices:
  `green` (default), `blue`, `purple`, `yellow`, `light` (white background,
  grayscale interface), and `pride` (an **animated** rainbow that cycles the
  whole UI through the hue wheel). The change applies live and is saved to the
  SD config.
- **Boot splash** — an ASCII-art portrait of Dwight rendered as a luminance
  bitmap (denser glyphs = brighter), tinted to the active color theme, with the
  title, version and an
  animated "booting..." line. Shown for ~3 s before the menu.
- **Dwight everywhere** — the same portrait is drawn on the left of the menu
  and of each app, with the menu list / app content on the right. The shared
  renderer lives in [src/core/DwightFace.h/.cpp](src/core/DwightFace.cpp).

## Hardware

- M5Stack **Cardputer** or **Cardputer ADV** (ESP32-S3, 8 MB flash)
- A micro-SD card (FAT32). Optional but strongly recommended — without it the
  firmware boots with built-in defaults and you cannot persist changes.

Pin assignments are centralized in [src/core/HardwareConfig.h](src/core/HardwareConfig.h):
- SD: SCK=40, MISO=39, MOSI=14, CS=12
- On-board WS2812 LED (SK6812 / GRB): GPIO 21
- Display: 240 × 135, rotation 1

## Build and flash

The project uses [PlatformIO](https://platformio.org/).

```bash
pio run                 # build
pio run -t upload       # flash over USB
```

After every successful build, a self-contained merged binary is produced at
the project root as `firmware.bin`. It can be flashed without PlatformIO:

```bash
esptool.py --chip esp32s3 --port /dev/cu.usbmodem* write_flash 0x0 firmware.bin
```

## Global controls

The same keys work everywhere:

| Key            | Action                                  |
|----------------|-----------------------------------------|
| `` ` ``        | Go back to the launcher menu            |
| `;` / `.`      | Move up / down in the menu              |
| `Enter`        | Launch the highlighted app              |

Each app may also use its own keys. For `presentation_remote`, the four keys
`,`  `.`  `;`  `/` send the configured BLE keys (default = arrow keys) — see
[Slide Sensei](#slide-sensei-presentation_remote).

The on-board RGB LED reflects global state:
- **Red** — no SD card or invalid JSON in the config file
- **Blue** — launcher menu
- **Green** — an app is foregrounded

## SD-card configuration

On first boot, if an SD card is inserted, the firmware creates
**`/dwight-assistant-config.json`** at the SD root with default
values. Edit it on a computer (the card is plain FAT32) and reboot the
Cardputer to apply changes.

A reference copy is shipped in [sd/dwight-assistant-config.json](sd/dwight-assistant-config.json).

### Full schema

```jsonc
{
  "system": {
    "user_name": "",              // shown top-right in the menu (empty = hidden)
    "language": "en",             // UI language. Currently only "en" is shipped.
    "brightness": 255,            // 0..255 display backlight
    "buzzer_volume": 128,         // 0..255 (reserved for future audio apps)
    "sleep_timeout_s": 0,         // 0 = never (reserved for future power mgmt)
    "primary_color": "#D8A25C",   // accent color, "#RRGGBB"
    "color_theme": "green",       // UI theme: green|blue|purple|yellow|light|pride
    "power_save_enabled": false   // eco mode: LED off, no sound, dimmed + screen-off
  },
  "wifi": {
    "enabled": false,             // master switch. WiFi is brought up only by
                                  // apps that need it (NTP clock, weather, ...).
    "ssid": "",
    "password": ""
  },
  "time": {
    "timezone": "Europe/Paris",
    "ntp_server": "pool.ntp.org",
    "format_24h": true,
    "date_format": "dd/mm/yyyy"   // dd/mm/yyyy | mm/dd/yyyy | yyyy-mm-dd
  },
  "menu": {
    "enabled_apps": [             // ordered list of app IDs. Controls ORDER.
      "presentation_remote",      // Any registered app missing from this list
      "amphetamine",              // is appended automatically at load (see note).
      "keyboard",
      "dwight_unlock",
      "dwight_answers",
      "dwight_focus",
      "dwight_coaching",
      "beet_farm_wifi",
      "config"
    ]
  },
  "apps": {
    "presentation_remote": {      // per-app config — schema below
      "device_name": "Dwight presentation control",
      "manufacturer": "M5Stack",
      "battery_level": 100,
      "keys": {
        "left":  "LEFT_ARROW",
        "right": "RIGHT_ARROW",
        "up":    "UP_ARROW",
        "down":  "DOWN_ARROW"
      }
    },
    "amphetamine": {
      "device_name": "Dwight Assistant",
      "manufacturer": "M5Stack",
      "battery_level": 100,
      "interval_s": 10            // seconds between keep-awake key taps (1..300)
    },
    "keyboard": {
      "device_name": "Dwight Assistant",
      "manufacturer": "M5Stack",
      "battery_level": 100,
      "layout": "azerty"          // host keyboard layout: "azerty" | "us"
    },
    "dwight_unlock": {
      "device_name": "Dwight Assistant",
      "manufacturer": "M5Stack",
      "battery_level": 100,
      "layout": "azerty",         // host keyboard layout: "azerty" | "us"
      "computers": [              // one entry per machine; pick one in the app
        {
          "name": "Work laptop",  // label shown in the list
          "password": "changeme"  // typed verbatim, then Enter, to unlock
        }
      ]
    },
    "dwight_answers": {
      "device_name": "Dwight Assistant",
      "manufacturer": "M5Stack",
      "battery_level": 100,
      "layout": "azerty",         // host keyboard layout: "azerty" | "us"
      "phrases": [                // ready-made answers; pick one to type on host
        "Sounds good, I'll take care of it.",
        "Per my last email..."
      ]
    },
    "dwight_focus": {
      "default_minutes": 25       // focus duration pre-selected on setup (1..180)
    },
    "dwight_coaching": {
      "quotes": [                 // random motivational lines; omit to use built-ins
        "Discipline equals freedom. Beets equal power.",
        "Identity theft is not a joke. Neither is wasting this day."
      ]
    },
    "beet_farm_wifi": {
      "ssid": "Schrute_Farms_Free_WiFi",                  // open AP name
      "youtube_url": "https://www.youtube.com/watch?v=AJmaVPfyudQ", // PLAY + QR target
      "channel": 1                // WiFi channel for the SoftAP
    },
    "config": {}                  // the Config app stores nothing of its own;
                                  // it edits dwight_unlock.computers in place
  }
}
```

> **`enabled_apps` only controls order, not visibility.** Any registered app
> not listed is appended automatically when the config loads, so newly added
> apps always appear. (Removing an ID no longer hides an app — that's a
> deliberate trade-off so you don't have to edit the SD card after every
> firmware update.)

> **One shared BLE keyboard.** NimBLE supports a single peripheral, so all
> BLE apps drive the *same* keyboard device (managed by
> [src/core/BleHid.h/.cpp](src/core/BleHid.cpp)). The first BLE app you open in
> a session sets the advertised name; pair once and every BLE app reuses it.

### Config load status

The launcher reflects the load result through the LED (see above). The same
info is logged on the serial console at 115200 baud:

- `loaded` — SD config read successfully
- `created` — no config found, default file written
- `no SD` — no SD card detected, defaults used (changes won't persist)
- `create err` — couldn't write the default file
- `open err` — file exists but couldn't be opened
- `json err: …` — file exists but its JSON is invalid

### Restore defaults

Either:
1. Delete `/dwight-assistant-config.json` on the SD card and reboot.
   The firmware will recreate it with built-in defaults.
2. Or edit any field by hand — invalid values fall back to defaults silently
   (the rest of the file is preserved).

### Backing up your config

The whole config lives in one file, so a copy/paste of
`dwight-assistant-config.json` is enough to migrate to a new SD card
or restore after a wipe.

## Apps

### Slide Sensei (`presentation_remote`)

Turns the Cardputer into a BLE HID keyboard for Google Slides, PowerPoint,
Keynote, etc. Dwight's portrait sits on the left; a speech bubble on the right
prompts you to pair when disconnected and shows a big arrow + label each time
you press a navigation key.

**Keys used while the app is active:**

| Physical key | Default action | Configurable as       |
|--------------|----------------|-----------------------|
| `,`          | Left arrow     | `apps.presentation_remote.keys.left`  |
| `.`          | Down arrow     | `apps.presentation_remote.keys.down`  |
| `;`          | Up arrow       | `apps.presentation_remote.keys.up`    |
| `/`          | Right arrow    | `apps.presentation_remote.keys.right` |

**Accepted key names** in the JSON (case-insensitive):
`LEFT_ARROW`, `RIGHT_ARROW`, `UP_ARROW`, `DOWN_ARROW`,
`PAGE_UP`, `PAGE_DOWN`, `HOME`, `END`,
`ESC`, `ENTER`, `TAB`, `SPACE`, `F5`,
or a single character (e.g. `"a"`, `"b"`).

> Tip: for Google Slides in a browser, `PAGE_UP` / `PAGE_DOWN` are often more
> reliable than arrows.

**Usage:**
1. Launch the app from the menu. The bubble shows the `WAITING` state.
2. From the host OS, pair the device named by `device_name` (default
   `Dwight presentation control`) in the Bluetooth settings. It must be
   recognized as a **keyboard**.
3. Once paired, the status switches to `CONNECTED`.
4. Open your presentation, switch it to presentation mode, and click inside
   it to give it OS focus.
5. The four keys above now flip through the slides.
6. Press `` ` `` (backtick) anytime to return to the launcher. The BLE
   pairing is kept alive across menu navigations.

Original standalone project (now superseded by this firmware):
https://github.com/blamouche/cardputer-presentation-remote

### Bears Don't Sleep (`amphetamine`)

Keeps a paired host awake — no more screen lock or sleep during a long read or
download. While active it sends a single arrow-key tap every `interval_s`
seconds, alternating **Up** then **Down** so nothing actually scrolls or
moves. On screen, Dwight stares back with fast-blinking **red eyes**; the
bubble asks you to pair over Bluetooth when disconnected and reads `AWAKE` once
connected.

**Config (`apps.amphetamine`):**

| Field          | Default            | Meaning                                  |
|----------------|--------------------|------------------------------------------|
| `device_name`  | `Dwight Assistant` | BLE keyboard name (shared — see note above) |
| `manufacturer` | `M5Stack`          | BLE manufacturer string                  |
| `battery_level`| `100`              | Reported HID battery level               |
| `interval_s`   | `10`               | Seconds between keep-awake taps (1..300) |

**Usage:**
1. Launch the app and pair the BLE keyboard from the host (same device used by
   the presentation app — pair once).
2. Leave the app foregrounded. As long as it's running and connected, the host
   receives a harmless key tap on the interval and won't sleep.
3. Press `` ` `` to return to the launcher (this stops the taps).

### Ghost Writer (`keyboard`)

Forwards the Cardputer's built-in **QWERTY** keyboard to the paired host over
BLE — type on the Cardputer, the characters land on your laptop. Letters,
digits and symbols are sent as-is (the library applies shift), and **Enter**,
**Backspace** and **Tab** are forwarded too. Dwight watches from the left and
drops an oppressive one-liner on every keystroke ("I saw that key.", "Nothing
you type is private.", …); the bubble asks you to pair when disconnected.

**Host layout (`layout`):** BLE HID transmits physical key positions, not
characters — the host applies its own layout. With a **French AZERTY** host,
pressing the Cardputer's QWERTY `q` would otherwise show `a`. Set
`apps.keyboard.layout` to `"azerty"` (default) so the firmware translates
letters (`a`↔`q`, `z`↔`w`, `m`) and digits to land correctly; use `"us"` if
your host is already QWERTY. (Some non-alphanumeric symbols may still differ on
AZERTY.)

**Config (`apps.keyboard`):** `device_name`, `manufacturer`, `battery_level`,
`layout` (BLE fields share the same meaning as the other apps; the keyboard is
shared — see the note in [Full schema](#full-schema)).

**Usage:**
1. Launch the app and pair the BLE keyboard from the host.
2. Type on the Cardputer; the keys appear on the connected machine.
3. Press **ESC** (the top-left `` ` `` key) to return to the launcher. That key
   is the global "back" shortcut, so it is never sent to the host.

### Open Sesame (`dwight_unlock`)

Types the **session password** of a paired computer over BLE so its lock
screen unlocks itself — handy when the machine is across the desk. You can save
**several computers** (a label + its password); pick one and Dwight types the
password followed by **Enter**.

**Keys used while the app is active:**

| Physical key | Action                                            |
|--------------|---------------------------------------------------|
| `;` / `.`    | Move up / down through the saved computers        |
| `Enter`      | Type the selected computer's password + Return    |
| `` ` ``      | Back to the launcher                              |

**Config (`apps.dwight_unlock`):**

| Field          | Default            | Meaning                                       |
|----------------|--------------------|-----------------------------------------------|
| `device_name`  | `Dwight Assistant` | BLE keyboard name (shared — see note above)   |
| `manufacturer` | `M5Stack`          | BLE manufacturer string                       |
| `battery_level`| `100`              | Reported HID battery level                    |
| `layout`       | `azerty`           | Host layout for typing the password: `azerty` \| `us` |
| `computers`    | `[]`               | Array of `{ "name", "password" }` entries     |

> **Layout matters here.** BLE HID sends key *positions*, not characters, so a
> password typed for a US host will be garbled on an AZERTY host (and vice
> versa). Set `layout` to match the target machine. As with `keyboard`, only
> the AZERTY/US-differing positions are remapped — exotic symbols may still
> need a US layout on the host.

**Usage:**
1. Add your computers in the **Config** app (or edit `computers` on the SD card).
2. Launch the app and pair the BLE keyboard from the host (shared device — pair
   once across all BLE apps).
3. With the host's lock screen focused, select the right computer and press
   **Enter**. The password is typed and submitted.

> ⚠️ Passwords are stored in **clear text** in the JSON file on the SD card.
> Treat the card accordingly; this app trades secrecy for convenience.

### FALSE. (`dwight_answers`)

A library of **ready-made phrases** ("answers") you can type onto the paired
host over BLE. Pick one in the scrolling list and Dwight types it character by
character — **no Return is sent**, so you can review (or keep editing on the
host) before submitting. Phrases can be **added, edited and deleted** directly
on the device; the list is persisted under `apps.dwight_answers.phrases`.

**Keys used while the app is active:**

| Mode | Physical key | Action                                          |
|------|--------------|-------------------------------------------------|
| List | `;` / `.`    | Move up / down through the saved phrases         |
| List | `Enter`      | Type the selected phrase on the host (no Return) |
| List | `n`          | New phrase                                       |
| List | `e`          | Edit the selected phrase                         |
| List | `d`          | Delete the selected phrase                       |
| List | `` ` ``      | Back to the launcher                             |
| Edit | (typing)     | Append characters; `Del` erases the last one     |
| Edit | `Enter`      | Save the phrase                                  |
| Edit | `` ` ``      | Cancel and return to the list                    |

**Config (`apps.dwight_answers`):**

| Field          | Default            | Meaning                                       |
|----------------|--------------------|-----------------------------------------------|
| `device_name`  | `Dwight Assistant` | BLE keyboard name (shared — see note above)   |
| `manufacturer` | `M5Stack`          | BLE manufacturer string                       |
| `battery_level`| `100`              | Reported HID battery level                    |
| `layout`       | `azerty`           | Host layout for typing the phrase: `azerty` \| `us` |
| `phrases`      | a few defaults     | Array of phrase strings shown in the list     |

> **Layout matters here too.** BLE HID sends key *positions*, not characters, so
> set `layout` to match the target machine. The translation targets **macOS
> French AZERTY**: letters, digits, common punctuation and the direct accents
> (`é è à ç ù § °`, edit them in the SD JSON) are handled. Characters that need
> Option/AltGr (`{ } [ ] | \ € ~ ^`) and accented capitals are not — set
> `layout` to `us` for a US host to send everything verbatim instead.

**Usage:**
1. Launch the app and pair the BLE keyboard from the host (shared device — pair
   once across all BLE apps).
2. Add or edit phrases with `n` / `e` (or edit `phrases` on the SD card).
3. With the target text field focused on the host, select a phrase and press
   **Enter** to type it.

### Back To Work (`dwight_focus`)

A focus timer. Pick a duration and a **big full-screen countdown** ticks down
while Dwight throws **escalating threats** to keep you on task. Try to bail out
before the clock runs out (the global back key) and Dwight intercepts: he
threatens you and demands you press back **again** to confirm the surrender —
any other key sends you back to work. Reach zero and he congratulates you,
grudgingly. No BLE or SD writes — it runs entirely on-device.

**Keys used while the app is active:**

| Phase   | Physical key | Action                                          |
|---------|--------------|-------------------------------------------------|
| Setup   | `;` / `.`    | Decrease / increase the duration by 1 minute    |
| Setup   | `,` / `/`    | Decrease / increase the duration by 5 minutes   |
| Setup   | `Enter`      | Start the countdown                             |
| Running | `` ` ``      | Ask to abandon (Dwight threatens); press again to quit |
| Running | any other key (while abandoning) | Resume the countdown            |
| Done    | `Enter`      | Set a new timer                                 |

**Config (`apps.dwight_focus`):**

| Field             | Default | Meaning                                          |
|-------------------|---------|--------------------------------------------------|
| `default_minutes` | `25`    | Duration pre-selected on the setup screen (1–180) |

> The completion chime and threat growls use the on-board buzzer and respect
> `system.buzzer_volume` — set it to `0` to keep Dwight silent.

### Schrute Wisdom (`dwight_coaching`)

A pep-talk machine. Dwight's portrait sits on the left and a **random
motivational line** appears in a speech bubble on the right. Press **any key**
(Enter, space, an arrow…) for the next random quote — it never repeats the same
line twice in a row. The global back key leaves the app. No BLE or SD writes —
it runs entirely on-device.

**Keys used while the app is active:**

| Physical key | Action                          |
|--------------|---------------------------------|
| any key      | Show the next random quote      |
| `` ` ``      | Back to the menu                |

**Config (`apps.dwight_coaching`):**

| Field    | Default        | Meaning                                            |
|----------|----------------|----------------------------------------------------|
| `quotes` | built-in list  | Array of strings drawn at random. Omit to use the built-in quotes. |

### Beet Farm WiFi (`beet_farm_wifi`)

Turns the Cardputer into an **open WiFi access point** named
`Schrute_Farms_Free_WiFi` with a **captive portal**. As soon as a phone or
laptop joins, its browser pops open a self-contained Dwight-themed page
("FALSE. You did not find free WiFi. I found you.") pitching an *Office*
YouTube video.

Because the access point has **no internet uplink**, the video can't stream on
the portal itself. Instead the page shows a **QR code** (generated on-device,
fully offline, as an inline SVG) plus a **PLAY button** so visitors open the
video on their **own data plan**. The Cardputer screen shows the AP state, SSID,
IP (`192.168.4.1`) and the live count of connected clients.

Under the hood: `WiFi.softAP` (open) + a `DNSServer` that resolves every lookup
to the AP (so the OS captive-portal probe is intercepted) + a synchronous
`WebServer` serviced from the app loop. Leaving via the back key tears the WiFi
stack down. Uses the `ricmoo/QRCode` library; no internet, no BLE.

**Keys used while the app is active:**

| Physical key | Action            |
|--------------|-------------------|
| `` ` ``      | Back to the menu (stops the WiFi AP) |

**Config (`apps.beet_farm_wifi`):**

| Field         | Default                                          | Meaning                          |
|---------------|--------------------------------------------------|----------------------------------|
| `ssid`        | `Schrute_Farms_Free_WiFi`                        | Broadcast network name (open).   |
| `youtube_url` | `https://www.youtube.com/watch?v=AJmaVPfyudQ`    | Target of the PLAY button + QR.  |
| `channel`     | `1`                                              | WiFi channel for the SoftAP.     |

### Config (`config`)

An on-device editor for the **Open Sesame** computer list — add, edit or
remove machines straight from the Cardputer, no SD-card removal needed. Changes
are written back to `/dwight-assistant-config.json` immediately.

**List view:**

| Key       | Action                              |
|-----------|-------------------------------------|
| `;` / `.` | Move up / down in the list          |
| `Enter`   | Edit the selected computer          |
| `n`       | Add a new computer                  |
| `d`       | Delete the selected computer        |
| `` ` ``   | Back to the launcher                |

**Edit view:** type to fill the **Name**, press **Enter** to move to the
**Password**, press **Enter** again to save. **Backspace** deletes the last
character; the `` ` `` (back) key cancels the current edit and returns to the
list. (Because `` ` `` is the global back shortcut, it can't be part of a
password — edit the JSON directly for that rare case.)

The app also exposes three more sections from its opening menu (`;` / `.` to
move, `Enter` to open, `` ` `` to leave):

- **Appearance** — pick the UI color theme, applied live.
- **Sound** — toggle the beeps and set the buzzer volume, with an audible
  preview.
- **Power** — toggle **power save** (eco mode). When on, the on-board LED is
  turned off, all sounds are muted, the screen brightness is capped and the
  screen switches off after 15 s of inactivity (any key wakes it). Persisted as
  `system.power_save_enabled`.

## Project layout

```
dwight-assistant/
├── platformio.ini
├── README.md
├── scripts/
│   ├── merge_bin.py                  # post-build step → ./firmware.bin
│   └── patch_blekeyboard.py          # pre-build patcher for the vendored lib
├── sd/
│   └── dwight-assistant-config.json   # reference SD config
└── src/
    ├── main.cpp                      # boot, app registration, main loop
    ├── core/
    │   ├── App.h                     # base class for every app
    │   ├── AppManager.h/.cpp         # lifecycle + global key routing
    │   ├── ConfigManager.h/.cpp      # SD JSON load/save
    │   ├── HardwareConfig.h          # pin / display defines, config path
    │   ├── Theme.h/.cpp              # CLI green palette and brightness
    │   ├── BleHid.h/.cpp             # the single shared BLE HID keyboard
    │   ├── DwightFace.h/.cpp         # shared ASCII-portrait renderer
    │   ├── dwight_ascii.h            # the portrait art (raw string)
    │   ├── SpeechBubble.h/.cpp       # shared bubble + word-wrap helpers
    │   └── Splash.h/.cpp             # boot splash (portrait + title)
    └── apps/
        ├── menu/
        │   └── MenuApp.h/.cpp        # launcher
        ├── presentation_remote/
        │   └── PresentationRemoteApp.h/.cpp
        ├── amphetamine/
        │   └── AmphetamineApp.h/.cpp
        ├── keyboard/
        │   └── KeyboardApp.h/.cpp
        ├── dwight_unlock/
        │   └── DwightUnlockApp.h/.cpp
        ├── dwight_answers/
        │   └── DwightAnswersApp.h/.cpp
        ├── focus/
        │   └── FocusApp.h/.cpp
        ├── dwight_coaching/
        │   └── DwightCoachingApp.h/.cpp
        ├── beet_farm_wifi/
        │   ├── BeetFarmWifiApp.h/.cpp    # SoftAP + captive portal + QR
        │   └── PortalPage.h              # self-contained Dwight HTML template
        └── config/
            └── ConfigApp.h/.cpp
```

## Adding a new app

1. Create `src/apps/<your_app>/<YourApp>.h` and `.cpp`.
2. Inherit from [`App`](src/core/App.h) and override:
   - `id()` — stable identifier used in the JSON config (e.g. `"clock"`)
   - `name()` — label shown in the menu
   - `subtitle()` — optional short hint
   - `onEnter()` / `onExit()` / `loop()`
   - `loadConfig(JsonObjectConst)` and `saveConfig(JsonObject)` if you need
     persistent per-app settings
3. In [src/main.cpp](src/main.cpp):
   - Include the header.
   - Add a static instance next to `gPresentationRemoteApp`.
   - Register it after the menu: `gApps.registerApp(&gYourApp);`
4. Nothing else to do for visibility — every registered app shows up in the
   menu automatically. To control its position, add its ID to
   `menu.enabled_apps` in `dwight-assistant-config.json` (apps not listed are
   appended after the listed ones).
5. If your app needs to act as a Bluetooth keyboard, use
   [`BleHid`](src/core/BleHid.h) instead of creating your own `BleKeyboard`
   (NimBLE only allows one BLE peripheral).

Minimal example:

```cpp
// src/apps/hello/HelloApp.h
#pragma once
#include "core/App.h"

class HelloApp : public App {
public:
    const char* id()   const override { return "hello"; }
    const char* name() const override { return "Hello"; }
    void onEnter() override;
    void loop() override;
};
```

That's it — the menu picks it up automatically and the JSON config gets a
`apps.hello` section the next time it's regenerated.

## Troubleshooting

**The host shows `CONNECTED` but key presses (or keep-awake taps) have no
effect.**

- Pairing completed, but not as an HID keyboard. Forget the device in your OS
  Bluetooth settings and re-pair.
- The target window doesn't have OS focus. Click into it before pressing keys.
- Some web apps intercept arrow keys differently — try `PAGE_UP` / `PAGE_DOWN`
  in the presentation app config.

**The device appears in Bluetooth but there's no "Connect" / pair-as-keyboard
option (macOS).**

This is fixed in the firmware via a build-time patch to the vendored BLE
library (it must start the GATT server explicitly on NimBLE 2.x). If you wiped
`.pio/`, the patch reapplies automatically on the next `pio run` — see
[scripts/patch_blekeyboard.py](scripts/patch_blekeyboard.py).

**An app I added doesn't show in the menu.**

Every registered app is shown automatically. If it's missing, it isn't
registered — check that `gApps.registerApp(&gYourApp);` is present in
[src/main.cpp](src/main.cpp). (Editing `menu.enabled_apps` is only needed to
reorder; it can no longer hide apps.)

**The LED stays red.**

The SD card is missing or the JSON is malformed. Plug the card into a
computer and check `/dwight-assistant-config.json` against the
[Full schema](#full-schema). If unsure, delete the file and reboot — it will
be recreated. Serial logs (USB-CDC, 115200 baud) print the exact load status.

## Credits

- M5Stack Cardputer hardware + libraries
- BLE HID keyboard: [wakwak-koba/ESP32-NimBLE-Keyboard](https://github.com/wakwak-koba/ESP32-NimBLE-Keyboard)
- JSON: [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- LED: [FastLED](https://github.com/FastLED/FastLED)
- The `presentation_remote` app derives from
  [blamouche/cardputer-presentation-remote](https://github.com/blamouche/cardputer-presentation-remote)
