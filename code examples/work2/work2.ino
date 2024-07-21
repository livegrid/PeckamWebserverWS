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
  String html = "<html>\
  <body>\
  <h1>ESP32 LED Control</h1>\
  <button onclick=\"toggleLED('red')\">Toggle Red LED</button>\
  <button onclick=\"toggleLED('green')\">Toggle Green LED</button>\
  <button onclick=\"toggleLED('blue')\">Toggle Blue LED</button>\
  <script>\
  function toggleLED(color) {\
    var xhr = new XMLHttpRequest();\
    xhr.open('GET', '/' + color, true);\
    xhr.onreadystatechange = function() {\
      if (xhr.readyState == 4 && xhr.status == 200) {\
        console.log(xhr.responseText);\
      }\
    };\
    xhr.send();\
  }\
  </script>\
  </body>\
  </html>";
  
  server.send(200, "text/html", html);
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

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(deviceName)) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/red", handleToggleRed);
  server.on("/green", handleToggleGreen);
  server.on("/blue", handleToggleBlue);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
