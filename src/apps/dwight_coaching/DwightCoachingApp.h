#pragma once

#include <Arduino.h>
#include <vector>

#include "core/App.h"

class AppManager;

// Schrute Wisdom: shows a random Dwight-style motivational line, portrait on
// the left, quote in a speech bubble on the right. Press any key (Enter, space,
// an arrow...) for the next random quote — never the same one twice in a row.
// The back key leaves the app.
//
// Quotes default to a built-in list and can be overridden from the SD config
// under apps.dwight_coaching.quotes (array of strings).
class DwightCoachingApp : public App {
public:
    const char* id()       const override { return "dwight_coaching"; }
    const char* name()     const override { return "Schrute Wisdom"; }
    const char* subtitle() const override { return "Motivation"; }

    void setup(AppManager& mgr) override;
    void onEnter() override;
    void loop() override;

    void loadConfig(JsonObjectConst node) override;
    void saveConfig(JsonObject node) override;

private:
    void draw();
    void pickRandomQuote();

    AppManager* _mgr = nullptr;

    std::vector<String> _quotes;
    int                 _index      = 0;
    bool                _seeded     = false;
    bool                _dirty      = true;
    uint32_t            _lastDrawMs = 0;
};
