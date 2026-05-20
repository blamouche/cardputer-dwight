#include "BeetFarmWifiApp.h"

#include <M5Cardputer.h>
#include <WiFi.h>
#include <qrcode.h>
#include <stdio.h>
#include <string.h>

#include "core/AppManager.h"
#include "core/DwightFace.h"
#include "core/HardwareConfig.h"
#include "core/SpeechBubble.h"
#include "core/Theme.h"

#include "PortalPage.h"

void BeetFarmWifiApp::setup(AppManager& mgr) {
    _mgr = &mgr;
}

void BeetFarmWifiApp::loadConfig(JsonObjectConst node) {
    if (node.isNull()) return;
    if (node["ssid"].is<const char*>())        _ssid       = (const char*)node["ssid"];
    if (node["youtube_url"].is<const char*>())  _youtubeUrl = (const char*)node["youtube_url"];
    if (node["channel"].is<int>())              _channel    = (uint8_t)node["channel"];
}

void BeetFarmWifiApp::saveConfig(JsonObject node) {
    node["ssid"]        = _ssid;
    node["youtube_url"] = _youtubeUrl;
    node["channel"]     = _channel;
}

// Builds the QR code of the YouTube URL as an inline SVG (one <rect> per dark
// module). Self-contained so it renders on the captive page with no network.
String BeetFarmWifiApp::buildQrSvg() const {
    QRCode qrcode;
    uint8_t data[qrcode_getBufferSize(4)];  // version 4 -> 33x33 modules
    qrcode_initText(&qrcode, data, 4, ECC_MEDIUM, _youtubeUrl.c_str());

    const int margin = 2;  // quiet zone in modules
    const int dim = qrcode.size + margin * 2;

    String svg;
    svg.reserve(9000);
    svg += "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 ";
    svg += dim;
    svg += " ";
    svg += dim;
    svg += "' shape-rendering='crispEdges'>";
    svg += "<rect width='100%' height='100%' fill='#fff'/>";
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                svg += "<rect x='";
                svg += (int)(x + margin);
                svg += "' y='";
                svg += (int)(y + margin);
                svg += "' width='1' height='1'/>";
            }
        }
    }
    svg += "</svg>";
    return svg;
}

void BeetFarmWifiApp::buildPage() {
    _pageHtml = String(FPSTR(PORTAL_HTML));
    _pageHtml.replace("%YT_URL%", _youtubeUrl);
    _pageHtml.replace("%QR%", buildQrSvg());
}

void BeetFarmWifiApp::handlePortal() {
    _server.send(200, "text/html", _pageHtml);
}

void BeetFarmWifiApp::startPortal() {
    if (_running) return;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(_ssid.c_str(), nullptr, _channel);  // nullptr password = open
    delay(100);
    IPAddress apIP = WiFi.softAPIP();

    // Resolve every DNS query to ourselves so the OS captive-portal probe is
    // intercepted and the sign-in page pops automatically.
    _dns.setErrorReplyCode(DNSReplyCode::NoError);
    _dns.start(53, "*", apIP);

    // Serve the page on the common OS connectivity-check URLs (returning HTML
    // instead of the expected 204/Success flags the captive portal) and on
    // anything else.
    auto serve = [this]() { handlePortal(); };
    _server.on("/", serve);
    _server.on("/generate_204", serve);
    _server.on("/gen_204", serve);
    _server.on("/hotspot-detect.html", serve);
    _server.on("/ncsi.txt", serve);
    _server.on("/connecttest.txt", serve);
    _server.onNotFound(serve);
    _server.begin();

    _running = true;
}

void BeetFarmWifiApp::stopPortal() {
    if (!_running) return;
    _server.stop();
    _dns.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    _running = false;
}

void BeetFarmWifiApp::onEnter() {
    buildPage();
    startPortal();
    _lastDrawMs = 0;
    draw();
}

void BeetFarmWifiApp::onExit() {
    stopPortal();
}

void BeetFarmWifiApp::loop() {
    if (_running) {
        _dns.processNextRequest();
        _server.handleClient();
    }

    uint32_t now = millis();
    if (now - _lastDrawMs >= 200) {
        _lastDrawMs = now;
        draw();
    }
}

void BeetFarmWifiApp::draw() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    // AP status, top-right (clear of the portrait on the left).
    const char* statusStr = _running ? "AP ON" : "OFF";
    uint16_t scolor = _running ? gTheme.success : gTheme.error;
    int sw = 6 * (int)strlen(statusStr);
    canvas->fillCircle(DISPLAY_W - sw - 8, 4, 3, scolor);
    canvas->setTextColor(scolor);
    canvas->setCursor(DISPLAY_W - sw - 2, 1);
    canvas->print(statusStr);

    // Full Dwight portrait on the left (same as the boot splash).
    dwightDrawAsciiFace(canvas, 0);

    const int colX = dwightAsciiFaceWidth() + 6;
    const int colW = DISPLAY_W - colX - 4;
    const int bx = colX, by = 16;
    const int bw = colW, bh = DISPLAY_H - by - 5;
    drawSpeechBubble(canvas, bx, by, bw, bh, gTheme.accent);

    const int ix = bx + 8, iw = bw - 16;
    int y = by + 9;

    canvas->setTextColor(gTheme.accent);
    canvas->setCursor(ix, y);
    canvas->print("BEET TRAP LIVE");
    y += 14;

    // SSID (may wrap to a second line).
    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(ix, y);
    canvas->print("SSID");
    y += 10;
    drawWrappedText(canvas, _ssid.c_str(), ix, y, iw, gTheme.foreground, 10);
    y += 21;

    // AP IP address.
    char buf[32];
    String ip = _running ? WiFi.softAPIP().toString() : String("--");
    snprintf(buf, sizeof(buf), "IP %s", ip.c_str());
    canvas->setTextColor(gTheme.foreground);
    canvas->setCursor(ix, y);
    canvas->print(buf);
    y += 12;

    // Connected client count.
    int clients = _running ? (int)WiFi.softAPgetStationNum() : 0;
    snprintf(buf, sizeof(buf), "Victims: %d", clients);
    canvas->setTextColor(clients > 0 ? gTheme.success : gTheme.foreground);
    canvas->setCursor(ix, y);
    canvas->print(buf);

    // Footer hint.
    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(ix, by + bh - 13);
    canvas->print("ESC = exit");

    canvas->pushSprite(0, 0);
}
