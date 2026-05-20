#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

class App;

enum class ConfigStatus {
    NotLoaded,
    Loaded,
    CreatedDefault,
    NoSdCard,
    OpenError,
    JsonError,
    CreateError,
};

struct SystemConfig {
    String userName;
    String language;        // "en"
    uint8_t brightness;     // 0..255
    bool buzzerEnabled;     // master on/off for beeps
    uint8_t buzzerVolume;   // 0..255
    uint32_t sleepTimeoutS; // 0 = never
    String primaryColor;    // "#RRGGBB"
    String colorTheme;      // UI theme id: "green","blue","purple","yellow","light"
    bool powerSaveEnabled;  // master eco mode: LED off, no sound, dimmed + screen-off
};

struct WifiConfig {
    bool enabled;
    String ssid;
    String password;
};

struct TimeConfig {
    String timezone;        // POSIX TZ or IANA-like, see README
    String ntpServer;
    bool format24h;
    String dateFormat;      // "dd/mm/yyyy", "mm/dd/yyyy", "yyyy-mm-dd"
};

struct MenuConfig {
    std::vector<String> enabledApps;  // ordered list of app ids
};

class ConfigManager {
public:
    bool begin();

    // Loads from SD, creates the file with defaults if missing.
    ConfigStatus load(const std::vector<App*>& apps);

    // Re-serialize current state + each app's saveConfig() into the file.
    bool save(const std::vector<App*>& apps);

    const SystemConfig& system() const { return _system; }
    const WifiConfig&   wifi()   const { return _wifi; }
    const TimeConfig&   time()   const { return _time; }
    const MenuConfig&   menu()   const { return _menu; }

    // Effective hardware settings once power-save mode is taken into account.
    // Use these instead of reading the raw fields so eco mode is honored
    // consistently everywhere (sound, screen brightness, LED).
    bool    soundsEnabled() const { return _system.buzzerEnabled && !_system.powerSaveEnabled; }
    bool    ledEnabled()    const { return !_system.powerSaveEnabled; }
    uint8_t effectiveBrightness() const;

    SystemConfig& mutSystem() { return _system; }
    WifiConfig&   mutWifi()   { return _wifi; }
    TimeConfig&   mutTime()   { return _time; }
    MenuConfig&   mutMenu()   { return _menu; }

    ConfigStatus status() const { return _status; }
    const String& statusMessage() const { return _statusMessage; }

private:
    void applyDefaults(const std::vector<App*>& apps);
    bool writeDefault(const std::vector<App*>& apps);

    SystemConfig _system{};
    WifiConfig   _wifi{};
    TimeConfig   _time{};
    MenuConfig   _menu{};
    ConfigStatus _status = ConfigStatus::NotLoaded;
    String       _statusMessage;
    bool         _sdReady = false;
};
