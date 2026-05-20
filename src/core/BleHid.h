#pragma once

#include <stdint.h>

// A single shared BLE HID keyboard. NimBLE only supports one peripheral, so
// every app that needs to act as a Bluetooth keyboard MUST go through here
// rather than instantiating its own BleKeyboard (two instances would re-init
// NimBLE and break the pairing).
namespace BleHid {

// Brings the BLE keyboard up once. No-op if already started (the first
// caller's name / manufacturer / battery level win for the session).
void begin(const char* deviceName, const char* manufacturer, uint8_t batteryLevel);

bool isActive();
bool isConnected();

// True only once the BLE link has settled after a connection. BleKeyboard flips
// "connected" on the GAP connect event, but the host (macOS/Windows) still needs
// a beat to subscribe to the HID input-report notifications and finish parameter
// negotiation; reports sent before that are dropped or mis-parsed — which is why
// the first keystrokes used to come out garbled. Typing helpers below are gated
// on this, so they no-op until the link is ready.
bool isReady();

// Connected but still inside the post-connect warm-up window. Apps can show a
// "stabilizing" hint so the user waits a beat before typing.
bool isWarmingUp();

// Press, brief hold, then release + defensive releaseAll() to clear the HID
// report. No-op if not connected or keycode == 0.
void tap(uint8_t keycode);

// Type a printable ASCII character (handles shift via the library's map).
void type(uint8_t c);

// Sends an all-zero HID report, clearing any keys/modifiers the host may still
// think are held. Use after bulk-typing to guarantee no key gets "stuck" if a
// release report was dropped under BLE congestion.
void releaseAll();

// Convenience taps. These let apps avoid pulling BleKeyboard's KEY_* macros,
// which clash with M5Cardputer's keyboard defines.
void tapUp();
void tapDown();
void tapEnter();
void tapBackspace();
void tapTab();

}  // namespace BleHid
