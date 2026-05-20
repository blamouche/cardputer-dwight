#include <M5Cardputer.h>

// FastLED on ESP32-S3 with arduino-esp32 3.x silently no-ops the on-board RMT
// driver unless these defines come before the FastLED include.
#define FASTLED_RMT_BUILTIN_DRIVER 1
#define FASTLED_RMT_MAX_CHANNELS   1
#define FASTLED_ESP32_RMT_CHANNEL_0 0
#include <FastLED.h>

#include "core/AppManager.h"
#include "core/ConfigManager.h"
#include "core/HardwareConfig.h"
#include "core/Splash.h"
#include "core/Theme.h"

#include "apps/menu/MenuApp.h"
#include "apps/presentation_remote/PresentationRemoteApp.h"
#include "apps/amphetamine/AmphetamineApp.h"
#include "apps/keyboard/KeyboardApp.h"
#include "apps/dwight_unlock/DwightUnlockApp.h"
#include "apps/dwight_answers/DwightAnswersApp.h"
#include "apps/focus/FocusApp.h"
#include "apps/dwight_coaching/DwightCoachingApp.h"
#include "apps/beet_farm_wifi/BeetFarmWifiApp.h"
#include "apps/config/ConfigApp.h"

namespace {

CRGB           gLeds[LED_NUM];
M5Canvas       gCanvas(&M5Cardputer.Display);
ConfigManager  gConfig;
AppManager     gApps;

MenuApp                gMenuApp;
PresentationRemoteApp  gPresentationRemoteApp;
AmphetamineApp         gAmphetamineApp;
KeyboardApp            gKeyboardApp;
DwightUnlockApp        gDwightUnlockApp;
DwightAnswersApp       gDwightAnswersApp;
FocusApp               gFocusApp;
DwightCoachingApp      gDwightCoachingApp;
BeetFarmWifiApp        gBeetFarmWifiApp;
ConfigApp              gConfigApp;

uint32_t gLastLedRefreshMs = 0;
uint32_t gLastActivityMs   = 0;
bool     gScreenOff        = false;

// Tints the on-board LED red when no app is active or no SD config loaded,
// green when an app other than the menu is foregrounded, blue otherwise.
// In power-save mode the LED is kept dark.
void updateLed() {
    uint32_t now = millis();
    if (now - gLastLedRefreshMs < 100) return;
    gLastLedRefreshMs = now;

    CRGB target = CRGB::Black;
    if (gConfig.ledEnabled()) {
        target = CRGB::Blue;
        if (gConfig.status() == ConfigStatus::NoSdCard ||
            gConfig.status() == ConfigStatus::JsonError) {
            target = CRGB::Red;
        } else if (gApps.current() && gApps.current() != gApps.home()) {
            target = CRGB::Green;
        }
    }
    gLeds[0] = target;
    FastLED.show();
}

// Power-save screen handling: any key resets the idle timer and wakes the
// screen; once eco mode has been idle long enough the screen turns off. The
// brightness cap itself lives in ConfigManager::effectiveBrightness().
void updatePowerSave() {
    uint32_t now = millis();

    if (gApps.inputPressed()) {
        gLastActivityMs = now;
        if (gScreenOff) {
            gScreenOff = false;
            M5Cardputer.Display.setBrightness(gConfig.effectiveBrightness());
        }
    }

    bool ps = gConfig.system().powerSaveEnabled;
    if (ps && !gScreenOff && now - gLastActivityMs >= POWER_SAVE_SCREEN_OFF_MS) {
        gScreenOff = true;
        M5Cardputer.Display.setBrightness(0);
    } else if (!ps && gScreenOff) {
        gScreenOff = false;
        M5Cardputer.Display.setBrightness(gConfig.effectiveBrightness());
    }
}

}  // namespace

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(255);
    Serial.begin(115200);

    themeApplyDefaults();
    gCanvas.createSprite(DISPLAY_W, DISPLAY_H);

    FastLED.addLeds<SK6812, LED_PIN, GRB>(gLeds, LED_NUM);
    FastLED.setBrightness(LED_BRIGHTNESS);
    gLeds[0] = CRGB::Red;
    FastLED.show();

    gApps.begin(&gCanvas, &gConfig);

    // Order matters: index 0 is the home/launcher app, others are listed
    // below it. The MenuApp itself is hidden from its own list.
    gApps.registerApp(&gMenuApp);
    gApps.registerApp(&gPresentationRemoteApp);
    gApps.registerApp(&gAmphetamineApp);
    gApps.registerApp(&gKeyboardApp);
    gApps.registerApp(&gDwightUnlockApp);
    gApps.registerApp(&gDwightAnswersApp);
    gApps.registerApp(&gFocusApp);
    gApps.registerApp(&gDwightCoachingApp);
    gApps.registerApp(&gBeetFarmWifiApp);
    gApps.registerApp(&gConfigApp);
    // Future apps: gApps.registerApp(&gClockApp); etc.

    // The Config app edits the unlock app's shared computer list.
    gConfigApp.attachUnlock(&gDwightUnlockApp);

    gConfig.begin();
    gConfig.load(gApps.apps());

    // Apply the UI color theme chosen in the config (defaults to green).
    themeApply(gConfig.system().colorTheme);

    M5Cardputer.Display.setBrightness(gConfig.effectiveBrightness());

    splashShow(&gCanvas, 3000);

    gApps.start();
    gLastActivityMs = millis();
}

void loop() {
    themeTick(millis());  // animates the "pride" theme; no-op for static themes
    gApps.loopFrame();
    updatePowerSave();
    updateLed();
    delay(5);
}
