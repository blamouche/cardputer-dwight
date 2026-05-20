#pragma once

#include <Arduino.h>
#include <vector>

#include "core/App.h"

class AppManager;

// One configured machine: a label and the session password to type for it.
struct UnlockComputer {
    String name;
    String password;
};

// Open Sesame: types the session password of a paired computer over BLE so
// the host's lock screen unlocks itself. Several computers can be configured
// (name + password); pick one with `;` / `.` and press Enter to type its
// password followed by Return. The list lives under
// apps.dwight_unlock.computers in the SD config and can also be edited
// on-device through the Config app, which shares this same list.
class DwightUnlockApp : public App {
public:
    const char* id()       const override { return "dwight_unlock"; }
    const char* name()     const override { return "Open Sesame"; }
    const char* subtitle() const override { return "BLE"; }

    void setup(AppManager& mgr) override;
    void onEnter() override;
    void loop() override;

    void loadConfig(JsonObjectConst node) override;
    void saveConfig(JsonObject node) override;

    // Shared with the Config app, which edits the very same list.
    std::vector<UnlockComputer>&       computers()       { return _computers; }
    const std::vector<UnlockComputer>& computers() const { return _computers; }

private:
    void draw();
    void sendPassword(const UnlockComputer& pc);

    AppManager* _mgr = nullptr;

    String  _deviceName   = "Dwight Assistant";
    String  _manufacturer = "M5Stack";
    uint8_t _batteryLevel = 100;

    // BLE HID sends physical key positions; the host applies its own layout.
    // When the host is AZERTY (French), translate the password characters so
    // they land correctly. Config: apps.dwight_unlock.layout = "azerty" | "us".
    bool _azerty = true;

    std::vector<UnlockComputer> _computers;
    int      _selection  = 0;
    bool     _justTyped  = false;
    uint32_t _lastTypeMs = 0;
    uint32_t _lastDrawMs = 0;
};
