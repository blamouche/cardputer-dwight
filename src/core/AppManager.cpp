#include "AppManager.h"

#include <M5Cardputer.h>
#include <string.h>

#include "App.h"
#include "ConfigManager.h"
#include "HardwareConfig.h"

void AppManager::begin(M5Canvas* canvas, ConfigManager* config) {
    _canvas = canvas;
    _config = config;
}

void AppManager::registerApp(App* app) {
    _apps.push_back(app);
    app->setup(*this);
}

void AppManager::start() {
    if (_apps.empty()) return;
    switchTo(_apps[0]);
}

void AppManager::switchTo(App* app) {
    if (!app || app == _current) return;
    if (_current) _current->onExit();
    _current = app;
    _current->onEnter();
}

void AppManager::switchToId(const char* id) {
    for (auto* a : _apps) {
        if (strcmp(a->id(), id) == 0) {
            switchTo(a);
            return;
        }
    }
}

void AppManager::backToMenu() {
    if (_apps.empty() || _current == _apps[0]) return;
    if (_current && !_current->onBackRequested()) return;
    _lastApp = _current;  // remembered so the menu can preselect this app
    switchTo(_apps[0]);
}

void AppManager::loopFrame() {
    M5Cardputer.update();

    // Sample once: isChange() is one-shot in M5Cardputer (consumes the flag).
    _inputChanged = M5Cardputer.Keyboard.isChange();
    _inputPressed = M5Cardputer.Keyboard.isPressed();

    // Global back-to-menu hotkey, intercepted before the active app sees it.
    if (_inputChanged && _inputPressed) {
        auto state = M5Cardputer.Keyboard.keysState();
        for (char c : state.word) {
            if (c == KEY_BACK_TO_MENU) {
                backToMenu();
                return;
            }
        }
    }

    if (_current) _current->loop();
}
