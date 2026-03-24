#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>

#define SD_CS   5
#define SD_SCK  18
#define SD_MISO 19
#define SD_MOSI 23

SPIClass spi(VSPI);
WebServer server(80);

const char* ssid = "Mini Excavator";
const char* password = "12345678";

float engineHours = 1200.5;
float batteryVoltage = 12.6;
float tempC = 32.4;
String statusText = "RUNNING";

unsigned long lastLiveLog = 0;
const unsigned long liveLogInterval = 10000;

// ---------- helpers ----------
String getTimestamp() {
  unsigned long sec = millis() / 1000;
  unsigned long mins = sec / 60;
  unsigned long hrs = mins / 60;
  sec %= 60;
  mins %= 60;

  char buf[32];
  snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", hrs, mins, sec);
  return String(buf);
}

// ---------- web page ----------
String buildPage() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body{
  margin:0;
  background:#0f1314;
  color:white;
  font-family:Arial;
}
.wrap{padding:10px;}

.maingrid{
  display:grid;
  grid-template-columns:repeat(3,1fr);
  gap:12px;
}

.gauge{
  aspect-ratio:1/1;
  border-radius:50%;
  display:flex;
  align-items:center;
  justify-content:center;
  background:
    radial-gradient(circle,#101617 50%,transparent 52%),
    conic-gradient(#2eb7b0 0 260deg,#333 260deg);
  border:3px solid #555;
}

.inner{text-align:center;}
.label{font-size:14px;}
.value{font-size:36px;font-weight:bold;}
.unit{font-size:12px;}

.camtile{
  aspect-ratio:1/1;
  background:#000;
  border-radius:10px;
  display:flex;
  align-items:center;
  justify-content:center;
}

</style>
</head>

<body>
<div class="wrap">

<div class="maingrid">

<div class="gauge">
<div class="inner">
<div class="label">HOURS</div>
<div class="value">__H__</div>
<div class="unit">HRS</div>
</div>
</div>

<div class="gauge">
<div class="inner">
<div class="label">VOLT</div>
<div class="value">__V__</div>
<div class="unit">V</div>
</div>
</div>

<div class="gauge">
<div class="inner">
<div class="label">TEMP</div>
<div class="value">__T__</div>
<div class="unit">C</div>
</div>
</div>

<div class="gauge">
<div class="inner">
<div class="label">STATUS</div>
<div class="value">__S__</div>
<div class="unit">LIVE</div>
</div>
</div>

<div class="gauge">
<div class="inner">
<div class="label">SD</div>
<div class="value">OK</div>
<div class="unit">LOG</div>
</div>
</div>

<div class="camtile">
CAMERA
</div>

</div>

</div>
</body>
</html>
)rawliteral";

  page.replace("__H__", String(engineHours,1));
  page.replace("__V__", String(batteryVoltage,2));
  page.replace("__T__", String(tempC,1));
  page.replace("__S__", statusText);

  return page;
}

// ---------- routes ----------
void handleRoot() {
  server.send(200, "text/html", buildPage());
}

// ---------- setup ----------
void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);

  Serial.println("Go to:");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
}

// ---------- loop ----------
void loop() {
  server.handleClient();

  static unsigned long last = 0;
  if (millis() - last > 2000) {
    last = millis();

    engineHours += 0.01;
    batteryVoltage = 12.4 + random(-20,20)/100.0;
    tempC = 30 + random(0,50)/10.0;

    if (batteryVoltage < 11.8) statusText = "LOW";
    else statusText = "RUN";
  }
}