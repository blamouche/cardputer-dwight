#pragma once

#include <Arduino.h>

// Captive-portal HTML served to every device that joins the SoftAP. It is a
// fully self-contained page (inline CSS, no external resources) because the
// portal has no internet uplink. Two markers are substituted at runtime by
// BeetFarmWifiApp before the page is served:
//   %YT_URL%  -> the YouTube watch URL (PLAY button + QR target)
//   %QR%      -> an inline <svg> QR code of that same URL
// The QR + PLAY button let visitors open the video on their own data plan,
// since they cannot reach YouTube through this access point.
static const char PORTAL_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Schrute Farms Free WiFi</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#160707;color:#f3e7d8;font-family:Georgia,'Times New Roman',serif;
 line-height:1.5;min-height:100vh;display:flex;justify-content:center;
 padding:24px 16px}
.card{max-width:480px;width:100%;text-align:center}
.brand{letter-spacing:4px;font-size:13px;color:#c98a3a;text-transform:uppercase}
h1{font-size:30px;margin:6px 0 2px;color:#e23b6d;text-shadow:0 0 12px #7b1f3a}
.sub{font-size:13px;color:#b89b86;font-style:italic;margin-bottom:18px}
.beet{font-size:46px;margin:4px 0 10px}
.quote{background:#220b0b;border-left:4px solid #7b1f3a;padding:12px 14px;
 text-align:left;font-size:15px;border-radius:4px;margin-bottom:20px}
.play{display:inline-block;background:#e23b6d;color:#160707;font-weight:bold;
 text-decoration:none;font-family:Arial,sans-serif;font-size:18px;padding:14px 26px;
 border-radius:8px;box-shadow:0 4px 0 #7b1f3a;letter-spacing:1px}
.play:active{transform:translateY(3px);box-shadow:0 1px 0 #7b1f3a}
.qrwrap{background:#fff;display:inline-block;padding:12px;border-radius:8px;
 margin-top:22px}
.qrwrap svg{width:180px;height:180px;display:block}
.hint{font-size:12px;color:#b89b86;margin-top:10px}
.sig{margin-top:24px;font-size:13px;color:#c98a3a}
.sig b{color:#e23b6d}
</style></head>
<body><div class="card">
<div class="brand">Schrute Farms</div>
<h1>FALSE.</h1>
<div class="sub">You did not find free WiFi. I found you.</div>
<div class="beet">&#129365;</div>
<div class="quote">
Welcome, drone. This network is mine, the bandwidth is mine, and now your
attention is mine. There is no internet here &mdash; only beets, bears, and
Battlestar Galactica. Reward yourself with productivity propaganda below.
</div>
<a class="play" href="%YT_URL%">&#9654;&nbsp; WATCH THE OFFICE</a>
<div class="qrwrap">%QR%</div>
<div class="hint">No signal on the farm. Scan the code with your own data
to watch.</div>
<div class="sig">&mdash; <b>Dwight K. Schrute</b>, Assistant Regional WiFi Manager</div>
</div></body></html>)HTML";
