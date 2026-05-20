#include "ConfigApp.h"

#include <M5Cardputer.h>
#include <algorithm>
#include <string.h>

#include "core/AppManager.h"
#include "core/ConfigManager.h"
#include "core/DwightFace.h"
#include "core/HardwareConfig.h"
#include "core/Theme.h"

#include "apps/dwight_unlock/DwightUnlockApp.h"

namespace {

// Top-level sections shown when the config app opens.
const char* const kSections[] = {"Computers", "Appearance", "Sound", "Power"};
constexpr int kSectionCount = sizeof(kSections) / sizeof(kSections[0]);

// Volume adjustment step for the Sound section (0..255 in ~16 steps).
constexpr int kVolumeStep = 16;
constexpr int kSoundRows = 2;  // 0 = beeps on/off, 1 = volume

}  // namespace

void ConfigApp::setup(AppManager& mgr) {
    _mgr = &mgr;
}

void ConfigApp::onEnter() {
    _mode = Mode::Sections;
    _sectionSel = 0;
    _selection = 0;
    _editIdx = -1;
    _dirty = true;
}

void ConfigApp::loop() {
    if (_mgr->inputChanged() && _mgr->inputPressed()) {
        switch (_mode) {
            case Mode::Sections:   handleSectionKeys();    break;
            case Mode::Computers:  handleComputerKeys();   break;
            case Mode::Appearance: handleAppearanceKeys(); break;
            case Mode::Sound:      handleSoundKeys();      break;
            case Mode::Power:      handlePowerKeys();      break;
            default:               handleEditKeys();       break;
        }
        _dirty = true;
    }

    uint32_t now = millis();
    if (_dirty || now - _lastDrawMs >= 120) {
        _lastDrawMs = now;
        _dirty = false;
        draw();
    }
}

// Back key navigation: edit -> Computers, a section -> Sections, and only the
// Sections menu actually leaves the app.
bool ConfigApp::onBackRequested() {
    switch (_mode) {
        case Mode::EditName:
        case Mode::EditPass:
            _mode = Mode::Computers;
            _dirty = true;
            return false;
        case Mode::Computers:
        case Mode::Appearance:
        case Mode::Sound:
        case Mode::Power:
            _mode = Mode::Sections;
            _dirty = true;
            return false;
        default:
            return true;
    }
}

void ConfigApp::handleSectionKeys() {
    auto st = M5Cardputer.Keyboard.keysState();
    for (char c : st.word) {
        if (c == ';') {
            _sectionSel = (_sectionSel - 1 + kSectionCount) % kSectionCount;
        } else if (c == '.') {
            _sectionSel = (_sectionSel + 1) % kSectionCount;
        }
    }
    if (st.enter) {
        if (_sectionSel == 0) {
            _mode = Mode::Computers;
            _selection = 0;
        } else if (_sectionSel == 1) {
            _mode = Mode::Appearance;
            int cur = themeIndexOf(_mgr->config()->system().colorTheme);
            _appearanceSel = cur < 0 ? 0 : cur;
        } else if (_sectionSel == 2) {
            _mode = Mode::Sound;
            _soundSel = 0;
        } else {
            _mode = Mode::Power;
        }
    }
}

void ConfigApp::handleComputerKeys() {
    if (!_unlock) return;
    auto& list = _unlock->computers();
    auto st = M5Cardputer.Keyboard.keysState();

    for (char c : st.word) {
        if (c == ';') {
            if (!list.empty())
                _selection = (_selection - 1 + (int)list.size()) % (int)list.size();
        } else if (c == '.') {
            if (!list.empty())
                _selection = (_selection + 1) % (int)list.size();
        } else if (c == 'n' || c == 'N') {
            beginNew();
            return;
        } else if (c == 'd' || c == 'D') {
            deleteSelected();
            return;
        }
    }
    if (st.enter && !list.empty()) beginEdit(_selection);
}

void ConfigApp::handleAppearanceKeys() {
    auto st = M5Cardputer.Keyboard.keysState();
    int n = themeCount();
    for (char c : st.word) {
        if (c == ';') {
            _appearanceSel = (_appearanceSel - 1 + n) % n;
        } else if (c == '.') {
            _appearanceSel = (_appearanceSel + 1) % n;
        }
    }
    if (st.enter) applySelectedTheme();
}

// Sound section: `;`/`.` move between the beeps toggle and the volume row.
// On the beeps row, `,`/`/` or Enter flip the master switch. On the volume row,
// `,`/`/` step the level down/up and play an audible preview. Every change is
// persisted to the SD config.
void ConfigApp::handleSoundKeys() {
    auto st = M5Cardputer.Keyboard.keysState();
    auto& sys = _mgr->config()->mutSystem();
    bool changed = false;
    bool preview = false;

    for (char c : st.word) {
        if (c == ';') {
            _soundSel = (_soundSel - 1 + kSoundRows) % kSoundRows;
        } else if (c == '.') {
            _soundSel = (_soundSel + 1) % kSoundRows;
        } else if (c == ',' || c == '/') {
            int dir = (c == '/') ? 1 : -1;
            if (_soundSel == 0) {
                sys.buzzerEnabled = !sys.buzzerEnabled;
                changed = true;
                preview = sys.buzzerEnabled;
            } else {
                int v = (int)sys.buzzerVolume + dir * kVolumeStep;
                if (v < 0) v = 0;
                if (v > 255) v = 255;
                if (v != (int)sys.buzzerVolume) {
                    sys.buzzerVolume = (uint8_t)v;
                    changed = true;
                    preview = true;
                }
            }
        }
    }
    if (st.enter && _soundSel == 0) {
        sys.buzzerEnabled = !sys.buzzerEnabled;
        changed = true;
        preview = sys.buzzerEnabled;
    }

    if (changed) {
        if (preview) previewBeep();
        persist();
    }
}

// Power section: a single eco-mode toggle. Enter or `,`/`/` flip it. Turning it
// on disables the LED and sound, caps the screen brightness and lets the screen
// switch off after a short idle delay; the brightness cap is applied live here
// (the LED and screen-off timer are handled in the main loop).
void ConfigApp::handlePowerKeys() {
    auto st = M5Cardputer.Keyboard.keysState();
    auto& sys = _mgr->config()->mutSystem();
    bool toggle = st.enter;
    for (char c : st.word) {
        if (c == ',' || c == '/') toggle = true;
    }
    if (toggle) {
        sys.powerSaveEnabled = !sys.powerSaveEnabled;
        M5Cardputer.Display.setBrightness(_mgr->config()->effectiveBrightness());
        persist();
    }
}

// Short feedback tone at the current setting, so adjusting the level is audible.
void ConfigApp::previewBeep() {
    const auto& sys = _mgr->config()->system();
    if (!_mgr->config()->soundsEnabled() || sys.buzzerVolume == 0) return;
    auto& spk = M5Cardputer.Speaker;
    if (!_spkReady) {
        spk.begin();
        _spkReady = true;
    }
    spk.setVolume(sys.buzzerVolume);
    spk.tone(880, 90);
}

void ConfigApp::handleEditKeys() {
    auto st = M5Cardputer.Keyboard.keysState();
    String& buf = (_mode == Mode::EditName) ? _nameBuf : _passBuf;

    for (char c : st.word) {
        if ((uint8_t)c >= 32 && (uint8_t)c < 127) buf += c;
    }
    if (st.del && buf.length() > 0) buf.remove(buf.length() - 1);

    if (st.enter) {
        if (_mode == Mode::EditName) {
            if (_nameBuf.length() == 0) return;  // a name is required
            _mode = Mode::EditPass;
        } else {  // EditPass: commit the entry
            if (!_unlock) { _mode = Mode::Computers; return; }
            auto& list = _unlock->computers();
            if (_editIdx >= 0 && _editIdx < (int)list.size()) {
                list[_editIdx].name     = _nameBuf;
                list[_editIdx].password = _passBuf;
                _selection = _editIdx;
            } else {
                list.push_back({_nameBuf, _passBuf});
                _selection = (int)list.size() - 1;
            }
            persist();
            _mode = Mode::Computers;
        }
    }
}

void ConfigApp::beginNew() {
    _editIdx = -1;
    _nameBuf = "";
    _passBuf = "";
    _mode = Mode::EditName;
}

void ConfigApp::beginEdit(int idx) {
    if (!_unlock) return;
    auto& list = _unlock->computers();
    if (idx < 0 || idx >= (int)list.size()) return;
    _editIdx = idx;
    _nameBuf = list[idx].name;
    _passBuf = list[idx].password;
    _mode = Mode::EditName;
}

void ConfigApp::deleteSelected() {
    if (!_unlock) return;
    auto& list = _unlock->computers();
    if (list.empty()) return;
    list.erase(list.begin() + _selection);
    if (_selection >= (int)list.size()) _selection = std::max(0, (int)list.size() - 1);
    persist();
}

// Applies the highlighted theme live and saves it to the SD config so it
// survives a reboot.
void ConfigApp::applySelectedTheme() {
    if (!_mgr || !_mgr->config()) return;
    String id = themeId(_appearanceSel);
    _mgr->config()->mutSystem().colorTheme = id;
    themeApply(id);
    persist();
}

void ConfigApp::persist() {
    if (_mgr && _mgr->config()) _mgr->config()->save(_mgr->apps());
}

void ConfigApp::draw() {
    switch (_mode) {
        case Mode::Sections:   drawSections();   break;
        case Mode::Computers:  drawComputers();  break;
        case Mode::Appearance: drawAppearance(); break;
        case Mode::Sound:      drawSound();      break;
        case Mode::Power:      drawPower();      break;
        default:               drawEdit();       break;
    }
}

void ConfigApp::drawSections() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    const int x = 6;
    const int w = DISPLAY_W - 12;

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(x, 3);
    canvas->print("CONFIG");

    const int listTop = 24;
    const int rowH = 20;
    int y = listTop;
    for (int i = 0; i < kSectionCount; ++i) {
        bool sel = (i == _sectionSel);
        if (sel) {
            canvas->fillRoundRect(x - 2, y - 1, w + 2, rowH - 2, 2, gTheme.accent);
            canvas->setTextColor(gTheme.background);
        } else {
            canvas->setTextColor(gTheme.foreground);
        }
        canvas->setCursor(x, y + 4);
        canvas->print(kSections[i]);
        y += rowH;
    }

    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(x, DISPLAY_H - 11);
    canvas->print("Enter=open  ;/.=move  `=back");
    canvas->pushSprite(0, 0);
}

void ConfigApp::drawComputers() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    const int x = 6;
    const int w = DISPLAY_W - 12;
    const int maxChars = w / 6;

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(x, 3);
    canvas->print("CONFIG / Computers");

    if (!_unlock || _unlock->computers().empty()) {
        canvas->setTextColor(gTheme.muted);
        canvas->setCursor(x, 28);
        canvas->print("No computer yet.");
        canvas->setCursor(x, 44);
        canvas->print("Press n to add one.");
    } else {
        auto& list = _unlock->computers();
        const int listTop = 22;
        const int rowH = 18;
        const int maxRows = (DISPLAY_H - listTop - 14) / rowH;
        int top = _selection - maxRows / 2;
        if (top < 0) top = 0;
        if (top + maxRows > (int)list.size())
            top = std::max(0, (int)list.size() - maxRows);
        int shown = std::min(maxRows, (int)list.size() - top);

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
            String nm = list[idx].name;
            if (nm.length() == 0) nm = "(unnamed)";
            String tag = list[idx].password.length() ? "  [set]" : "  [empty]";
            int room = maxChars - (int)tag.length();
            if (room > 0 && (int)nm.length() > room) nm = nm.substring(0, room);
            canvas->setCursor(x, y + 4);
            canvas->print(nm + tag);
            y += rowH;
        }
    }

    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(x, DISPLAY_H - 11);
    canvas->print("Ent=edit n=new d=del ;/.=move");
    canvas->pushSprite(0, 0);
}

void ConfigApp::drawAppearance() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    const int x = 6;
    const int w = DISPLAY_W - 12;

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(x, 3);
    canvas->print("CONFIG / Appearance");

    const String active = _mgr->config()->system().colorTheme;
    const int listTop = 22;
    const int rowH = 18;
    const int n = themeCount();
    const int maxRows = (DISPLAY_H - listTop - 14) / rowH;
    int top = _appearanceSel - maxRows / 2;
    if (top < 0) top = 0;
    if (top + maxRows > n) top = std::max(0, n - maxRows);
    int shown = std::min(maxRows, n - top);

    int y = listTop;
    for (int i = 0; i < shown; ++i) {
        int idx = top + i;
        bool sel = (idx == _appearanceSel);
        if (sel) {
            canvas->fillRoundRect(x - 2, y - 1, w + 2, rowH - 2, 2, gTheme.accent);
            canvas->setTextColor(gTheme.background);
        } else {
            canvas->setTextColor(gTheme.foreground);
        }
        String label = themeLabel(idx);
        if (active == themeId(idx)) label += "  *";
        canvas->setCursor(x, y + 4);
        canvas->print(label);
        y += rowH;
    }

    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(x, DISPLAY_H - 11);
    canvas->print("Enter=apply  ;/.=move  `=back");
    canvas->pushSprite(0, 0);
}

void ConfigApp::drawSound() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    const int x = 6;
    const int w = DISPLAY_W - 12;

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(x, 3);
    canvas->print("CONFIG / Sound");

    const auto& sys = _mgr->config()->system();
    const int rowH = 20;
    int y = 26;

    // Row 0: master beeps on/off.
    {
        bool sel = (_soundSel == 0);
        if (sel) {
            canvas->fillRoundRect(x - 2, y - 1, w + 2, rowH - 2, 2, gTheme.accent);
            canvas->setTextColor(gTheme.background);
        } else {
            canvas->setTextColor(gTheme.foreground);
        }
        canvas->setCursor(x, y + 4);
        canvas->print("Beeps:");
        canvas->setCursor(x + 90, y + 4);
        canvas->print(sys.buzzerEnabled ? "On" : "Off");
        y += rowH;
    }

    // Row 1: volume level with a small bar gauge.
    {
        bool sel = (_soundSel == 1);
        uint16_t fg = sys.buzzerEnabled ? gTheme.foreground : gTheme.muted;
        if (sel) {
            canvas->fillRoundRect(x - 2, y - 1, w + 2, rowH - 2, 2, gTheme.accent);
            fg = gTheme.background;
        }
        canvas->setTextColor(fg);
        canvas->setCursor(x, y + 4);
        canvas->print("Volume:");

        const int barX = x + 64, barY = y + 3, barW = w - 90, barH = 9;
        canvas->drawRect(barX, barY, barW, barH, fg);
        int fill = (int)((barW - 2) * (int)sys.buzzerVolume / 255);
        if (fill > 0) canvas->fillRect(barX + 1, barY + 1, fill, barH - 2, fg);
        y += rowH;
    }

    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(x, DISPLAY_H - 11);
    canvas->print(",//=change ;/.=move Ent=toggle");
    canvas->pushSprite(0, 0);
}

void ConfigApp::drawPower() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    const int x = 6;
    const int w = DISPLAY_W - 12;

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(x, 3);
    canvas->print("CONFIG / Power");

    const bool on = _mgr->config()->system().powerSaveEnabled;
    const int rowH = 20;
    int y = 26;

    // Single row: the eco-mode toggle (always selected).
    canvas->fillRoundRect(x - 2, y - 1, w + 2, rowH - 2, 2, gTheme.accent);
    canvas->setTextColor(gTheme.background);
    canvas->setCursor(x, y + 4);
    canvas->print("Power save:");
    canvas->setCursor(x + 110, y + 4);
    canvas->print(on ? "On" : "Off");
    y += rowH + 6;

    // What eco mode does, so the trade-off is clear on device.
    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(x, y);      canvas->print("LED off, no sound,");
    canvas->setCursor(x, y + 12); canvas->print("dim screen, off after 15s.");

    canvas->setCursor(x, DISPLAY_H - 11);
    canvas->print(",// or Enter = toggle  `=back");
    canvas->pushSprite(0, 0);
}

void ConfigApp::drawEdit() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    const int x = 6;
    const int w = DISPLAY_W - 12;
    const int maxChars = w / 6;
    bool blink = (millis() / 500) % 2 == 0;

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(x, 3);
    canvas->print("CONFIG / Computers");

    canvas->setTextColor(gTheme.foreground);
    canvas->setCursor(x, 26);
    canvas->print(_editIdx >= 0 ? "Edit computer" : "New computer");

    // Name field.
    canvas->setTextColor(_mode == Mode::EditName ? gTheme.accent : gTheme.muted);
    canvas->setCursor(x, 48);
    canvas->print("Name:");
    canvas->setTextColor(gTheme.foreground);
    {
        String shown = _nameBuf;
        if (_mode == Mode::EditName && blink) shown += "_";
        int room = maxChars - 6;
        if ((int)shown.length() > room) shown = shown.substring(shown.length() - room);
        canvas->setCursor(x + 36, 48);
        canvas->print(shown);
    }

    // Password field (shown in clear — it's your own device).
    canvas->setTextColor(_mode == Mode::EditPass ? gTheme.accent : gTheme.muted);
    canvas->setCursor(x, 68);
    canvas->print("Pass:");
    canvas->setTextColor(gTheme.foreground);
    {
        String shown = _passBuf;
        if (_mode == Mode::EditPass && blink) shown += "_";
        int room = maxChars - 6;
        if ((int)shown.length() > room) shown = shown.substring(shown.length() - room);
        canvas->setCursor(x + 36, 68);
        canvas->print(shown);
    }

    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(x, DISPLAY_H - 11);
    canvas->print("Enter=next/save  `=cancel");
    canvas->pushSprite(0, 0);
}
