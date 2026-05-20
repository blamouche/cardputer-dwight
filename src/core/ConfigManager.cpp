#include "ConfigManager.h"

#include <SD.h>
#include <SPI.h>

#include "App.h"
#include "HardwareConfig.h"

bool ConfigManager::begin() {
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    _sdReady = SD.begin(SD_CS, SPI);
    return _sdReady;
}

void ConfigManager::applyDefaults(const std::vector<App*>& apps) {
    _system.userName      = "";
    _system.language      = "en";
    _system.brightness    = 255;
    _system.buzzerEnabled = true;
    _system.buzzerVolume  = 128;
    _system.sleepTimeoutS = 0;
    _system.primaryColor  = "#D8A25C";
    _system.colorTheme    = "green";
    _system.powerSaveEnabled = false;

    _wifi.enabled  = false;
    _wifi.ssid     = "";
    _wifi.password = "";

    _time.timezone   = "Europe/Paris";
    _time.ntpServer  = "pool.ntp.org";
    _time.format24h  = true;
    _time.dateFormat = "dd/mm/yyyy";

    _menu.enabledApps.clear();
    for (auto* a : apps) _menu.enabledApps.push_back(a->id());
}

bool ConfigManager::writeDefault(const std::vector<App*>& apps) {
    JsonDocument doc;

    JsonObject sys = doc["system"].to<JsonObject>();
    sys["user_name"]       = _system.userName;
    sys["language"]        = _system.language;
    sys["brightness"]      = _system.brightness;
    sys["buzzer_enabled"]  = _system.buzzerEnabled;
    sys["buzzer_volume"]   = _system.buzzerVolume;
    sys["sleep_timeout_s"] = _system.sleepTimeoutS;
    sys["primary_color"]   = _system.primaryColor;
    sys["color_theme"]     = _system.colorTheme;
    sys["power_save_enabled"] = _system.powerSaveEnabled;

    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["enabled"]  = _wifi.enabled;
    wifi["ssid"]     = _wifi.ssid;
    wifi["password"] = _wifi.password;

    JsonObject tm = doc["time"].to<JsonObject>();
    tm["timezone"]    = _time.timezone;
    tm["ntp_server"]  = _time.ntpServer;
    tm["format_24h"]  = _time.format24h;
    tm["date_format"] = _time.dateFormat;

    JsonObject menu = doc["menu"].to<JsonObject>();
    JsonArray enabled = menu["enabled_apps"].to<JsonArray>();
    for (auto& id : _menu.enabledApps) enabled.add(id);

    JsonObject appsNode = doc["apps"].to<JsonObject>();
    for (auto* a : apps) {
        JsonObject node = appsNode[a->id()].to<JsonObject>();
        a->saveConfig(node);
    }

    File f = SD.open(CONFIG_PATH, FILE_WRITE);
    if (!f) return false;
    bool ok = serializeJsonPretty(doc, f) > 0;
    f.close();
    return ok;
}

ConfigStatus ConfigManager::load(const std::vector<App*>& apps) {
    applyDefaults(apps);

    if (!_sdReady) {
        _status = ConfigStatus::NoSdCard;
        _statusMessage = "no SD";
        return _status;
    }

    if (!SD.exists(CONFIG_PATH)) {
        // Give apps a chance to seed their defaults before we serialize.
        for (auto* a : apps) {
            JsonDocument tmp;
            JsonObject node = tmp.to<JsonObject>();
            a->loadConfig(node);
        }
        if (writeDefault(apps)) {
            _status = ConfigStatus::CreatedDefault;
            _statusMessage = "created";
        } else {
            _status = ConfigStatus::CreateError;
            _statusMessage = "create err";
        }
        return _status;
    }

    File f = SD.open(CONFIG_PATH, FILE_READ);
    if (!f) {
        _status = ConfigStatus::OpenError;
        _statusMessage = "open err";
        return _status;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err) {
        _status = ConfigStatus::JsonError;
        _statusMessage = String("json err: ") + err.c_str();
        return _status;
    }

    JsonObjectConst sys = doc["system"];
    if (!sys.isNull()) {
        if (sys["user_name"].is<const char*>())     _system.userName      = (const char*)sys["user_name"];
        if (sys["language"].is<const char*>())      _system.language      = (const char*)sys["language"];
        if (sys["brightness"].is<int>())            _system.brightness    = sys["brightness"];
        if (sys["buzzer_enabled"].is<bool>())       _system.buzzerEnabled = sys["buzzer_enabled"];
        if (sys["buzzer_volume"].is<int>())         _system.buzzerVolume  = sys["buzzer_volume"];
        if (sys["sleep_timeout_s"].is<int>())       _system.sleepTimeoutS = sys["sleep_timeout_s"];
        if (sys["primary_color"].is<const char*>()) _system.primaryColor  = (const char*)sys["primary_color"];
        if (sys["color_theme"].is<const char*>())   _system.colorTheme    = (const char*)sys["color_theme"];
        if (sys["power_save_enabled"].is<bool>())    _system.powerSaveEnabled = sys["power_save_enabled"];
    }

    JsonObjectConst wifi = doc["wifi"];
    if (!wifi.isNull()) {
        if (wifi["enabled"].is<bool>())         _wifi.enabled  = wifi["enabled"];
        if (wifi["ssid"].is<const char*>())     _wifi.ssid     = (const char*)wifi["ssid"];
        if (wifi["password"].is<const char*>()) _wifi.password = (const char*)wifi["password"];
    }

    JsonObjectConst tm = doc["time"];
    if (!tm.isNull()) {
        if (tm["timezone"].is<const char*>())    _time.timezone   = (const char*)tm["timezone"];
        if (tm["ntp_server"].is<const char*>())  _time.ntpServer  = (const char*)tm["ntp_server"];
        if (tm["format_24h"].is<bool>())         _time.format24h  = tm["format_24h"];
        if (tm["date_format"].is<const char*>()) _time.dateFormat = (const char*)tm["date_format"];
    }

    JsonObjectConst menu = doc["menu"];
    if (!menu.isNull()) {
        JsonArrayConst enabled = menu["enabled_apps"];
        if (!enabled.isNull()) {
            _menu.enabledApps.clear();
            for (JsonVariantConst v : enabled) {
                if (v.is<const char*>()) _menu.enabledApps.push_back((const char*)v);
            }
        }
    }

    // Auto-include any registered app missing from the saved list so newly
    // added firmware apps appear without editing the SD config. Trade-off: an
    // app can no longer be hidden just by removing it from enabled_apps.
    for (auto* a : apps) {
        bool present = false;
        for (const auto& id : _menu.enabledApps) {
            if (id == a->id()) { present = true; break; }
        }
        if (!present) _menu.enabledApps.push_back(a->id());
    }

    JsonObjectConst appsNode = doc["apps"];
    if (!appsNode.isNull()) {
        for (auto* a : apps) {
            JsonObjectConst node = appsNode[a->id()];
            if (!node.isNull()) a->loadConfig(node);
        }
    }

    _status = ConfigStatus::Loaded;
    _statusMessage = "loaded";
    return _status;
}

uint8_t ConfigManager::effectiveBrightness() const {
    if (_system.powerSaveEnabled && _system.brightness > POWER_SAVE_BRIGHTNESS)
        return POWER_SAVE_BRIGHTNESS;
    return _system.brightness;
}

bool ConfigManager::save(const std::vector<App*>& apps) {
    if (!_sdReady) return false;
    return writeDefault(apps);
}
