#pragma once

#include <Arduino.h>
#include <vector>

#include "core/App.h"

class AppManager;

// Dwight productivity answers: a library of ready-made phrases ("answers") you
// can type onto the paired host over BLE. Pick one in the scrolling list with
// `;` / `.` and press Enter to type it (no Return is sent, so you can review
// before submitting). Phrases can be added (`n`), edited (`e`) and deleted
// (`d`) on-device. The list lives under apps.dwight_answers.phrases in the SD
// config.
class DwightAnswersApp : public App {
public:
    const char* id()       const override { return "dwight_answers"; }
    const char* name()     const override { return "FALSE."; }
    const char* subtitle() const override { return "BLE"; }

    void setup(AppManager& mgr) override;
    void onEnter() override;
    void loop() override;
    bool onBackRequested() override;

    void loadConfig(JsonObjectConst node) override;
    void saveConfig(JsonObject node) override;

private:
    enum class Mode { List, Edit };

    void draw();
    void drawList();
    void drawEdit();

    void handleListKeys();
    void handleEditKeys();

    void beginNew();
    void beginEdit(int idx);
    void deleteSelected();
    void typeSelected();
    void persist();

    AppManager* _mgr = nullptr;

    String  _deviceName   = "Dwight Assistant";
    String  _manufacturer = "M5Stack";
    uint8_t _batteryLevel = 100;

    // BLE HID sends physical key positions; the host applies its own layout.
    // When the host is AZERTY (French), translate the phrase characters so they
    // land correctly. Config: apps.dwight_answers.layout = "azerty" | "us".
    bool _azerty = true;

    // A handful of useful defaults so the very first run (no SD entry yet) is
    // already productive. Overwritten by the SD config when a phrases array is
    // present.
    std::vector<String> _phrases = {
        "Sounds good, I'll take care of it.",
        "Let me circle back on that.",
        "Per my last email...",
        "Adding this to the agenda.",
        "Identity theft is not a joke.",
    };

    Mode     _mode       = Mode::List;
    int      _selection  = 0;
    int      _editIdx    = -1;   // -1 = creating a new phrase
    String   _editBuf;
    bool     _justTyped  = false;
    uint32_t _lastTypeMs = 0;
    uint32_t _lastDrawMs = 0;
    bool     _dirty      = true;
};
