#include "DwightUnlockApp.h"

#include <M5Cardputer.h>
#include <algorithm>
#include <string.h>

#include "core/AppManager.h"
#include "core/AzertyText.h"
#include "core/BleHid.h"
#include "core/DwightFace.h"
#include "core/HardwareConfig.h"
#include "core/SpeechBubble.h"
#include "core/Theme.h"

void DwightUnlockApp::setup(AppManager& mgr) {
    _mgr = &mgr;
}

void DwightUnlockApp::onEnter() {
    BleHid::begin(_deviceName.c_str(), _manufacturer.c_str(), _batteryLevel);
    if (_selection >= (int)_computers.size()) _selection = 0;
    _justTyped = false;
    _lastDrawMs = 0;
    draw();
}

void DwightUnlockApp::loop() {
    if (_mgr->inputChanged() && _mgr->inputPressed()) {
        auto st = M5Cardputer.Keyboard.keysState();
        for (char c : st.word) {
            if (_computers.empty()) break;
            if (c == ';') {
                _selection = (_selection - 1 + (int)_computers.size()) % (int)_computers.size();
            } else if (c == '.') {
                _selection = (_selection + 1) % (int)_computers.size();
            }
        }
        if (st.enter && !_computers.empty() && BleHid::isConnected()) {
            sendPassword(_computers[_selection]);
        }
    }

    uint32_t now = millis();
    if (now - _lastDrawMs >= 60) {
        _lastDrawMs = now;
        draw();
    }
}

void DwightUnlockApp::sendPassword(const UnlockComputer& pc) {
    azerty::typeText(pc.password, _azerty);
    BleHid::tapEnter();
    _justTyped = true;
    _lastTypeMs = millis();
}

void DwightUnlockApp::draw() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    bool connected = BleHid::isConnected();

    // BLE status, top-right (clear of the portrait on the left).
    const char* statusStr = connected ? "CONNECTED" : "WAITING";
    canvas->setTextColor(connected ? gTheme.success : gTheme.warning);
    int sw = 6 * (int)strlen(statusStr);
    canvas->setCursor(DISPLAY_W - sw - 2, 1);
    canvas->print(statusStr);

    // Portrait on the left.
    dwightDrawAsciiFace(canvas, 0);

    const int colX = dwightAsciiFaceWidth() + 6;
    const int colW = DISPLAY_W - colX - 4;

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(colX, 14);
    canvas->print("UNLOCK");

    if (_computers.empty()) {
        drawWrappedText(canvas, "No computer set. Add one in the Config app.",
                        colX, 30, colW, gTheme.warning, 11);
        canvas->pushSprite(0, 0);
        return;
    }

    if (_justTyped && millis() - _lastTypeMs < 2000) {
        drawWrappedText(canvas, "Password sent. Welcome back.",
                        colX, 32, colW, gTheme.success, 11);
    } else {
        _justTyped = false;

        // Scrolling window of computer names around the selection.
        const int listTop = 30;
        const int rowH = 16;
        const int maxRows = (DISPLAY_H - listTop - 14) / rowH;
        int top = _selection - maxRows / 2;
        if (top < 0) top = 0;
        if (top + maxRows > (int)_computers.size())
            top = std::max(0, (int)_computers.size() - maxRows);
        int shown = std::min(maxRows, (int)_computers.size() - top);
        const int maxChars = colW / 6 - 1;

        int y = listTop;
        for (int i = 0; i < shown; ++i) {
            int idx = top + i;
            bool sel = (idx == _selection);
            if (sel) {
                canvas->fillRoundRect(colX - 2, y - 1, colW + 2, rowH - 2, 2, gTheme.accent);
                canvas->setTextColor(gTheme.background);
            } else {
                canvas->setTextColor(gTheme.foreground);
            }
            String nm = _computers[idx].name;
            if (nm.length() == 0) nm = "(unnamed)";
            if ((int)nm.length() > maxChars) nm = nm.substring(0, maxChars);
            canvas->setCursor(colX, y + 3);
            canvas->print(nm);
            y += rowH;
        }
    }

    // Footer hint.
    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(colX, DISPLAY_H - 12);
    canvas->print(connected ? "Enter = unlock" : "Pair me first");

    canvas->pushSprite(0, 0);
}

void DwightUnlockApp::loadConfig(JsonObjectConst node) {
    if (node.isNull()) return;
    if (node["device_name"].is<const char*>())  _deviceName   = (const char*)node["device_name"];
    if (node["manufacturer"].is<const char*>()) _manufacturer = (const char*)node["manufacturer"];
    if (node["battery_level"].is<int>())         _batteryLevel = (uint8_t)node["battery_level"];
    if (node["layout"].is<const char*>()) {
        String l = (const char*)node["layout"];
        l.toLowerCase();
        _azerty = (l == "azerty");
    }

    JsonArrayConst arr = node["computers"];
    if (!arr.isNull()) {
        _computers.clear();
        for (JsonObjectConst pc : arr) {
            UnlockComputer u;
            if (pc["name"].is<const char*>())     u.name     = (const char*)pc["name"];
            if (pc["password"].is<const char*>()) u.password = (const char*)pc["password"];
            if (u.name.length() || u.password.length()) _computers.push_back(u);
        }
    }
}

void DwightUnlockApp::saveConfig(JsonObject node) {
    node["device_name"]   = _deviceName;
    node["manufacturer"]  = _manufacturer;
    node["battery_level"] = _batteryLevel;
    node["layout"]        = _azerty ? "azerty" : "us";

    JsonArray arr = node["computers"].to<JsonArray>();
    for (auto& pc : _computers) {
        JsonObject o = arr.add<JsonObject>();
        o["name"]     = pc.name;
        o["password"] = pc.password;
    }
}
