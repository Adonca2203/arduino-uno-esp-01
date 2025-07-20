#include "esp01.h"

ESP01::ESP01(uint8_t rx_pin, uint8_t tx_pin, long baud_rate) {
  esp_serial = new SoftwareSerial(rx_pin, tx_pin);
  esp_serial->begin(baud_rate);
  timeout_ms = 5000;
}

ESP01::~ESP01() {
  delete esp_serial;
}

bool ESP01::begin() {
  Serial.println(F("Initializing ESP-01"));

  if (!sendATCommandRaw("AT", "OK")) {
    Serial.println(F("Failed to communicate"));
    return false;
  }

  if (!sendATCommandRaw("AT+CWMODE=1", "OK")) {
    Serial.println(F("Failed to set mode"));
    return false;
  }

  Serial.println(F("ESP-01 initialized"));
  return true;
}

bool ESP01::sendATCommandRaw(const char* command, const char* expected1, const char* expected2, unsigned long timeoutMs) {
  Serial.print(F("Sending: "));
  Serial.println(command);

  // Flush the serial before sending a new command
  while (esp_serial->available()) esp_serial->read();

  esp_serial->println(command);

  char responseBuffer[100];
  int pos = 0;
  unsigned long start = millis();

  while (millis() - start < timeoutMs) {
    while (esp_serial->available()) {
      char c = esp_serial->read();

      if (pos < (int)sizeof(responseBuffer) - 1) {
        responseBuffer[pos++] = c;
        responseBuffer[pos] = 0;
      }

      if (expected1 && strstr(responseBuffer, expected1)) return true;
      if (expected2 && strstr(responseBuffer, expected2)) return true;
    }
    delay(10);
  }

  Serial.println(F("Timeout or no expected response"));
  return false;
}

bool ESP01::isConnected() {
  return sendATCommandRaw("AT", "OK");
}

bool ESP01::connectToWiFi(const char* ssid, const char* password) {
  Serial.print(F("Connecting to: "));
  Serial.println(ssid);

  char command[80];
  snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

  if (!sendATCommandRaw(command, "OK", "FAIL", 15000)) {
    Serial.println(F("WiFi failed"));
    return false;
  }

  Serial.println(F("WiFi connected"));
  return true;
}

bool ESP01::disconnectWiFi() {
  return sendATCommandRaw("AT+CWQAP", "OK");
}

bool ESP01::getWeatherData(const char* host, int port, const char* path,
                           char* out_temp_f, int temp_f_len,
                           char* out_condition, int cond_len) {
  char connect_cmd[60];
  snprintf(connect_cmd, sizeof(connect_cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d", host, port);
  if (!sendATCommandRaw(connect_cmd, "OK", "ALREADY CONNECT", 7000)) {
    Serial.println(F("TCP connect failed"));
    return false;
  }

  char http_request[150];
  snprintf(http_request, sizeof(http_request),
           "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
           path, host);
  char send_cmd[30];
  snprintf(send_cmd, sizeof(send_cmd), "AT+CIPSEND=%d", (int)strlen(http_request));
  if (!sendATCommandRaw(send_cmd, ">", "ERROR", 5000)) {
    Serial.println(F("Failed to get > prompt"));
    sendATCommandRaw("AT+CIPCLOSE", "OK", nullptr, 2000);
    return false;
  }

  esp_serial->print(http_request);

  if (!sendATCommandRaw("", "SEND OK", "ERROR", 5000)) {
    Serial.println(F("Send failed"));
    sendATCommandRaw("AT+CIPCLOSE", "OK", nullptr, 2000);
    return false;
  }

  const char* headerEndSeq = "\r\n\r\n";
  int headerMatch = 0;

  const char* temp_f_key = "\"temp_f\":";
  const char* text_key = "\"text\":\"";

  int temp_f_idx = 0, text_idx = 0;
  bool found_temp_f = false, found_text = false;
  bool capturing_temp = false, capturing_text = false;
  int match_temp = 0;
  int match_text = 0;

  unsigned long startTime = millis();

  while (millis() - startTime < timeout_ms) {
    if (esp_serial->available()) {
      char c = esp_serial->read();

      // Skip headers
      if (headerMatch < 4) {
        if (c == headerEndSeq[headerMatch]) {
          headerMatch++;
        } else {
          headerMatch = 0;
        }
        if (headerMatch == 4) {
          Serial.println(F("\nHeaders done, parsing JSON..."));
        }
        continue;
      }

      // Parse "temp_f"
      if (!found_temp_f && !capturing_temp) {
        if (c == temp_f_key[match_temp]) {
          match_temp++;
          if (temp_f_key[match_temp] == '\0') {
            found_temp_f = true;
            capturing_temp = true;
          }
        } else {
          match_temp = 0;
        }
      } else if (capturing_temp) {
        if (c == ',' || c == '}') {
          out_temp_f[temp_f_idx] = '\0';
          capturing_temp = false;
        } else if (temp_f_idx < temp_f_len - 1) {
          out_temp_f[temp_f_idx++] = c;
        }
      }

      // Parse "text"
      if (!found_text && !capturing_text) {
        if (c == text_key[match_text]) {
          match_text++;
          if (text_key[match_text] == '\0') {
            found_text = true;
            capturing_text = true;
          }
        } else {
          match_text = 0;
        }
      } else if (capturing_text) {
        if (c == '"') {
          out_condition[text_idx] = '\0';
          capturing_text = false;
        } else if (text_idx < cond_len - 1) {
          out_condition[text_idx++] = c;
        }
      }

      if (!capturing_temp && !capturing_text && found_temp_f && found_text) {
        break;
      }
    }
  }

  sendATCommandRaw("AT+CIPCLOSE", "OK", nullptr, 2000);

  return found_temp_f && found_text;
}

void ESP01::reset() {
  Serial.println(F("Resetting ESP-01"));
  sendATCommandRaw("AT+RST", "OK");
  delay(3000);
}