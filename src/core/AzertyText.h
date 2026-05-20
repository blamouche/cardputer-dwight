#pragma once

#include <Arduino.h>

// macOS French (AZERTY) text output over BLE HID.
//
// BLE HID transmits US key *positions*, not characters; the host applies its
// own layout. To make a logical character appear on a macOS French AZERTY host
// we send the US-ASCII byte whose US position+shift equals that character's
// position+shift on the mac French layout. Only characters reachable WITHOUT
// Option/AltGr are supported (so @ # & é è à ç ù §, digits and common
// punctuation work; { } [ ] | \ € ~ ^ and accented capitals do not — they need
// Option or dead keys and are passed through / skipped as best effort).
namespace azerty {

// US-ASCII byte to send so the printable ASCII char `c` shows up correctly on a
// macOS French host. Characters with no remap pass through unchanged.
uint8_t mapAscii(uint8_t c);

// Decodes UTF-8 `text`, translates each character for the host layout (when
// `azerty` is true), and types it over BLE with safe pacing, then clears the
// HID report. No-op if BLE is not connected. When `azerty` is false the ASCII
// bytes are sent verbatim and non-ASCII (multi-byte) characters are skipped.
void typeText(const String& text, bool azerty);

}  // namespace azerty
