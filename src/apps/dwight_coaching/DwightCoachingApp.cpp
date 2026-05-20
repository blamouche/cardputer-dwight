#include "DwightCoachingApp.h"

#include <M5Cardputer.h>
#include <esp_random.h>

#include "core/AppManager.h"
#include "core/DwightFace.h"
#include "core/HardwareConfig.h"
#include "core/SpeechBubble.h"
#include "core/Theme.h"

namespace {

// Built-in Dwight coaching lines, used when the SD config provides none.
const char* const kDefaultQuotes[] = {
    "Whenever I'm about to do something, I think 'would an idiot do that?' and if they would, I do not do that thing.",
    "Discipline equals freedom. Beets equal power. You equal mine to command.",
    "Before you criticize a man, walk a mile in his shoes. Then you have his shoes. Now go win.",
    "A real entrepreneur is a major risk taker. Take the risk. Then crush it.",
    "I am fast. To give you a reference point, I am somewhere between a snake and a mongoose. So move.",
    "Not everything is a lesson. Sometimes you just fail. Then you stop failing. Today you stop.",
    "Identity theft is not a joke. Neither is wasting this day. Seize it, drone.",
    "When my mother was pregnant with me, they did an ultrasound and found she was having twins. I resorbed the other one. Be that decisive.",
    "There are 3.7 trillion fish in the ocean and yet you fixate on excuses. Stop. Work.",
    "I never smile if I can help it. Showing one's teeth is a submission signal in primates. Frown and conquer.",
    "Through concentration, I can raise and lower my cholesterol at will. You can finish that task.",
    "Nobody should have to go to work thinking 'oh this is the place I might die today'. So make today count instead.",
};
const int kDefaultQuoteCount = (int)(sizeof(kDefaultQuotes) / sizeof(kDefaultQuotes[0]));

}  // namespace

void DwightCoachingApp::setup(AppManager& mgr) {
    _mgr = &mgr;
    if (_quotes.empty()) {
        for (int i = 0; i < kDefaultQuoteCount; ++i)
            _quotes.push_back(kDefaultQuotes[i]);
    }
}

void DwightCoachingApp::onEnter() {
    if (!_seeded) {
        randomSeed(esp_random());
        _seeded = true;
    }
    pickRandomQuote();
    _dirty      = true;
    _lastDrawMs = 0;
}

// Pick a fresh quote, avoiding an immediate repeat when more than one exists.
void DwightCoachingApp::pickRandomQuote() {
    int n = (int)_quotes.size();
    if (n <= 1) { _index = 0; return; }
    int next = _index;
    while (next == _index) next = (int)random(n);
    _index = next;
}

void DwightCoachingApp::loop() {
    // Any key press serves up the next random quote.
    if (_mgr->inputChanged() && _mgr->inputPressed()) {
        pickRandomQuote();
        _dirty = true;
    }

    uint32_t now = millis();
    if (_dirty || now - _lastDrawMs >= 200) {
        _lastDrawMs = now;
        _dirty      = false;
        draw();
    }
}

void DwightCoachingApp::draw() {
    auto* canvas = _mgr->canvas();
    canvas->fillScreen(gTheme.background);
    canvas->setTextSize(1);

    // Portrait on the left, coaching bubble on the right.
    dwightDrawAsciiFace(canvas, 0);

    const int bx = dwightAsciiFaceWidth() + 6, by = 8;
    const int bw = DISPLAY_W - bx - 4, bh = DISPLAY_H - by - 16;
    drawSpeechBubble(canvas, bx, by, bw, bh, gTheme.accent);

    const char* quote = _quotes.empty() ? "" : _quotes[_index].c_str();
    drawWrappedText(canvas, quote, bx + 8, by + 8, bw - 16, gTheme.foreground, 11);

    // Footer hint.
    canvas->setTextColor(gTheme.muted);
    canvas->setCursor(2, DISPLAY_H - 11);
    canvas->print("Any key = next  `=back");

    canvas->pushSprite(0, 0);
}

void DwightCoachingApp::loadConfig(JsonObjectConst node) {
    if (node.isNull()) return;
    JsonArrayConst arr = node["quotes"];
    if (!arr.isNull()) {
        _quotes.clear();
        for (JsonVariantConst v : arr) {
            if (v.is<const char*>()) {
                String s = (const char*)v;
                if (s.length()) _quotes.push_back(s);
            }
        }
    }
}

void DwightCoachingApp::saveConfig(JsonObject node) {
    JsonArray arr = node["quotes"].to<JsonArray>();
    for (auto& q : _quotes) arr.add(q);
}
