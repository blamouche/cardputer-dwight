#include "AzertyText.h"

#include "BleHid.h"

namespace {

// Latin-1 French accents that sit on a *single* key (no dead key) on macOS
// French AZERTY: they live on the unshifted number row, plus ù and the §/°
// keys. Returns the US-ASCII byte to send, or 0 if the codepoint can't be typed
// without Option / a dead key (e.g. accented capitals, â ê î ô û, ë ï ü).
uint8_t mapCodepoint(uint32_t cp) {
    switch (cp) {
        case 0x00E9: return '2';   // é  (unshifted "2" key)
        case 0x00E8: return '7';   // è  (unshifted "7" key)
        case 0x00E0: return '0';   // à  (unshifted "0" key)
        case 0x00E7: return '9';   // ç  (unshifted "9" key)
        case 0x00F9: return '\'';  // ù  (unshifted US "'" key)
        case 0x00A7: return '6';   // §  (unshifted "6" key)
        case 0x00B0: return '_';   // °  (shifted US "-" key)
        default:     return 0;
    }
}

}  // namespace

namespace azerty {

uint8_t mapAscii(uint8_t c) {
    switch (c) {
        // Letters whose position differs between US QWERTY and FR AZERTY.
        case 'a': return 'q';  case 'A': return 'Q';
        case 'q': return 'a';  case 'Q': return 'A';
        case 'z': return 'w';  case 'Z': return 'W';
        case 'w': return 'z';  case 'W': return 'Z';
        case 'm': return ';';  case 'M': return ':';

        // Bottom-row punctuation cluster (US m , . / keys on mac French).
        case ',': return 'm';   // , is unshifted on the US "m" key
        case '?': return 'M';   // ? is shifted   on the US "m" key
        case ';': return ',';   // ; is unshifted on the US "," key
        case '.': return '<';   // . is shifted   on the US "," key
        case ':': return '.';   // : is unshifted on the US "." key
        case '/': return '>';   // / is shifted   on the US "." key
        case '=': return '/';   // = is unshifted on the US "/" key
        case '+': return '?';   // + is shifted   on the US "/" key

        // Number row — unshifted symbols/letters (the part that was broken).
        case '&':  return '1';
        case '"':  return '3';
        case '\'': return '4';
        case '(':  return '5';
        case ')':  return '-';   // ) is unshifted on the US "-" key
        case '!':  return '8';   // mac-specific: ! is unshifted on the "8" key
        case '-':  return '=';   // - is unshifted on the US "=" key
        case '_':  return '+';   // _ is shifted   on the US "=" key
        case '@':  return '`';   // @ is unshifted on the US "`" key
        case '#':  return '~';   // # is shifted   on the US "`" key
        case '%':  return '"';   // % is shifted   on the US "'" key (ù key)

        // Digits require their US shifted twins (digits are shifted on AZERTY).
        case '1': return '!';
        case '2': return '@';
        case '3': return '#';
        case '4': return '$';
        case '5': return '%';
        case '6': return '^';
        case '7': return '&';
        case '8': return '*';
        case '9': return '(';
        case '0': return ')';

        default:  return c;
    }
}

void typeText(const String& text, bool azerty) {
    const char* s = text.c_str();
    size_t len = text.length();

    for (size_t i = 0; i < len; ) {
        uint8_t b = (uint8_t)s[i];
        uint8_t out = 0;

        if (b < 0x80) {
            // Plain ASCII.
            out = azerty ? mapAscii(b) : b;
            i += 1;
        } else if ((b & 0xE0) == 0xC0 && i + 1 < len) {
            // 2-byte UTF-8 (covers the Latin-1 French accents).
            uint32_t cp = ((uint32_t)(b & 0x1F) << 6) | ((uint8_t)s[i + 1] & 0x3F);
            out = azerty ? mapCodepoint(cp) : 0;
            i += 2;
        } else if ((b & 0xF0) == 0xE0) {
            i += 3;  // 3-byte sequence — unsupported, skip
        } else if ((b & 0xF8) == 0xF0) {
            i += 4;  // 4-byte sequence — unsupported, skip
        } else {
            i += 1;  // stray continuation byte — skip
        }

        if (out == 0) continue;  // unmapped / unsupported character
        BleHid::type(out);
        // Space characters out by more than one BLE connection interval
        // (~15 ms on macOS) so the press/release reports don't collide; a
        // dropped release would otherwise stick a key and fire shortcuts.
        delay(28);
    }

    BleHid::releaseAll();  // guarantee nothing stays held after the text
}

}  // namespace azerty
