#pragma once

#include <Arduino.h>

#include "core/App.h"

class AppManager;

// Back To Work: a focus timer. Pick a duration, then a big full-screen
// countdown ticks down while Dwight throws escalating threats to keep you on
// task. Try to bail out before time is up (the global back key) and Dwight
// intercepts with a threat and demands you confirm the surrender. Reach zero
// and he congratulates you — grudgingly.
//
// Controls:
//   Setup   : `;`/`.` -/+ 1 min, `,`/`/` -/+ 5 min, Enter = start.
//   Running : back key = ask to abandon (Dwight threatens).
//   Done    : Enter = set a new timer.
//
// Config lives under apps.dwight_focus in the SD config:
//   default_minutes (1..180).
class FocusApp : public App {
public:
    const char* id()       const override { return "dwight_focus"; }
    const char* name()     const override { return "Back To Work"; }
    const char* subtitle() const override { return "Timer"; }

    void setup(AppManager& mgr) override;
    void onEnter() override;
    void loop() override;
    bool onBackRequested() override;

    void loadConfig(JsonObjectConst node) override;
    void saveConfig(JsonObject node) override;

private:
    enum class Phase { Setup, Running, ConfirmAbandon, Done };

    void draw();
    void drawSetup();
    void drawRunning();
    void drawDone();
    void drawBigTime(uint32_t remainingMs, uint16_t color);
    void startTimer();
    void beep(uint16_t freq, uint32_t durMs);

    AppManager* _mgr = nullptr;

    int      _minutes       = 25;    // selected focus duration
    int      _defaultMin    = 25;    // persisted default
    Phase    _phase         = Phase::Setup;

    uint32_t _endMs         = 0;     // millis() at which the focus ends
    uint32_t _durationMs    = 0;     // total focus duration for progress
    uint32_t _lastDrawMs    = 0;

    // Escalating threats while running: index advances over time.
    int      _threatIdx     = 0;
    uint32_t _nextThreatMs  = 0;

    bool     _spkReady      = false;
};
