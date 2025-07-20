# What is this?

Small arduino program I wrote to start learning a bit about embedded.

It uses an ESP-01 to connect to your local wifi and the free to use weather api to parse out
information for local weather like temperature and short description.

The parsed information is then displayed to a UCTRONICS 0.96 inch OLED screen.

## Requirements
You will need the above mentioned hardware. By default I use Software Serial to utilize pins 2 and 3 on the
Arduino Uno board to communicate with the ESP-01. Additionally ensure you are using the A4 (SDA) and A5 (SCL)
pins for the OLED screen on the arduino board as these are the default Wire ones on it.

You will need to create your own `config.h` file containing the following variables for the program to run:
```cpp
#ifndef CONFIG_H
#define CONFIG_H

const char WIFI_SSID[] = ""; // Your Wi-Fi's Name
const char WIFI_PW[] = ""; // You Wi-Fi's Password

const char WEATHER_API_KEY[] = ""; // API Key for https://www.weatherapi.com/
const char weatherLocation[] PROGMEM = "&q="; // Input your location after the = sign, I.E &q=London https://www.weatherapi.com/api-explorer.aspx

#endif
```

You will also need to reprogram the ESP-01 to use BAUD 9600 by connecting it to the normal Serial Monitor and sending
```
AT+UART_DEF=9600,8,1,0,0
```
