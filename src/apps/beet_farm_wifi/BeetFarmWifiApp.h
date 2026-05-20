#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>

#include "core/App.h"

class AppManager;

// Turns the Cardputer into an open WiFi access point with a captive portal.
// Any device that joins the network has its browser pop open a Dwight-themed
// page that pitches an "The Office" YouTube video. Because the access point has
// no internet uplink, the video itself can't stream here: the page offers a QR
// code + PLAY button so visitors open it on their own data plan instead.
// ESC / backtick returns to the launcher (handled globally by AppManager) and
// tears the WiFi stack down.
class BeetFarmWifiApp : public App {
public:
    const char* id()       const override { return "beet_farm_wifi"; }
    const char* name()     const override { return "Beet Farm WiFi"; }
    const char* subtitle() const override { return "WiFi"; }

    void setup(AppManager& mgr) override;
    void onEnter() override;
    void onExit() override;
    void loop() override;

    void loadConfig(JsonObjectConst node) override;
    void saveConfig(JsonObject node) override;

private:
    void startPortal();
    void stopPortal();
    void buildPage();          // assembles _pageHtml (template + QR + URL), once
    String buildQrSvg() const; // inline <svg> QR code of _youtubeUrl
    void handlePortal();       // serves the page for any request
    void draw();

    AppManager* _mgr = nullptr;

    String _ssid       = "Schrute_Farms_Free_WiFi";
    String _youtubeUrl = "https://www.youtube.com/watch?v=AJmaVPfyudQ";
    uint8_t _channel   = 1;

    WebServer _server{80};
    DNSServer _dns;
    bool _running = false;

    String _pageHtml;  // cached, fully-substituted portal page

    uint32_t _lastDrawMs = 0;
};
