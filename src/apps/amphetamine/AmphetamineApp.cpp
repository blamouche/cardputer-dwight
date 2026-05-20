#include "AmphetamineApp.h"

#include <string.h>

#include "core/AppManager.h"
#include "core/BleHid.h"
#include "core/DwightFace.h"
#include "core/HardwareConfig.h"
#include "core/SpeechBubble.h"
#include "core/Theme.h"

namespace {

const uint16_t kRedEye = 0xF800;  // pure red in RGB565

// Red eyes overlaid on the portrait. Offsets are relative to the portrait's
// top-left corner and tuned to the ASCII art; tweak if the art changes.
const int EYE_Y   = 62;
const int EYE_L_X = 38;
const int EYE_R_X = 66;
const int EYE_R   = 3;

}  // namespace

void AmphetamineApp::setup(AppManager& mgr) {
    _mgr = &mgr;
}

void AmphetamineApp::onEnter() {
    BleHid::begin(_deviceName.c_str(), _manufacturer.c_str(), _batteryLevel);
    _lastTapMs = millis();
    _lastDrawMs = 0;
    _tapUp = true;
}

void AmphetamineApp::loop() {
    uint32_t now = millis();
    if (BleHid::isReady() && now - _lastTapMs >= _intervalMs) {
        _lastTapMs = now;
        if (_tapUp) BleHid::tapUp();
        else        BleHid::tapDown();
        _tapUp = !_tapUp;
        _tapCount++;
    }
    if (now - _lastDrawMs >= 100) {  // redraw so the eyes blink
        _lastDrawMs = now;
        draw();
    }
}

void AmphetamineApp::draw() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    bool connected = BleHid::isConnected();
    bool warming   = BleHid::isWarmingUp();

    // Status, top-right (clear of the portrait). SYNC = post-connect warm-up.
    const char* statusStr = !connected ? "WAITING" : (warming ? "SYNC" : "AWAKE");
    canvas->setTextColor((connected && !warming) ? gTheme.success : gTheme.warning);
    int sw = 6 * (int)strlen(statusStr);
    canvas->setCursor(DISPLAY_W - sw - 2, 1);
    canvas->print(statusStr);

    // Portrait + fast-blinking red eyes.
    dwightDrawAsciiFace(canvas, 0);
    int faceTop = (DISPLAY_H - dwightAsciiFaceHeight()) / 2;
    if ((millis() / 250) & 1) {
        canvas->fillCircle(EYE_L_X, faceTop + EYE_Y, EYE_R, kRedEye);
        canvas->fillCircle(EYE_R_X, faceTop + EYE_Y, EYE_R, kRedEye);
    }

    // Speech bubble on the right.
    const int bx = dwightAsciiFaceWidth() + 6, by = 22;
    const int bw = DISPLAY_W - bx - 4, bh = 100;
    drawSpeechBubble(canvas, bx, by, bw, bh, gTheme.accent);
    const int innerX = bx + 8, innerTop = by + 10, innerW = bw - 16;

    if (!connected) {
        drawWrappedText(canvas,
                    "Pair me over Bluetooth and this machine will NOT sleep.",
                    innerX, innerTop, innerW, gTheme.foreground, 11);
    } else if (warming) {
        drawWrappedText(canvas,
                    "Synchronizing. Stand by before I take the watch.",
                    innerX, innerTop, innerW, gTheme.foreground, 11);
    } else {
        drawWrappedText(canvas,
                    "Wired and wide awake. No naps on my watch.",
                    innerX, innerTop, innerW, gTheme.foreground, 11);
    }

    canvas->pushSprite(0, 0);
}

void AmphetamineApp::loadConfig(JsonObjectConst node) {
    if (node.isNull()) return;
    if (node["device_name"].is<const char*>())  _deviceName   = (const char*)node["device_name"];
    if (node["manufacturer"].is<const char*>()) _manufacturer = (const char*)node["manufacturer"];
    if (node["battery_level"].is<int>())         _batteryLevel = (uint8_t)node["battery_level"];
    if (node["interval_s"].is<int>()) {
        int s = node["interval_s"];
        if (s >= 1 && s <= 300) _intervalMs = (uint32_t)s * 1000;
    }
}

void AmphetamineApp::saveConfig(JsonObject node) {
    node["device_name"]   = _deviceName;
    node["manufacturer"]  = _manufacturer;
    node["battery_level"] = _batteryLevel;
    node["interval_s"]    = (int)(_intervalMs / 1000);
}
