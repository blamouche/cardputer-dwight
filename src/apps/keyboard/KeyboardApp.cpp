#include "KeyboardApp.h"

#include <M5Cardputer.h>
#include <string.h>

#include "core/AppManager.h"
#include "core/AzertyText.h"
#include "core/BleHid.h"
#include "core/DwightFace.h"
#include "core/HardwareConfig.h"
#include "core/SpeechBubble.h"
#include "core/Theme.h"

namespace {

// Dwight's running commentary — one line per keystroke, deliberately oppressive.
const char* LINES[] = {
    "I saw that key.",
    "Every keystroke is mine now.",
    "Logged. All of it.",
    "I never blink. Keep typing.",
    "That one. I felt it.",
    "Nothing you type is private.",
    "Slower. Let me savor each key.",
    "I am always watching.",
    "Wrong key? I noticed.",
    "Your fingers obey me.",
    "Type. Do not stop.",
    "I record everything you press.",
};
const int LINE_COUNT = (int)(sizeof(LINES) / sizeof(LINES[0]));

}  // namespace

void KeyboardApp::setup(AppManager& mgr) {
    _mgr = &mgr;
}

void KeyboardApp::onEnter() {
    BleHid::begin(_deviceName.c_str(), _manufacturer.c_str(), _batteryLevel);
    _lastKeyMs = 0;
    _lastDrawMs = 0;
}

void KeyboardApp::loop() {
    if (BleHid::isConnected() && _mgr->inputChanged() && _mgr->inputPressed()) {
        auto st = M5Cardputer.Keyboard.keysState();
        bool typed = false;

        for (char c : st.word) {
            BleHid::type((uint8_t)(_azerty ? azerty::mapAscii((uint8_t)c) : (uint8_t)c));
            typed = true;
        }
        if (st.enter) { BleHid::tapEnter();     typed = true; }
        if (st.del)   { BleHid::tapBackspace(); typed = true; }
        if (st.tab)   { BleHid::tapTab();       typed = true; }

        if (typed) {
            _lineIdx = (_lineIdx + 1) % LINE_COUNT;
            _lastKeyMs = millis();
        }
    }

    uint32_t now = millis();
    if (now - _lastDrawMs >= 80) {
        _lastDrawMs = now;
        draw();
    }
}

void KeyboardApp::draw() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    bool connected = BleHid::isConnected();

    // Status, top-right (clear of the portrait).
    const char* statusStr = connected ? "ONLINE" : "WAITING";
    canvas->setTextColor(connected ? gTheme.success : gTheme.warning);
    int sw = 6 * (int)strlen(statusStr);
    canvas->setCursor(DISPLAY_W - sw - 2, 1);
    canvas->print(statusStr);

    // Portrait on the left.
    dwightDrawAsciiFace(canvas, 0);

    // Speech bubble on the right.
    const int bx = dwightAsciiFaceWidth() + 6, by = 22;
    const int bw = DISPLAY_W - bx - 4, bh = 100;
    drawSpeechBubble(canvas, bx, by, bw, bh, gTheme.accent);
    const int innerX = bx + 8, innerTop = by + 12, innerW = bw - 16;

    const char* msg;
    if (!connected) {
        msg = "Pair me over Bluetooth. Then every key is mine.";
    } else if (millis() - _lastKeyMs < 2500) {
        msg = LINES[_lineIdx];
    } else {
        msg = "Type. I am watching every key.";
    }
    drawWrappedText(canvas, msg, innerX, innerTop, innerW, gTheme.foreground, 11);

    // Footer hint inside the bubble.
    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(innerX, by + bh - 14);
    canvas->print("ESC = exit");

    canvas->pushSprite(0, 0);
}

void KeyboardApp::loadConfig(JsonObjectConst node) {
    if (node.isNull()) return;
    if (node["device_name"].is<const char*>())  _deviceName   = (const char*)node["device_name"];
    if (node["manufacturer"].is<const char*>()) _manufacturer = (const char*)node["manufacturer"];
    if (node["battery_level"].is<int>())         _batteryLevel = (uint8_t)node["battery_level"];
    if (node["layout"].is<const char*>()) {
        String l = (const char*)node["layout"];
        l.toLowerCase();
        _azerty = (l == "azerty");
    }
}

void KeyboardApp::saveConfig(JsonObject node) {
    node["device_name"]   = _deviceName;
    node["manufacturer"]  = _manufacturer;
    node["battery_level"] = _batteryLevel;
    node["layout"]        = _azerty ? "azerty" : "us";
}
