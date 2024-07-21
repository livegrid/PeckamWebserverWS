/*
  Combined ESP32 I2S Microphone Sample and ESP Dash Chart Example

  Includes:
  - ESP32 I2S Microphone setup
  - ESP Dash setup for real-time chart updates

  Dependencies:
  - ESP Dash library
  - INMP441 I2S microphone

  DroneBot Workshop 2022
  https://dronebotworkshop.com

  ESP-DASH Lite - Chart Example
  https://github.com/ayushsharma82/ESP-DASH
  https://docs.espdash.pro
*/

// Include I2S driver and ESP Dash libraries
#include <driver/i2s.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>

// Microphone I2S pin configurations
#define I2S_WS 25
#define I2S_SD 33
#define I2S_SCK 32
#define I2S_PORT I2S_NUM_0

// Define input buffer length for I2S
#define bufferLen 64
int16_t sBuffer[bufferLen];

// WiFi credentials
const char *ssid = "PeckamWebserver";
const char *password = "webserver22";

// Webserver and Dashboard setup
AsyncWebServer server(80);
ESPDash dashboard(&server);

// Number of data points to display in the chart
const int chartSize = 60;
int chartData[chartSize] = {0}; // Initialize with zeros
int XAxis[chartSize] = {0};

// Line Chart for displaying microphone data
Chart micData(&dashboard, BAR_CHART, "Microphone Data");


// Timer interval and last update time
unsigned long updateInterval = 5000; // Update every 1000ms
unsigned long lastUpdateTime = 0;

// Variables for accumulating readings
float accumulatedMean = 0;
int accumulatedCount = 0;

void i2s_install() {
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  Serial.begin(115200);

  // Connect WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize I2S
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
  Serial.println("I2S Initialized");

  micData.updateX(XAxis, chartSize);

  // Start AsyncWebServer
  server.begin();
  Serial.println("Server started");
}

void loop() {
  unsigned long currentMillis = millis();
  
  size_t bytesIn = 0;
  esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen, &bytesIn, portMAX_DELAY);

  if (result == ESP_OK) {
    int16_t samples_read = bytesIn / 2; // Corrected division by 2 for 16-bit samples
    if (samples_read > 0) {
      float mean = 0;
      for (int16_t i = 0; i < samples_read; ++i) {
        mean += sBuffer[i];
      }
      mean /= samples_read;

      // Accumulate the mean values
      accumulatedMean += mean;
      accumulatedCount++;
    }
  } else {
    Serial.print("I2S read error: ");
    Serial.println(result);
  }

  // Check if it's time to update the chart
  if (currentMillis - lastUpdateTime >= updateInterval) {
    lastUpdateTime = currentMillis;

    // Calculate the average of accumulated mean values
    if (accumulatedCount > 0) {
      float avgMean = accumulatedMean / accumulatedCount;
      Serial.print("Average Mean value: ");
      Serial.println(avgMean);

      // Shift chart data to the left and add new data point
      for (int i = 1; i < chartSize; i++) {
        chartData[i - 1] = chartData[i];
      }
      chartData[chartSize - 1] = avgMean;

      // Update chart Y axis with the new data
      micData.updateY(chartData, chartSize);

      // Send updates to the dashboard
      dashboard.sendUpdates();
      Serial.println("Dashboard updated");

      // Reset the accumulation variables
      accumulatedMean = 0;
      accumulatedCount = 0;
    } else {
      Serial.println("No samples accumulated");
    }
  }
}
