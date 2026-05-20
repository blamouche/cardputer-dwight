#include "MenuApp.h"

#include <M5Cardputer.h>
#include <algorithm>
#include <string.h>

#include "core/AppManager.h"
#include "core/ConfigManager.h"
#include "core/DwightFace.h"
#include "core/HardwareConfig.h"
#include "core/Theme.h"

namespace {

// Returns the list of apps (in display order) to actually show in the menu.
// Honors config.menu.enabled_apps order/filter; falls back to registration
// order. The launcher itself (apps[0]) is always excluded. The Config app is
// always forced to the end of the list, whatever its position in enabled_apps,
// since it is a settings entry that belongs at the bottom of the menu.
std::vector<App*> visibleApps(AppManager& mgr) {
    std::vector<App*> out;
    const auto& all = mgr.apps();
    const auto& enabled = mgr.config()->menu().enabledApps;

    if (!enabled.empty()) {
        for (const auto& id : enabled) {
            for (auto* a : all) {
                if (a == mgr.home()) continue;
                if (id == a->id()) { out.push_back(a); break; }
            }
        }
    } else {
        for (auto* a : all) {
            if (a == mgr.home()) continue;
            out.push_back(a);
        }
    }

    // Keep the Config app pinned to the bottom of the menu.
    auto it = std::stable_partition(out.begin(), out.end(), [](App* a) {
        return strcmp(a->id(), "config") != 0;
    });
    (void)it;

    return out;
}

}  // namespace

void MenuApp::setup(AppManager& mgr) {
    _mgr = &mgr;
}

void MenuApp::onEnter() {
    _selection = 0;
    _dirty = true;
}

void MenuApp::loop() {
    auto items = visibleApps(*_mgr);
    if (!items.empty() && _selection >= (int)items.size()) {
        _selection = 0;
        _dirty = true;
    }

    if (_mgr->inputChanged() && _mgr->inputPressed()) {
        auto state = M5Cardputer.Keyboard.keysState();
        for (char c : state.word) {
            if (items.empty()) break;
            if (c == ';') {  // up
                _selection = (_selection - 1 + items.size()) % items.size();
                _dirty = true;
            } else if (c == '.') {  // down
                _selection = (_selection + 1) % items.size();
                _dirty = true;
            }
        }
        if (state.enter && !items.empty()) {
            _mgr->switchTo(items[_selection]);
            return;
        }
    }

    uint32_t now = millis();
    if (_dirty || now - _lastDrawMs >= 100) {
        _lastDrawMs = now;
        _dirty = false;
        draw();
    }
}

// Word-wrap `text` into up to `maxLines` lines of at most `maxChars` chars.
// Returns the number of lines produced.
static int wrapText(const char* text, int maxChars, int maxLines, String out[]) {
    int count = 0;
    String cur;
    const char* p = text;
    while (*p && count < maxLines) {
        const char* ws = p;
        while (*p && *p != ' ') p++;
        String word = String(ws).substring(0, (int)(p - ws));
        while (*p == ' ') p++;
        if (cur.length() == 0) {
            cur = word;
        } else if ((int)(cur.length() + 1 + word.length()) <= maxChars) {
            cur += ' ';
            cur += word;
        } else {
            out[count++] = cur;
            cur = word;
        }
    }
    if (count < maxLines && cur.length() > 0) out[count++] = cur;
    return count;
}

void MenuApp::draw() {
    auto* canvas = _mgr->canvas();
    auto items = visibleApps(*_mgr);

    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    // Full Dwight portrait on the left (same as the boot splash).
    dwightDrawAsciiFace(canvas, 0);

    // Visible window of up to 4 items around the selection.
    const int visible = 4;

    // Reserve a thin column on the right for the scrollbar, but only when the
    // list overflows the visible window (otherwise the bar carries no info).
    const bool hasScrollbar = (int)items.size() > visible;
    const int sbW = 3;
    const int sbGap = 3;
    const int rightReserve = hasScrollbar ? (sbW + sbGap) : 0;

    // The menu list sits to the right of the portrait.
    const int listX = dwightAsciiFaceWidth() + 6;
    const int listW = DISPLAY_W - listX - 2 - rightReserve;
    const int maxChars = listW / 6;

    if (items.empty()) {
        String lines[3];
        int n = wrapText("No app enabled. Edit menu.enabled_apps on SD.",
                         maxChars, 3, lines);
        int y = (DISPLAY_H - n * 12) / 2;
        canvas->setTextColor(gTheme.warning);
        for (int i = 0; i < n; i++) {
            canvas->setCursor(listX, y);
            canvas->print(lines[i]);
            y += 12;
        }
        canvas->pushSprite(0, 0);
        return;
    }

    const int rowH = 30;
    int top = _selection - visible / 2;
    if (top < 0) top = 0;
    if (top + visible > (int)items.size()) top = std::max(0, (int)items.size() - visible);

    int shown = std::min(visible, (int)items.size() - top);
    int y = std::max(4, (DISPLAY_H - shown * rowH) / 2);

    for (int i = 0; i < shown; i++) {
        int idx = top + i;
        bool sel = (idx == _selection);

        String lines[2];
        int n = wrapText(items[idx]->name(), maxChars, 2, lines);

        if (sel) {
            canvas->fillRoundRect(listX - 4, y, listW + 4, rowH - 4, 3, gTheme.accent);
            canvas->setTextColor(gTheme.background);
        } else {
            canvas->setTextColor(gTheme.foreground);
        }
        int ty = y + (rowH - 4 - n * 11) / 2 + 1;
        for (int k = 0; k < n; k++) {
            canvas->setCursor(listX, ty);
            canvas->print(lines[k]);
            ty += 11;
        }
        y += rowH;
    }

    // Vertical scrollbar: a muted track with a foreground thumb whose size and
    // position reflect the visible window within the full list. The thumb uses
    // `foreground` (rather than `accent`) so it stays clearly contrasted with
    // both the track and the background in every theme, including "light".
    if (hasScrollbar) {
        const int total = (int)items.size();
        const int sbX = DISPLAY_W - sbW - 1;
        const int sbY = 4;
        const int sbH = DISPLAY_H - 8;

        canvas->fillRoundRect(sbX, sbY, sbW, sbH, 1, gTheme.muted);

        int thumbH = std::max(8, sbH * visible / total);
        int maxTop = total - visible;  // > 0 because hasScrollbar
        int thumbY = sbY + (sbH - thumbH) * top / maxTop;
        canvas->fillRoundRect(sbX, thumbY, sbW, thumbH, 1, gTheme.foreground);
    }

    canvas->pushSprite(0, 0);
}
