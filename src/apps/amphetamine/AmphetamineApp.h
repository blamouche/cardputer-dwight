#pragma once

#include "core/App.h"

class AppManager;

// Keeps the paired host awake by periodically sending a harmless arrow-key tap
// (alternating up/down so nothing actually scrolls away). Shows Dwight with
// twitchy red eyes; asks to pair over Bluetooth when not connected.
class AmphetamineApp : public App {
public:
    const char* id()       const override { return "amphetamine"; }
    const char* name()     const override { return "Bears Don't Sleep"; }
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

    uint32_t _intervalMs = 10000;  // time between keep-awake taps
    uint32_t _lastTapMs  = 0;
    bool     _tapUp      = true;    // alternate up / down
    uint32_t _tapCount   = 0;
    uint32_t _lastDrawMs = 0;
};
