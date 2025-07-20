#ifndef ESP01_H
#define ESP01_H

#include <SoftwareSerial.h>
#include <Arduino.h>

class ESP01 {
private:
  SoftwareSerial* esp_serial;
  unsigned long timeout_ms;

  bool sendATCommandRaw(const char* command, const char* expected1 = nullptr, const char* expected2 = nullptr, unsigned long timeoutMs = 5000);

public:
  ESP01(uint8_t rx_pin, uint8_t tx_pin, long baud_rate = 115200);
  ~ESP01();

  bool begin();
  bool isConnected();
  bool connectToWiFi(const char* ssid, const char* password);
  bool disconnectWiFi();
  void reset();

  // Parses weather JSON response streaming from ESP01 and extracts temp_f and condition text
  bool getWeatherData(const char* host, int port, const char* path,
                      char* out_temp_f, int temp_f_len,
                      char* out_condition, int cond_len);
};

#endif
