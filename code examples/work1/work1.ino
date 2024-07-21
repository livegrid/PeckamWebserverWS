#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <FastLED.h>

const char *ssid = "PeckamWebserver";
const char *password = "webserver22";

#define deviceName "esp32"

WebServer server(80);

#define NUM_LEDS 1
#define DATA_PIN 16
CRGB leds[NUM_LEDS];

bool redLedON = false;
bool greenLedON = false;
bool blueLedON = false;

void handleRoot() {
  server.send(200, "text/plain", "hello from esp32!");
}

void handleToggleRed() {
  redLedON = !redLedON;
  if(redLedON) leds[0].red = 255;
  else leds[0].red = 0;
  FastLED.show();
  server.send(200, "text/plain", redLedON ? "Red LED ON" : "Red LED OFF");
}

void handleToggleGreen() {}

void handleToggleBlue() {}

void handleNotFound() {
  server.send(404, "text/plain", "File Not Found");
}

void setup(void) {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(deviceName)) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/red", handleToggleRed);
  server.on("/blue", handleToggleBlue);
  server.on("/green", handleToggleGreen);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);  //allow the cpu to switch to other tasks
}
