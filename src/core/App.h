#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class AppManager;

// Lifecycle:
//   onEnter() -> loop() ... -> onExit()
// The AppManager drives transitions; apps just override what they need.
class App {
public:
    virtual ~App() = default;

    // Stable identifier used in the SD config (e.g. "presentation_remote").
    virtual const char* id() const = 0;

    // Human-readable name shown in the menu.
    virtual const char* name() const = 0;

    // Optional one-line subtitle shown under the name in the menu.
    virtual const char* subtitle() const { return ""; }

    // Called once before the first onEnter(). Use for heavy init.
    virtual void setup(AppManager& mgr) { (void)mgr; }

    // Called when the app becomes active.
    virtual void onEnter() {}

    // Called when the app is leaving (back to menu, sleep, etc.).
    virtual void onExit() {}

    // Called every iteration of the main loop while this app is active.
    virtual void loop() {}

    // Called when the user presses the global "back to menu" key while
    // this app is active. Default: leave the app. Override to intercept
    // (e.g. show a "save before quitting?" prompt) and return false to
    // veto the exit.
    virtual bool onBackRequested() { return true; }

    // Called by ConfigManager with the JsonObject reserved for this app
    // under "apps.<id>". Apps should read defaults from it on load and
    // write their current state on save. Both default to no-op.
    virtual void loadConfig(JsonObjectConst /*node*/) {}
    virtual void saveConfig(JsonObject /*node*/) {}
};
