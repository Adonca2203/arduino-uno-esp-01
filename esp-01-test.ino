#include "display.h"
#include "fonts.h"
#include "esp01.h"
#include "config.h"

ESP01 wifi(3, 2, 9600);

unsigned long lastWeatherUpdate = 0;
const unsigned long WEATHER_INTERVAL = 300000;  // 5 minutes
const uint8_t TEMPERATURE_PAGE = 2;
const uint8_t WEATHER_DESC_PAGE = 4;

const char weatherHost[] PROGMEM = "api.weatherapi.com";
const char weatherBase[] PROGMEM = "/v1/current.json?key=";

char currentTemp[10] = "--";       // highest should be 1xx.xx°F (degree symbol is 2 bytes and null terminator is 1)
char weatherCondition[17] = "--";  // My OLED display has 128 rows to display text each 8 pixels wide 128 / 8 = 16
                                   // Maybe change this so it overflows into a new line on the board? (Save 2 lines for description)
bool initialized = false;
bool shouldRetryWeather = false;

void setup() {
  Serial.begin(9600);
  initDisplay();
  displayText("Init ESP...", 2, 0);

  if (wifi.begin()) {
    clearPage(2);
    displayText("WiFi Mod OK", 2, 0);
    displayText("Connecting wifi", 4, 0);

    if (wifi.connectToWiFi(WIFI_SSID, WIFI_PW)) {
      displayText("Connected!", 6, 0);
      clearDisplay();
    }
  }
}

void loop() {
  if (!initialized) {
    updateDisplay();
    initialized = true;
  }

  unsigned long currentTime = millis();
  if (lastWeatherUpdate == 0 || (currentTime - lastWeatherUpdate > WEATHER_INTERVAL)) {
    updateWeather();

    if (shouldRetryWeather) {
      delay(5000);
      return;
    }

    updateDisplay();
    lastWeatherUpdate = currentTime;
  }

  delay(1000);
}

void updateDisplay() {
  // Clear data areas only
  clearPage(TEMPERATURE_PAGE);
  clearPage(WEATHER_DESC_PAGE);

  // Redraw static parts
  displayText("Weather:", 0, 0);

  // Display weather data using char arrays
  displayText(currentTemp, TEMPERATURE_PAGE, 0);
  displayText(weatherCondition, WEATHER_DESC_PAGE, 0);

  // WiFi status
  if (wifi.isConnected()) {
    displayText("*", 0, 120);
  } else {
    displayText("X", 0, 120);
  }
}

void updateWeather() {
  char path[80];
  char hostBuf[20];

  strcpy_P(hostBuf, weatherHost);
  strcpy_P(path, weatherBase);
  strcat(path, WEATHER_API_KEY);

  char locationBuf[15];
  strcpy_P(locationBuf, weatherLocation);
  strcat(path, locationBuf);

  char tempF[10];
  char condition[17];

  bool success = wifi.getWeatherData(hostBuf, 80, path, tempF, sizeof(tempF), condition, sizeof(condition));

  if (success) {
    Serial.print(F("Temp (F): "));
    Serial.println(tempF);
    Serial.print(F("Condition: "));
    Serial.println(condition);

    snprintf(currentTemp, sizeof(currentTemp), "%s°F", tempF);

    strncpy(weatherCondition, condition, sizeof(weatherCondition) - 1);
    weatherCondition[sizeof(weatherCondition) - 1] = '\0';
    shouldRetryWeather = false;
  } else {
    shouldRetryWeather = true;
  }
}