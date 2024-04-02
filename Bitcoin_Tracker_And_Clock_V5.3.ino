/*
 Bitcoin Tracker and Clock V5.3
 Robert James Hastings
 Github CountZero1066
   ###############################################################
   #                 Bitcoin Tracker & Clock                     #
   #                                                             #
   #      Intended for use with the ESPIRIFF Systems ESP8266     #
   #      and the AZDelivery MAX7219 LED Dot Matrix Display      #
   #      (8 x 32). Can be adapted for more than 4 units,        #
   #      important for when Bitcoin exceeds $100,000.           # 
   #      Current version will display time and date on          #
   #      the hour and every quarter hour                        #
   #                                                             #
   ###############################################################

	Version detail: Includes adjustments for DST. Increments hour + 1 between 1 April and 31 October
*/
//-------------------------------------------------------------------------------------------
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 14 // or SCK
#define DATA_PIN 13 // or MOSI
#define CS_PIN 15 // or SS

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
DynamicJsonDocument jsonDoc(1024); // Adjust the size as needed

String jsonAnswer; // Declaration for the JSON data variable
const char* ssid     = "Network name";
const char* password = "Password";
float price;
String new_hour, priceString;
bool pulse_brightness = true;
String previous_non_zero_price = "JSON NOT FOUND";
String previous_non_zero_time_and_date = "JSON NOT FOUND";
String previous_non_zero_time_24 = "JSON NOT FOUND";
int previous_non_zero_hour, current_hour;

// Function prototypes
String getBitcoinPriceUSD();
String getCurrentTime();
String getCurrentDate();
bool isOnHourOrQuarter(String time);
void Display_Scroll_Text(String text_to_scroll, int timedelay);
void Pulse_Display_Brightness();
void Get_JSON_Data();
//-------------------------------------------------------------------------------------------
//Setup
//-------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(250);

  myDisplay.begin();
  myDisplay.setIntensity(0);
  myDisplay.displayClear();
  delay(1000);

  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.setInvert(false);
  
  myDisplay.print("V5.3");
  Pulse_Display_Brightness();
  myDisplay.setIntensity(5);
  delay(50);

   WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  myDisplay.setInvert(false);
  Display_Scroll_Text("Starting, please wait", 20);
  myDisplay.displayClear();
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.setInvert(false);
  myDisplay.print("------");
}
//-------------------------------------------------------------------------------------------
//Main Loop
//-------------------------------------------------------------------------------------------
void loop() {
  Get_JSON_Data();
  myDisplay.displayClear();
  Display_Scroll_Text("USD $" + getBitcoinPriceUSD(), 24);
  myDisplay.displayClear();
  myDisplay.print(getBitcoinPriceUSD());
  delay(1000);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(getBitcoinPriceUSD());
  if (isOnHourOrQuarter(getCurrentTime())) {
    Display_Scroll_Text(getCurrentDate(), 65);
  }
  myDisplay.displayClear();
  myDisplay.print(getCurrentTime());
  Pulse_Display_Brightness();
  delay(250);
}
//-------------------------------------------------------------------------------------------
// Extract Bitcoin price
//-------------------------------------------------------------------------------------------

String getBitcoinPriceUSD() {
  int priceInt = static_cast<int>(jsonDoc["bpi"]["USD"]["rate_float"].as<float>());
  return String(priceInt);
}
//-------------------------------------------------------------------------------------------
//Extract Time
//-------------------------------------------------------------------------------------------
String getCurrentTime() {
  // Parse the JSON data
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + 210;
  DynamicJsonDocument doc(bufferSize);
  deserializeJson(doc, jsonAnswer);

  // Extract the updatedISO time string
  String isoTime = doc["time"]["updatedISO"];

  // Adjust for daylight saving time
  isoTime = adjustForDST(isoTime);

  // Extract and format the time part with offset consideration (without seconds and timezone info)
  String timePart = isoTime.substring(0, 5); // Extract hh:mm part

  // Print the extracted time for debugging
  Serial.print("Extracted Time: ");
  Serial.println(timePart);

  return timePart;
}
//-------------------------------------------------------------------------------------------
//Check for adjusting night time brightness
//-------------------------------------------------------------------------------------------
bool nighttimebrightness(String time) {
  // Extract the hour part of the time
  int hour = time.substring(0, 2).toInt();

  // Check if the hour is between 01 and 07 (inclusive)
  if (hour >= 1 && hour <= 7){
    pulse_brightness = false;
    return true;
  }
  else{
    pulse_brightness = true;
    return false;
  }
}
//-------------------------------------------------------------------------------------------
//Adjust time for daylight saving
//-------------------------------------------------------------------------------------------
String adjustForDST(String time) {
  Serial.print("Time String: ");
  Serial.println(time);

  // Extract the month
  String monthString = time.substring(5, 7); // Extract characters at positions 5 and 6
  int month = monthString.toInt(); 

  // Extract the day
  String dayString = time.substring(8, 10); // Extract characters at positions 8 and 9
  int day = dayString.toInt(); 

  // Print month and day for debugging
  Serial.print("Month: ");
  Serial.println(monthString);
  Serial.print("Day: ");
  Serial.println(dayString);

  // If the month is between April and October (inclusive), and the day is not the last day of the month, adjust for daylight saving time
  if (month >= 4 && month <= 10 && day != 31) {
    int hours = time.substring(11, 13).toInt(); // Extract characters at positions 11 and 12 for hours
    hours = (hours + 1) % 24; // Add one hour for DST
    time = (hours < 10 ? "0" : "") + String(hours) + time.substring(13); // Reconstruct the time string with adjusted hours
  }

  return time;
}
//-------------------------------------------------------------------------------------------
//Extract Date
//-------------------------------------------------------------------------------------------
String getCurrentDate() {
  // Extract and return the date part
  String updatedTime = jsonDoc["time"]["updated"].as<String>();
  return updatedTime.substring(0, updatedTime.indexOf(',') + 6);
}
//-------------------------------------------------------------------------------------------
// Check if the time is either on the hour or the quarter-hour
//-------------------------------------------------------------------------------------------
bool isOnHourOrQuarter(String time) {
  // Extract the minute part of the time
  String minutePart = time.substring(3);
  return (minutePart == "00" || minutePart == "15" || minutePart == "30" || minutePart == "45");
}
//-------------------------------------------------------------------------------------------
//Scroll Text
//-------------------------------------------------------------------------------------------
void Display_Scroll_Text(String text_to_scroll, int timedelay) {
  const char* x = text_to_scroll.c_str();
  myDisplay.displayClear();
  delay(50);
  myDisplay.displayText(x, PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  int charlen = text_to_scroll.length() * timedelay;
  for (int i = 0; i < charlen; ++i) {
    delay(10);
    if (myDisplay.displayAnimate()) {
      myDisplay.displayReset();
    }
    delay(50);
  }
}
//-------------------------------------------------------------------------------------------
//Pulse the brightness of the display up and down
//-------------------------------------------------------------------------------------------
void Pulse_Display_Brightness() {
  if (pulse_brightness) {
    int n;
    myDisplay.setIntensity(0);
    for (n = 0; n < 14; ++n) {
      myDisplay.setIntensity(n);
      delay(200);
    }
    for (n = 14; n > 1; --n) {
      myDisplay.setIntensity(n);
      delay(200);
    }
    for (n = 0; n < 5; ++n) {
      myDisplay.setIntensity(n);
      delay(200);
    }
    myDisplay.setIntensity(5);
  } else {
    delay(2000);
  }
}
//-------------------------------------------------------------------------------------------
//Retrieve JSON data from API
//-------------------------------------------------------------------------------------------
void Get_JSON_Data() {
  const int maxRetries = 5;
  int retryCount = 0;

  while (retryCount <= maxRetries) {
    WiFiClient client;
    const int httpPort = 80;

    if (!client.connect("api.coindesk.com", httpPort)) {
      Serial.println("Connection failed, retrying...");
      retryCount++;
      delay(1000); // Wait for a moment before retrying
      continue;
    }

    client.print(String("GET ") + "/v1/bpi/currentprice.json" + " HTTP/1.1\r\n" +
                 "Host: " + "api.coindesk.com" + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(50);

    String answer;
    while (client.available()) {
      String line = client.readStringUntil('\r');
      answer += line;
    }

    client.stop();

    for (int i = 0; i < answer.length(); i++) {
      if (answer[i] == '{') {
        int jsonIndex = i;
        jsonAnswer = answer.substring(jsonIndex);

        // Parse JSON once and reuse the document
        deserializeJson(jsonDoc, jsonAnswer);
        return; // JSON successfully retrieved, exit the function
      }
    }

    Serial.println("Failed to retrieve JSON, retrying...");
    retryCount++;
    delay(1000); // Wait for a moment before retrying
  }

  Serial.println("Maximum retry count reached, giving up.");
}
