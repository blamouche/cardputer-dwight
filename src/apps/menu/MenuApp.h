#pragma once

#include "core/App.h"

class AppManager;

// Home launcher: lists registered apps and starts the selected one.
class MenuApp : public App {
public:
    const char* id()   const override { return "menu"; }
    const char* name() const override { return "Menu"; }

    void setup(AppManager& mgr) override;
    void onEnter() override;
    void loop() override;

private:
    void draw();
    void launchSelected();

    AppManager* _mgr = nullptr;
    int  _selection = 0;
    bool _dirty = true;
    uint32_t _lastDrawMs = 0;
};
