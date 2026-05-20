#include "BleHid.h"

#include <Arduino.h>
#include <BleKeyboard.h>

namespace {
BleKeyboard* g_ble = nullptr;
}

namespace BleHid {

void begin(const char* deviceName, const char* manufacturer, uint8_t batteryLevel) {
    if (g_ble) return;
    g_ble = new BleKeyboard(deviceName, manufacturer, batteryLevel);
    g_ble->begin();
}

bool isActive() { return g_ble != nullptr; }

bool isConnected() { return g_ble && g_ble->isConnected(); }

void tap(uint8_t keycode) {
    if (!g_ble || !g_ble->isConnected() || keycode == 0) return;
    g_ble->press(keycode);
    delay(10);
    g_ble->release(keycode);
    g_ble->releaseAll();
}

void type(uint8_t c) {
    if (!g_ble || !g_ble->isConnected() || c == 0) return;
    g_ble->write(c);
}

void releaseAll() {
    if (!g_ble || !g_ble->isConnected()) return;
    g_ble->releaseAll();
}

void tapUp()        { tap(KEY_UP_ARROW); }
void tapDown()      { tap(KEY_DOWN_ARROW); }
void tapEnter()     { tap(KEY_RETURN); }
void tapBackspace() { tap(KEY_BACKSPACE); }
void tapTab()       { tap(KEY_TAB); }

}  // namespace BleHid
