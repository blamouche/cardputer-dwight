#pragma once

#include "core/App.h"

class AppManager;

// Turns the Cardputer into a live BLE keyboard for the paired host: every key
// you press is forwarded over Bluetooth. Dwight comments on each keystroke in
// an oppressive tone. The bubble asks to pair when disconnected. The ESC /
// backtick key returns to the launcher (handled globally by the AppManager).
class KeyboardApp : public App {
public:
    const char* id()       const override { return "keyboard"; }
    const char* name()     const override { return "Ghost Writer"; }
    const char* subtitle() const override { return "BLE"; }

    void setup(AppManager& mgr) override;
    void onEnter() override;
    void loop() override;

    void loadConfig(JsonObjectConst node) override;
    void saveConfig(JsonObject node) override;

private:
    void draw();

    AppManager* _mgr = nullptr;

    String  _deviceName   = "Dwight Assistant";
    String  _manufacturer = "M5Stack";
    uint8_t _batteryLevel = 100;

    // BLE HID sends physical key positions, not characters; the host applies
    // its own layout. When the host is AZERTY (French), translate so what you
    // type on the Cardputer's QWERTY shows the same character. Config:
    // apps.keyboard.layout = "azerty" (default) | "us".
    bool _azerty = true;

    int      _lineIdx    = 0;   // which oppressive line to show
    uint32_t _lastKeyMs  = 0;   // when the last key was forwarded
    uint32_t _lastDrawMs = 0;
};
