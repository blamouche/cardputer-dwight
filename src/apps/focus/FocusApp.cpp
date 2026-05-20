#include "FocusApp.h"

#include <M5Cardputer.h>
#include <stdio.h>
#include <string.h>

#include "core/AppManager.h"
#include "core/ConfigManager.h"
#include "core/DwightFace.h"
#include "core/HardwareConfig.h"
#include "core/SpeechBubble.h"
#include "core/Theme.h"

namespace {

const int kMinMinutes = 1;
const int kMaxMinutes = 180;

// How often the running threat escalates (clamped at the last line).
const uint32_t kThreatPeriodMs = 9000;

// Escalating threats Dwight throws while the clock runs.
const char* const kThreats[] = {
    "Eyes down. Work. I am monitoring you.",
    "Bears. Beets. Deadlines. FOCUS, drone.",
    "Idle hands are Jim's workshop. Get back to it.",
    "Distraction is the cousin of failure. Fact.",
    "Slack off and I report you to Michael. And HR. And the bears.",
};
const int kThreatCount = (int)(sizeof(kThreats) / sizeof(kThreats[0]));

}  // namespace

void FocusApp::setup(AppManager& mgr) {
    _mgr = &mgr;
}

void FocusApp::onEnter() {
    _phase     = Phase::Setup;
    _minutes   = _defaultMin;
    _lastDrawMs = 0;
    draw();
}

void FocusApp::startTimer() {
    uint32_t now = millis();
    _durationMs   = (uint32_t)_minutes * 60UL * 1000UL;
    _endMs        = now + _durationMs;
    _phase        = Phase::Running;
    _threatIdx    = 0;
    _nextThreatMs = now + kThreatPeriodMs;
    _lastDrawMs   = 0;
    beep(660, 120);  // crisp "go"
}

void FocusApp::loop() {
    uint32_t now = millis();

    // Countdown elapses in Running and even while you cower in ConfirmAbandon —
    // Dwight does not pause for weakness.
    if (_phase == Phase::Running || _phase == Phase::ConfirmAbandon) {
        if ((int32_t)(now - _endMs) >= 0) {
            _phase = Phase::Done;
            _lastDrawMs = 0;
            beep(880, 120);
            beep(1175, 200);  // grudging fanfare
        }
    }

    if (_mgr->inputChanged() && _mgr->inputPressed()) {
        auto st = M5Cardputer.Keyboard.keysState();

        if (_phase == Phase::Setup) {
            for (char c : st.word) {
                if (c == ';')      _minutes -= 1;   // up arrow
                else if (c == '.') _minutes += 1;   // down arrow
                else if (c == ',') _minutes -= 5;   // left arrow
                else if (c == '/') _minutes += 5;   // right arrow
            }
            if (_minutes < kMinMinutes) _minutes = kMinMinutes;
            if (_minutes > kMaxMinutes) _minutes = kMaxMinutes;
            if (st.enter) { startTimer(); return; }
            _lastDrawMs = 0;
        } else if (_phase == Phase::ConfirmAbandon) {
            // The back key is swallowed by AppManager (-> onBackRequested);
            // any other key here means "fine, back to work".
            _phase = Phase::Running;
            _lastDrawMs = 0;
        } else if (_phase == Phase::Done) {
            if (st.enter) {
                _phase = Phase::Setup;
                _minutes = _defaultMin;
                _lastDrawMs = 0;
            }
        }
    }

    // Escalate the running threat over time.
    if (_phase == Phase::Running && (int32_t)(now - _nextThreatMs) >= 0) {
        _nextThreatMs = now + kThreatPeriodMs;
        if (_threatIdx < kThreatCount - 1) {
            _threatIdx++;
            beep(330, 90);  // a low growl with each new threat
        }
        _lastDrawMs = 0;
    }

    if (now - _lastDrawMs >= 200) {
        _lastDrawMs = now;
        draw();
    }
}

bool FocusApp::onBackRequested() {
    if (_phase == Phase::Running) {
        // First back press: veto the exit and threaten.
        _phase = Phase::ConfirmAbandon;
        _lastDrawMs = 0;
        beep(196, 220);  // ominous low note
        return false;
    }
    // ConfirmAbandon (second back press), Setup or Done: let the user leave.
    return true;
}

void FocusApp::draw() {
    switch (_phase) {
        case Phase::Setup:          drawSetup();   break;
        case Phase::Running:        drawRunning(); break;
        case Phase::ConfirmAbandon: drawRunning(); break;
        case Phase::Done:           drawDone();    break;
    }
}

// Big centered MM:SS at text size 5.
void FocusApp::drawBigTime(uint32_t remainingMs, uint16_t color) {
    auto* canvas = _mgr->canvas();
    uint32_t totalS = (remainingMs + 999) / 1000;  // ceil so it shows 00:00 only at 0
    int mm = (int)(totalS / 60);
    int ss = (int)(totalS % 60);
    if (mm > 99) mm = 99;

    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:%02d", mm, ss);

    const int size = 5;
    const int charW = 6 * size;   // 30 px advance per glyph
    const int charH = 8 * size;   // 40 px tall
    int w = (int)strlen(buf) * charW;
    int x = (DISPLAY_W - w) / 2;
    int y = 46;

    canvas->setTextSize(size);
    canvas->setTextColor(color);
    canvas->setCursor(x, y);
    canvas->print(buf);
    canvas->setTextSize(1);
    (void)charH;
}

void FocusApp::drawSetup() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    // Portrait on the left, briefing bubble on the right.
    dwightDrawAsciiFace(canvas, 0);

    const int bx = dwightAsciiFaceWidth() + 6, by = 8;
    const int bw = DISPLAY_W - bx - 4, bh = 70;
    drawSpeechBubble(canvas, bx, by, bw, bh, gTheme.accent);
    drawWrappedText(canvas,
                    "Set your focus. Discipline equals freedom. We start now.",
                    bx + 8, by + 9, bw - 16, gTheme.foreground, 11);

    // Big minute selector below the bubble.
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", _minutes);
    canvas->setTextSize(4);
    int w = (int)strlen(buf) * 24;
    int x = bx + (bw - w - 30) / 2;
    if (x < bx) x = bx;
    int y = by + bh + 6;
    canvas->setTextColor(gTheme.foreground);
    canvas->setCursor(x, y);
    canvas->print(buf);
    canvas->setTextSize(1);
    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(x + w + 4, y + 18);
    canvas->print("min");

    // Footer hints.
    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(2, DISPLAY_H - 11);
    canvas->print(";/. +-1  ,// +-5  Enter=start");

    canvas->pushSprite(0, 0);
}

void FocusApp::drawRunning() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    uint32_t now = millis();
    uint32_t remaining = ((int32_t)(_endMs - now) > 0) ? (_endMs - now) : 0;

    bool abandoning = (_phase == Phase::ConfirmAbandon);
    bool lastMinute = (remaining < 60000);

    // Top label.
    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(2, 2);
    canvas->print("FOCUS");

    // Progress bar across the top.
    const int barX = 2, barY = 14, barW = DISPLAY_W - 4, barH = 5;
    canvas->drawRect(barX, barY, barW, barH, gTheme.muted);
    if (_durationMs > 0) {
        uint32_t elapsed = _durationMs - remaining;
        int fill = (int)((uint64_t)(barW - 2) * elapsed / _durationMs);
        if (fill > 0) canvas->fillRect(barX + 1, barY + 1, fill, barH - 2, gTheme.accent);
    }

    // Big countdown — color signals urgency / abandon.
    uint16_t timeColor = gTheme.foreground;
    if (abandoning)       timeColor = gTheme.error;
    else if (lastMinute)  timeColor = gTheme.warning;
    drawBigTime(remaining, timeColor);

    // Threat / abandon message in a bubble at the bottom.
    const int by = 96, bh = DISPLAY_H - by - 2;
    drawSpeechBubble(canvas, 2, by, DISPLAY_W - 4, bh, abandoning ? gTheme.error : gTheme.warning);
    const char* msg = abandoning
        ? "Surrender? Press the back key AGAIN to quit. Anything else: back to work."
        : kThreats[_threatIdx];
    drawWrappedText(canvas, msg, 10, by + 8, DISPLAY_W - 20,
                    abandoning ? gTheme.error : gTheme.foreground, 10);

    canvas->pushSprite(0, 0);
}

void FocusApp::drawDone() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    dwightDrawAsciiFace(canvas, 0);

    const int bx = dwightAsciiFaceWidth() + 6, by = 8;
    const int bw = DISPLAY_W - bx - 4, bh = 64;
    drawSpeechBubble(canvas, bx, by, bw, bh, gTheme.success);
    drawWrappedText(canvas,
                    "Focus complete. Acceptable work, drone. You may have one beet.",
                    bx + 8, by + 9, bw - 16, gTheme.foreground, 11);

    // Big "DONE".
    canvas->setTextSize(4);
    const char* d = "DONE";
    int w = (int)strlen(d) * 24;
    int x = bx + (bw - w) / 2;
    if (x < bx) x = bx;
    canvas->setTextColor(gTheme.success);
    canvas->setCursor(x, by + bh + 8);
    canvas->print(d);
    canvas->setTextSize(1);

    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(bx, DISPLAY_H - 11);
    canvas->print("Enter = new timer");

    canvas->pushSprite(0, 0);
}

void FocusApp::beep(uint16_t freq, uint32_t durMs) {
    if (!_mgr->config()->soundsEnabled()) return;
    uint8_t vol = _mgr->config()->system().buzzerVolume;
    if (vol == 0) return;
    auto& spk = M5Cardputer.Speaker;
    if (!_spkReady) {
        spk.begin();
        _spkReady = true;
    }
    spk.setVolume(vol);
    spk.tone(freq, durMs);
}

void FocusApp::loadConfig(JsonObjectConst node) {
    if (node.isNull()) return;
    if (node["default_minutes"].is<int>()) {
        int m = node["default_minutes"];
        if (m < kMinMinutes) m = kMinMinutes;
        if (m > kMaxMinutes) m = kMaxMinutes;
        _defaultMin = m;
        _minutes    = m;
    }
}

void FocusApp::saveConfig(JsonObject node) {
    node["default_minutes"] = _defaultMin;
}
