#pragma once

#include <Arduino.h>

#include "core/App.h"

class AppManager;
class DwightUnlockApp;

// On-device configuration editor. It opens on a small section menu:
//   - Computers  : add / edit / remove the name+password entries used by the
//                  Open Sesame app, written back to the SD config immediately.
//   - Appearance : pick the UI color theme (green/blue/purple/yellow/light),
//                  applied live and persisted to the SD config.
//   - Sound      : toggle the beeps on/off and set the buzzer volume, with an
//                  audible preview, persisted to the SD config.
//   - Power      : toggle eco mode (LED off, no sound, dimmed screen that turns
//                  off after a short idle delay), persisted to the SD config.
//
// Section keys:   `;` / `.` move   Enter = open   `` ` `` (back) = leave
// Computers keys: `;` / `.` move   Enter = edit   n = new   d = delete
// Edit keys:      type to fill the field, Backspace deletes, Enter confirms,
//                 the `` ` `` (back) key cancels the current edit.
// Appearance keys:`;` / `.` move   Enter = apply  `` ` `` (back) = sections
// Sound keys:     `;` / `.` move row   `,` / `/` change value   Enter = toggle
// Power keys:     `,` / `/` or Enter toggle eco mode   `` ` `` (back) = sections
class ConfigApp : public App {
public:
    const char* id()       const override { return "config"; }
    const char* name()     const override { return "Config"; }
    const char* subtitle() const override { return "Settings"; }

    void setup(AppManager& mgr) override;
    void onEnter() override;
    void loop() override;
    bool onBackRequested() override;

    // Wires the editor to the unlock app's shared computer list (set in main).
    void attachUnlock(DwightUnlockApp* unlock) { _unlock = unlock; }

private:
    enum class Mode { Sections, Computers, EditName, EditPass, Appearance, Sound, Power };

    void draw();
    void drawSections();
    void drawComputers();
    void drawEdit();
    void drawAppearance();
    void drawSound();
    void drawPower();

    void persist();
    void handleSectionKeys();
    void handleComputerKeys();
    void handleEditKeys();
    void handleAppearanceKeys();
    void handleSoundKeys();
    void handlePowerKeys();
    void beginNew();
    void beginEdit(int idx);
    void deleteSelected();
    void applySelectedTheme();
    void previewBeep();

    AppManager*      _mgr    = nullptr;
    DwightUnlockApp* _unlock = nullptr;

    Mode    _mode        = Mode::Sections;
    int     _sectionSel  = 0;
    int     _selection   = 0;   // selected computer in the Computers list
    int     _appearanceSel = 0; // selected theme in the Appearance list
    int     _soundSel    = 0;   // selected row in the Sound section (0=beeps,1=volume)
    int     _editIdx     = -1;  // index being edited; -1 = creating a new entry
    String  _nameBuf;
    String  _passBuf;
    bool    _dirty       = true;
    uint32_t _lastDrawMs = 0;
    bool    _spkReady    = false; // lazy speaker init for the volume preview
};
