#include "PresentationRemoteApp.h"

#include <M5Cardputer.h>
#include <stdio.h>

// M5Cardputer's Keyboard_def.h defines these as numeric macros, which clashes
// with BleKeyboard's `const uint8_t KEY_* = ...` declarations. We only read the
// Cardputer keyboard via chars (state.word), so dropping the macros is safe.
#undef KEY_LEFT_CTRL
#undef KEY_LEFT_SHIFT
#undef KEY_LEFT_ALT
#undef KEY_FN
#undef KEY_OPT
#undef KEY_BACKSPACE
#undef KEY_TAB
#undef KEY_ENTER

#include <BleKeyboard.h>

#include "core/AppManager.h"
#include "core/BleHid.h"
#include "core/DwightFace.h"
#include "core/HardwareConfig.h"
#include "core/SpeechBubble.h"
#include "core/Theme.h"

void PresentationRemoteApp::setup(AppManager& mgr) {
    _mgr = &mgr;
    _mapping.left  = KEY_LEFT_ARROW;
    _mapping.right = KEY_RIGHT_ARROW;
    _mapping.up    = KEY_UP_ARROW;
    _mapping.down  = KEY_DOWN_ARROW;
}

void PresentationRemoteApp::loadConfig(JsonObjectConst node) {
    if (node.isNull()) return;
    if (node["device_name"].is<const char*>())   _deviceName   = (const char*)node["device_name"];
    if (node["manufacturer"].is<const char*>())  _manufacturer = (const char*)node["manufacturer"];
    if (node["battery_level"].is<int>())         _batteryLevel = (uint8_t)node["battery_level"];

    JsonObjectConst keys = node["keys"];
    if (!keys.isNull()) {
        if (keys["left"].is<const char*>())  _mapping.left  = parseKey(keys["left"]);
        if (keys["right"].is<const char*>()) _mapping.right = parseKey(keys["right"]);
        if (keys["up"].is<const char*>())    _mapping.up    = parseKey(keys["up"]);
        if (keys["down"].is<const char*>())  _mapping.down  = parseKey(keys["down"]);
    }
}

void PresentationRemoteApp::saveConfig(JsonObject node) {
    node["device_name"]   = _deviceName;
    node["manufacturer"]  = _manufacturer;
    node["battery_level"] = _batteryLevel;
    JsonObject keys = node["keys"].to<JsonObject>();
    keys["left"]  = keyName(_mapping.left);
    keys["right"] = keyName(_mapping.right);
    keys["up"]    = keyName(_mapping.up);
    keys["down"]  = keyName(_mapping.down);
}

void PresentationRemoteApp::ensureBle() {
    BleHid::begin(_deviceName.c_str(), _manufacturer.c_str(), _batteryLevel);
}

void PresentationRemoteApp::onEnter() {
    ensureBle();
    _currentDir = DIR_NONE;
    _lastDrawMs = 0;
    _startMs = millis();  // restart the presentation timer
    draw();
}

void PresentationRemoteApp::onExit() {
    // BLE stack is intentionally kept alive across app switches so the host
    // pairing survives navigating in/out. Set _ble=nullptr only on full
    // reboot. If you'd rather stop BLE here, call _ble->end() / delete.
}

void PresentationRemoteApp::loop() {
    if (_mgr->inputChanged() && _mgr->inputPressed()) {
        auto state = M5Cardputer.Keyboard.keysState();
        for (char c : state.word) {
            switch (c) {
                case ',': sendKey(_mapping.left,  DIR_LEFT);  break;
                case '.': sendKey(_mapping.down,  DIR_DOWN);  break;
                case ';': sendKey(_mapping.up,    DIR_UP);    break;
                case '/': sendKey(_mapping.right, DIR_RIGHT); break;
            }
        }
    }

    uint32_t now = millis();
    if (now - _lastDrawMs >= 33) {
        _lastDrawMs = now;
        draw();
    }
}

void PresentationRemoteApp::sendKey(uint8_t code, Direction dir) {
    if (!BleHid::isConnected() || code == 0) return;
    BleHid::tap(code);
    _currentDir = dir;
    _pressTimeMs = millis();
}

namespace {

// Big filled triangle arrow pointing in `dir` (matches the Direction enum).
void drawDirArrow(M5Canvas* c, int cx, int cy, int dir, uint16_t color) {
    const int s = 16;
    switch (dir) {
        case 1: c->fillTriangle(cx - s, cy, cx + s, cy - s, cx + s, cy + s, color); break;  // LEFT
        case 2: c->fillTriangle(cx + s, cy, cx - s, cy - s, cx - s, cy + s, color); break;  // RIGHT
        case 3: c->fillTriangle(cx, cy - s, cx - s, cy + s, cx + s, cy + s, color); break;  // UP
        case 4: c->fillTriangle(cx, cy + s, cx - s, cy - s, cx + s, cy - s, color); break;  // DOWN
        default: break;
    }
}

}  // namespace

void PresentationRemoteApp::draw() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    bool connected = BleHid::isConnected();

    // BLE status, top-right (clear of the portrait on the left).
    const char* statusStr = connected ? "CONNECTED" : "WAITING";
    const int ICON_W = 7, ICON_PAD = 2;
    int totalW = ICON_W + ICON_PAD + 6 * (int)strlen(statusStr);
    int startX = DISPLAY_W - totalW;
    int icx = startX + ICON_W / 2;
    int icy = 4;
    uint16_t scolor = connected ? gTheme.success : gTheme.warning;
    if (connected) {
        canvas->fillCircle(icx, icy, 3, scolor);
    } else {
        canvas->drawCircle(icx, icy, 3, scolor);
        canvas->drawLine(icx, icy, icx,     icy - 2, scolor);
        canvas->drawLine(icx, icy, icx + 2, icy,     scolor);
    }
    canvas->setTextColor(scolor);
    canvas->setCursor(startX + ICON_W + ICON_PAD, 1);
    canvas->print(statusStr);

    // Full Dwight portrait on the left (same as the boot splash).
    dwightDrawAsciiFace(canvas, 0);

    const int colX = dwightAsciiFaceWidth() + 6;
    const int colW = DISPLAY_W - colX - 4;

    // Big presentation timer (counts up from launch) at the top of the column.
    uint32_t elapsed = (millis() - _startMs) / 1000;
    char tbuf[12];
    snprintf(tbuf, sizeof(tbuf), "%02u:%02u",
             (unsigned)(elapsed / 60), (unsigned)(elapsed % 60));
    canvas->setTextSize(3);
    canvas->setTextColor(gTheme.foreground);
    int tw = 18 * (int)strlen(tbuf);  // size-3 glyph advance = 18 px
    canvas->setCursor(colX + (colW - tw) / 2, 16);
    canvas->print(tbuf);

    // Speech bubble below the timer.
    const int bx = colX, by = 48;
    const int bw = colW, bh = DISPLAY_H - by - 5;
    drawSpeechBubble(canvas, bx, by, bw, bh, gTheme.accent);
    const int innerX = bx + 8, innerTop = by + 10, innerW = bw - 16;

    if (!connected) {
        drawWrappedText(canvas,
                    "Pair me over Bluetooth first. I don't wait forever.",
                    innerX, innerTop, innerW, gTheme.foreground, 11);
    } else if (_currentDir != DIR_NONE && millis() - _pressTimeMs < 1500) {
        drawDirArrow(canvas, bx + bw / 2, by + bh / 2 - 6, (int)_currentDir,
                     gTheme.foreground);
        const char* label = dirLabel(_currentDir);
        canvas->setTextSize(1);
        canvas->setTextColor(gTheme.foreground);
        int lx = bx + (bw - 6 * (int)strlen(label)) / 2;
        canvas->setCursor(lx, by + bh - 20);
        canvas->print(label);
    } else {
        drawWrappedText(canvas,
                    "Ready. Press , . ; / and I move your slides.",
                    innerX, innerTop, innerW, gTheme.foreground, 11);
    }

    canvas->pushSprite(0, 0);
}

const char* PresentationRemoteApp::dirLabel(Direction d) {
    switch (d) {
        case DIR_LEFT:  return "LEFT";
        case DIR_RIGHT: return "RIGHT";
        case DIR_UP:    return "UP";
        case DIR_DOWN:  return "DOWN";
        default:        return "";
    }
}

uint8_t PresentationRemoteApp::parseKey(const char* name) {
    String s(name);
    s.toUpperCase();
    if (s == "LEFT"  || s == "LEFT_ARROW")  return KEY_LEFT_ARROW;
    if (s == "RIGHT" || s == "RIGHT_ARROW") return KEY_RIGHT_ARROW;
    if (s == "UP"    || s == "UP_ARROW")    return KEY_UP_ARROW;
    if (s == "DOWN"  || s == "DOWN_ARROW")  return KEY_DOWN_ARROW;
    if (s == "PAGE_UP"   || s == "PAGEUP")   return KEY_PAGE_UP;
    if (s == "PAGE_DOWN" || s == "PAGEDOWN") return KEY_PAGE_DOWN;
    if (s == "HOME")   return KEY_HOME;
    if (s == "END")    return KEY_END;
    if (s == "ESC"    || s == "ESCAPE") return KEY_ESC;
    if (s == "ENTER"  || s == "RETURN") return KEY_RETURN;
    if (s == "TAB")    return KEY_TAB;
    if (s == "SPACE")  return ' ';
    if (s == "F5")     return KEY_F5;
    if (s.length() == 1) return (uint8_t)s[0];
    return 0;
}

const char* PresentationRemoteApp::keyName(uint8_t code) {
    if (code == KEY_LEFT_ARROW)  return "LEFT_ARROW";
    if (code == KEY_RIGHT_ARROW) return "RIGHT_ARROW";
    if (code == KEY_UP_ARROW)    return "UP_ARROW";
    if (code == KEY_DOWN_ARROW)  return "DOWN_ARROW";
    if (code == KEY_PAGE_UP)     return "PAGE_UP";
    if (code == KEY_PAGE_DOWN)   return "PAGE_DOWN";
    if (code == KEY_HOME)        return "HOME";
    if (code == KEY_END)         return "END";
    if (code == KEY_ESC)         return "ESC";
    if (code == KEY_RETURN)      return "ENTER";
    if (code == KEY_TAB)         return "TAB";
    if (code == KEY_F5)          return "F5";
    if (code == ' ')             return "SPACE";
    return "";
}
