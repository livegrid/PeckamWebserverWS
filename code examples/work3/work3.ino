
/*
  -----------------------------
  ESPDASH Lite - Interactive Example
  -----------------------------

  Skill Level: Intermediate

  In this example we will be creating a interactive dashboard which consists 
  of a button and a slider.

  Github: https://github.com/ayushsharma82/ESP-DASH
  WiKi: https://docs.espdash.pro

  Works with both ESP8266 & ESP32

  -------------------------------

  Upgrade to ESP-DASH Pro: https://espdash.pro

*/

#include <Arduino.h>
  /* ESP32 Dependencies */
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include <FastLED.h>

/* Your WiFi Credentials */
const char *ssid = "PeckamWebserver";
const char *password = "webserver22";

#define deviceName "esp32"

/* Start Webserver */
AsyncWebServer server(80);

/* Attach ESP-DASH to AsyncWebServer */
ESPDash dashboard(&server); 

#define NUM_LEDS 1
#define DATA_PIN 16
CRGB leds[NUM_LEDS];

/* 
  Button Card
  Format - (Dashboard Instance, Card Type, Card Name)
*/
Card buttonRed(&dashboard, BUTTON_CARD, "Red");

/* 
  Slider Card
  Format - (Dashboard Instance, Card Type, Card Name, Card Symbol(optional), int min, int max)
*/
Card sliderRed(&dashboard, SLIDER_CARD, "Red", "", 0, 255);

int redValue = 0;

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  Serial.begin(115200);

  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  /* Attach Button Callback */
  buttonRed.attachCallback([&](int value){
    /* Print our new button value received from dashboard */
    Serial.println("Red Triggered: "+String((value == 1)?"true":"false"));
    if(value == 1) redValue = 255;
    else redValue = 0;
  /* Make sure we update our button's value and send update to dashboard */
    buttonRed.update(value);
    dashboard.sendUpdates();
  });

  /* Attach Slider Callback */
  sliderRed.attachCallback([&](int value){
    /* Print our new slider value received from dashboard */
    Serial.println("Slider Triggered: "+String(value));
    redValue = value;
    /* Make sure we update our slider's value and send update to dashboard */
    sliderRed.update(value);
    dashboard.sendUpdates();
  });

  /* Start AsyncWebServer */
  server.begin();
}

void loop() {
  leds[0].red = redValue;
  FastLED.show();
}
