# Bitcoin Tracker and Clock v5.3

![bitclock_ani (1)](https://github.com/CountZero1066/Bitcoin-Tracker-and-Clock/assets/32957102/baf71995-f1fb-4dd9-848c-cda52f265354)

# Description
This project is designed for use with the ESP8266 microcontroller and the AZDelivery MAX7219 LED Dot Matrix Display. It functions as a Bitcoin price tracker, displaying real-time updates and the current time and date. The display brightness pulsates for a visual effect.

# Features
- Real-time Bitcoin price tracking using the CoinDesk API.
- Time and date display on the hour and every quarter-hour.
- Adjustable display brightness with a pulsating effect.
- Nighttime brightness reduction between 01:00 and 07:00.
- Switch to BST (UTC + 1) from 1 April to 31 October.
  
# Components
- ESP8266 microcontroller.
- MAX7219 LED Dot Matrix Display.
  
# Libraries
- MD_Parola
- MD_MAX72xx
- SPI
- ESP8266WiFi
- ArduinoJson
  
# Setup
1) Connect the ESP8266 and MAX7219 display.
2) Set up the Wi-Fi connection.
3) Upload the code to the ESP8266.
   

