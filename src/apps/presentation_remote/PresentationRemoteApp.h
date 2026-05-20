#pragma once

#include "core/App.h"

class AppManager;

// BLE HID keyboard for slide-deck navigation (Google Slides, Keynote, etc.).
// Originally https://github.com/blamouche/cardputer-presentation-remote ,
// adapted as an App for Dwight Assistant.
class PresentationRemoteApp : public App {
public:
    const char* id()       const override { return "presentation_remote"; }
    const char* name()     const override { return "Slide Sensei"; }
    const char* subtitle() const override { return "BLE"; }

    void setup(AppManager& mgr) override;
    void onEnter() override;
    void onExit() override;
    void loop() override;

    void loadConfig(JsonObjectConst node) override;
    void saveConfig(JsonObject node) override;

private:
    enum Direction { DIR_NONE, DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN };

    struct KeyMapping {
        uint8_t left;
        uint8_t right;
        uint8_t up;
        uint8_t down;
    };

    void draw();
    void sendKey(uint8_t code, Direction dir);
    void ensureBle();
    static uint8_t parseKey(const char* name);
    static const char* keyName(uint8_t code);
    static const char* dirLabel(Direction d);

    AppManager*   _mgr = nullptr;

    String     _deviceName   = "Dwight presentation control";
    String     _manufacturer = "M5Stack";
    uint8_t    _batteryLevel = 100;
    KeyMapping _mapping{};

    Direction _currentDir = DIR_NONE;
    uint32_t  _pressTimeMs = 0;
    uint32_t  _lastDrawMs = 0;
    uint32_t  _startMs = 0;     // presentation timer origin (set on onEnter)
};
