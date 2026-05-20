#pragma once

// SD pins on Cardputer / Cardputer ADV
#define SD_SCK  40
#define SD_MISO 39
#define SD_MOSI 14
#define SD_CS   12

// On-board WS2812 RGB LED (SK6812, GRB)
#define LED_PIN        21
#define LED_NUM        1
#define LED_BRIGHTNESS 255

// Display
#define DISPLAY_W 240
#define DISPLAY_H 135

// Config file at SD root
#define CONFIG_PATH "/dwight-assistant-config.json"

// Per-app data directory on SD (notes, todos, IR codes, ...)
#define DATA_DIR "/dwight-assistant"

// Global return-to-menu key (Cardputer keyboard char)
#define KEY_BACK_TO_MENU '`'

// Power-save mode: capped screen brightness (0..255) and inactivity delay
// before the screen turns off. Both apply only while power save is enabled.
#define POWER_SAVE_BRIGHTNESS    40
#define POWER_SAVE_SCREEN_OFF_MS 15000
