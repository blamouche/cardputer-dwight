#include "BleHid.h"

#include <Arduino.h>
#include <BleKeyboard.h>

namespace {
BleKeyboard* g_ble = nullptr;

// Timestamp of the last disconnected->connected transition, and whether we have
// sent the post-warm-up "clean" report yet.
uint32_t g_connectedAtMs = 0;
bool     g_primed        = false;

// Grace period after the GAP connect event before we trust the HID link. The
// host needs this long to subscribe to the input-report notifications and
// finish connection-parameter negotiation; anything sent earlier is lost or
// mis-parsed. ~1.5 s is comfortable on macOS, including first-ever pairings.
constexpr uint32_t kWarmupMs = 1500;

// Edge-detects the connection so we can timestamp it. Safe to call many times
// per frame (apps poll isConnected() every draw); only the transition matters.
bool refresh() {
    static bool wasConnected = false;
    bool now = g_ble && g_ble->isConnected();
    if (now && !wasConnected) { g_connectedAtMs = millis(); g_primed = false; }
    if (!now)                 { g_connectedAtMs = 0;        g_primed = false; }
    wasConnected = now;
    return now;
}

// First report after warm-up is a clean all-zero one: if the host happens to
// drop this very first notification, no real keystroke is lost.
void primeOnce() {
    if (!g_primed) {
        g_ble->releaseAll();
        g_primed = true;
    }
}
}  // namespace

namespace BleHid {

void begin(const char* deviceName, const char* manufacturer, uint8_t batteryLevel) {
    if (g_ble) return;
    g_ble = new BleKeyboard(deviceName, manufacturer, batteryLevel);
    g_ble->begin();
}

bool isActive() { return g_ble != nullptr; }

bool isConnected() { return refresh(); }

bool isReady() {
    if (!refresh() || g_connectedAtMs == 0) return false;
    return (millis() - g_connectedAtMs) >= kWarmupMs;
}

bool isWarmingUp() { return refresh() && !isReady(); }

void tap(uint8_t keycode) {
    if (!isReady() || keycode == 0) return;
    primeOnce();
    g_ble->press(keycode);
    delay(10);
    g_ble->release(keycode);
    g_ble->releaseAll();
}

void type(uint8_t c) {
    if (!isReady() || c == 0) return;
    primeOnce();
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
