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
