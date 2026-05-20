#pragma once

#include <Arduino.h>
#include <M5GFX.h>
#include <vector>

class App;
class ConfigManager;

// Drives lifecycle of every App and routes the global "back to menu" key.
// The first app registered is the "home" app (typically the launcher menu).
class AppManager {
public:
    void begin(M5Canvas* canvas, ConfigManager* config);

    // Register an app. The AppManager keeps a non-owning pointer; storage
    // is up to the caller (statics in main.cpp).
    void registerApp(App* app);

    const std::vector<App*>& apps() const { return _apps; }

    void start();        // enter the home app
    void loopFrame();    // call once per main loop iteration

    void switchTo(App* app);
    void switchToId(const char* id);
    void backToMenu();

    App* current() const { return _current; }
    App* home() const    { return _apps.empty() ? nullptr : _apps[0]; }

    // Last non-home app the user was in before returning to the menu. The menu
    // uses it to preselect the row of the app that was just active.
    App* lastApp() const { return _lastApp; }

    M5Canvas*      canvas() const { return _canvas; }
    ConfigManager* config() const { return _config; }

    // Per-frame snapshot of the keyboard. M5Cardputer's isChange() mutates
    // internal state on read (one-shot), so apps MUST consult these
    // accessors rather than calling the keyboard API directly — otherwise
    // the first reader consumes the change flag and later readers see false.
    bool inputChanged() const { return _inputChanged; }
    bool inputPressed() const { return _inputPressed; }

private:
    std::vector<App*> _apps;
    App*           _current = nullptr;
    App*           _lastApp = nullptr;
    M5Canvas*      _canvas  = nullptr;
    ConfigManager* _config  = nullptr;
    bool           _inputChanged = false;
    bool           _inputPressed = false;
};
