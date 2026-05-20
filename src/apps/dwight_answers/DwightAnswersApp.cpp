#include "DwightAnswersApp.h"

#include <M5Cardputer.h>
#include <algorithm>
#include <string.h>

#include "core/AppManager.h"
#include "core/AzertyText.h"
#include "core/BleHid.h"
#include "core/ConfigManager.h"
#include "core/HardwareConfig.h"
#include "core/Theme.h"

void DwightAnswersApp::setup(AppManager& mgr) {
    _mgr = &mgr;
}

void DwightAnswersApp::onEnter() {
    BleHid::begin(_deviceName.c_str(), _manufacturer.c_str(), _batteryLevel);
    if (_selection >= (int)_phrases.size()) _selection = 0;
    _mode = Mode::List;
    _justTyped = false;
    _lastDrawMs = 0;
    _dirty = true;
}

void DwightAnswersApp::loop() {
    if (_mgr->inputChanged() && _mgr->inputPressed()) {
        if (_mode == Mode::List) handleListKeys();
        else                     handleEditKeys();
        _dirty = true;
    }

    uint32_t now = millis();
    if (_dirty || now - _lastDrawMs >= 80) {
        _lastDrawMs = now;
        _dirty = false;
        draw();
    }
}

// Back key: from the editor, cancel back to the list; from the list, leave the
// app (default behaviour).
bool DwightAnswersApp::onBackRequested() {
    if (_mode == Mode::Edit) {
        _mode = Mode::List;
        _dirty = true;
        return false;
    }
    return true;
}

void DwightAnswersApp::handleListKeys() {
    auto st = M5Cardputer.Keyboard.keysState();
    for (char c : st.word) {
        if (c == ';') {
            if (!_phrases.empty())
                _selection = (_selection - 1 + (int)_phrases.size()) % (int)_phrases.size();
        } else if (c == '.') {
            if (!_phrases.empty())
                _selection = (_selection + 1) % (int)_phrases.size();
        } else if (c == 'n' || c == 'N') {
            beginNew();
            return;
        } else if (c == 'e' || c == 'E') {
            if (!_phrases.empty()) beginEdit(_selection);
            return;
        } else if (c == 'd' || c == 'D') {
            deleteSelected();
            return;
        }
    }
    if (st.enter && !_phrases.empty() && BleHid::isReady()) {
        typeSelected();
    }
}

void DwightAnswersApp::handleEditKeys() {
    auto st = M5Cardputer.Keyboard.keysState();
    for (char c : st.word) {
        if ((uint8_t)c >= 32 && (uint8_t)c < 127) _editBuf += c;
    }
    if (st.del && _editBuf.length() > 0) _editBuf.remove(_editBuf.length() - 1);

    if (st.enter) {
        String trimmed = _editBuf;
        trimmed.trim();
        if (trimmed.length() == 0) return;  // ignore empty phrases
        if (_editIdx >= 0 && _editIdx < (int)_phrases.size()) {
            _phrases[_editIdx] = _editBuf;
            _selection = _editIdx;
        } else {
            _phrases.push_back(_editBuf);
            _selection = (int)_phrases.size() - 1;
        }
        persist();
        _mode = Mode::List;
    }
}

void DwightAnswersApp::beginNew() {
    _editIdx = -1;
    _editBuf = "";
    _mode = Mode::Edit;
}

void DwightAnswersApp::beginEdit(int idx) {
    if (idx < 0 || idx >= (int)_phrases.size()) return;
    _editIdx = idx;
    _editBuf = _phrases[idx];
    _mode = Mode::Edit;
}

void DwightAnswersApp::deleteSelected() {
    if (_phrases.empty()) return;
    _phrases.erase(_phrases.begin() + _selection);
    if (_selection >= (int)_phrases.size())
        _selection = std::max(0, (int)_phrases.size() - 1);
    persist();
}

void DwightAnswersApp::typeSelected() {
    if (_selection < 0 || _selection >= (int)_phrases.size()) return;
    azerty::typeText(_phrases[_selection], _azerty);
    _justTyped = true;
    _lastTypeMs = millis();
}

void DwightAnswersApp::persist() {
    if (_mgr && _mgr->config()) _mgr->config()->save(_mgr->apps());
}

void DwightAnswersApp::draw() {
    if (_mode == Mode::Edit) drawEdit();
    else                     drawList();
}

void DwightAnswersApp::drawList() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    const int x = 6;
    const int w = DISPLAY_W - 12;
    const int maxChars = w / 6;

    bool connected = BleHid::isConnected();
    bool warming   = BleHid::isWarmingUp();

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(x, 3);
    canvas->print("DWIGHT ANSWERS");

    // BLE status, top-right. SYNC = post-connect warm-up (typing held back).
    const char* statusStr = !connected ? "WAITING" : (warming ? "SYNC" : "ONLINE");
    canvas->setTextColor((connected && !warming) ? gTheme.success : gTheme.warning);
    int sw = 6 * (int)strlen(statusStr);
    canvas->setCursor(DISPLAY_W - sw - 2, 3);
    canvas->print(statusStr);

    if (_justTyped && millis() - _lastTypeMs < 1500) {
        canvas->setTextColor(gTheme.success);
        canvas->setCursor(x, 60);
        canvas->print("Typed. You're welcome.");
        canvas->setTextColor(gTheme.muted);
        canvas->setCursor(x, DISPLAY_H - 11);
        canvas->print("Ent=type n=new e=edit d=del");
        canvas->pushSprite(0, 0);
        return;
    }
    _justTyped = false;

    if (_phrases.empty()) {
        canvas->setTextColor(gTheme.muted);
        canvas->setCursor(x, 30);
        canvas->print("No answers yet.");
        canvas->setCursor(x, 46);
        canvas->print("Press n to add one.");
        canvas->setCursor(x, DISPLAY_H - 11);
        canvas->print("n=new  `=back");
        canvas->pushSprite(0, 0);
        return;
    }

    // Scrolling window of phrases around the selection.
    const int listTop = 22;
    const int rowH = 18;
    const int maxRows = (DISPLAY_H - listTop - 14) / rowH;
    int top = _selection - maxRows / 2;
    if (top < 0) top = 0;
    if (top + maxRows > (int)_phrases.size())
        top = std::max(0, (int)_phrases.size() - maxRows);
    int shown = std::min(maxRows, (int)_phrases.size() - top);

    int y = listTop;
    for (int i = 0; i < shown; ++i) {
        int idx = top + i;
        bool sel = (idx == _selection);
        if (sel) {
            canvas->fillRoundRect(x - 2, y - 1, w + 2, rowH - 2, 2, gTheme.accent);
            canvas->setTextColor(gTheme.background);
        } else {
            canvas->setTextColor(gTheme.foreground);
        }
        String ph = _phrases[idx];
        if (ph.length() == 0) ph = "(empty)";
        if ((int)ph.length() > maxChars) ph = ph.substring(0, maxChars - 1) + "~";
        canvas->setCursor(x, y + 4);
        canvas->print(ph);
        y += rowH;
    }

    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(x, DISPLAY_H - 11);
    canvas->print(!connected ? "Pair me first  n=new e=edit"
                  : warming   ? "Stabilizing... n=new e=edit"
                              : "Ent=type n=new e=edit d=del");
    canvas->pushSprite(0, 0);
}

void DwightAnswersApp::drawEdit() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    const int x = 6;
    const int w = DISPLAY_W - 12;
    const int maxChars = w / 6;
    bool blink = (millis() / 500) % 2 == 0;

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(x, 3);
    canvas->print("DWIGHT ANSWERS");

    canvas->setTextColor(gTheme.foreground);
    canvas->setCursor(x, 26);
    canvas->print(_editIdx >= 0 ? "Edit answer" : "New answer");

    // Phrase field — show the tail end so the caret stays visible while typing.
    String shown = _editBuf;
    if (blink) shown += "_";
    if ((int)shown.length() > maxChars)
        shown = shown.substring(shown.length() - maxChars);
    canvas->setTextColor(gTheme.foreground);
    canvas->setCursor(x, 56);
    canvas->print(shown);

    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(x, DISPLAY_H - 11);
    canvas->print("Enter=save  `=cancel");
    canvas->pushSprite(0, 0);
}

void DwightAnswersApp::loadConfig(JsonObjectConst node) {
    if (node.isNull()) return;
    if (node["device_name"].is<const char*>())  _deviceName   = (const char*)node["device_name"];
    if (node["manufacturer"].is<const char*>()) _manufacturer = (const char*)node["manufacturer"];
    if (node["battery_level"].is<int>())         _batteryLevel = (uint8_t)node["battery_level"];
    if (node["layout"].is<const char*>()) {
        String l = (const char*)node["layout"];
        l.toLowerCase();
        _azerty = (l == "azerty");
    }

    JsonArrayConst arr = node["phrases"];
    if (!arr.isNull()) {
        _phrases.clear();
        for (JsonVariantConst v : arr) {
            if (v.is<const char*>()) {
                String s = (const char*)v;
                if (s.length()) _phrases.push_back(s);
            }
        }
    }
}

void DwightAnswersApp::saveConfig(JsonObject node) {
    node["device_name"]   = _deviceName;
    node["manufacturer"]  = _manufacturer;
    node["battery_level"] = _batteryLevel;
    node["layout"]        = _azerty ? "azerty" : "us";

    JsonArray arr = node["phrases"].to<JsonArray>();
    for (auto& ph : _phrases) arr.add(ph);
}
