//Bitcoin Tracker and Clock V2.0
//03-12-2021
//Robert James Hastings
//117757785@umail.ucc.ie

/*
   ###############################################################
   #                 Bitcoin Tracker & Clock                     #
   #                                                             #
   #      Intended for use with the ESPIRIFF Systems ESP8266     #
   #      and the AZDelivery MAX7219 LED Dot Matrix Display      #
   #      (8 x 32). Can be adapted for more than 4 units,        #
   #      important for when Bitcoin exceeds $100,000.           # 
   #      Current version will only display time and date on     #
   #      the hour (exp 14:00).                                  #
   #      V2 will now dim the display between the hours of       #
   #      1am - 8am and will supress the                         #
   #      Pulse_Display_Brightness() function during these       #
   #      hours                                                  #
   ###############################################################
*/

//_________________libraries__________________
//********************************************************************************
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

//________________initializing Display__________
// Define hardware type, size, and output pins:
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define  CLK_PIN   14 // or SCK
#define DATA_PIN  13 // or MOSI
#define CS_PIN    15 // or SS
// Create a new instance of the MD_Parola class with hardware SPI connection:
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);


//________________Declaring Variables__________
const char* host = "api.coindesk.com";
const char* ssid     = "xxxxxxxxxxxx";
const char* password = "xxxxxxxxxxxx";
const char * c;
String previous_non_zero_price = "JSON NOT FOUND";
String previous_non_zero_time_and_date = "JSON NOT FOUND";
String previous_non_zero_time_24 = "JSON NOT FOUND";
String url = "/v1/bpi/currentprice.json";
String jsonAnswer;
int jsonIndex;
int rateIndex;
String priceString;
float price;
int check_zero;
int timeIndex;
String timeString;
String time24hr;
String new_hour;
int current_hour;
bool pulse_brightness;

//____________________Setup__________________________
//********************************************************************************
void setup() {
  Serial.begin(115200);
  delay(250);
  // Intialize the object:
  myDisplay.begin();
  // Set the intensity (brightness) of the display (0-15):
  myDisplay.setIntensity(0);
  myDisplay.displayClear();
  delay(1000);
   myDisplay.setTextAlignment(PA_CENTER);
   myDisplay.setInvert(true);
    myDisplay.print("Bitcoin");
    Pulse_Display_Brightness();
    myDisplay.setIntensity(5);
    delay(50);
    
//____________________Establish Wifi Connection__________________________
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
    Display_Scroll_Text("Starting, please wait", 60);
    myDisplay.displayClear();
     myDisplay.setTextAlignment(PA_CENTER);
     myDisplay.setInvert(false);
     myDisplay.print("......");
}


//____________________Main Loop__________________________
//********************************************************************************
void loop() {
  
 Get_JSON_Data();
 Get_Data_From_JSON();
 myDisplay.displayClear();
 Main_Logic_Tree();
 
}


//____________________Scrolling Text Function__________________________
//********************************************************************************
void Display_Scroll_Text(String text_to_scroll, int timedelay){
  
  const char * x = text_to_scroll.c_str();
  myDisplay.displayClear();
    delay(50);
    myDisplay.displayText(x, PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    int charlen = sizeof(x)/sizeof(char) * timedelay;
    for(int i = 0; i <charlen; ++i){ 
      delay(10);
    if (myDisplay.displayAnimate()) {
    myDisplay.displayReset();
    }
    delay(50);
}

}

//____________________Pulse Display Brightness For Static Text__________________________
//********************************************************************************
void Pulse_Display_Brightness(){
  
if (pulse_brightness)
 {
  int n;
  myDisplay.setIntensity(0);
  for(n=0; n<14; ++n){
      myDisplay.setIntensity(n);
      delay(200);
    }
    for(n=14; n>1; --n){
      myDisplay.setIntensity(n);
      delay(200);
    }
    for(n=0; n<5; ++n){
      myDisplay.setIntensity(n);
      delay(200);
    }
    myDisplay.setIntensity(5);
   }
   else
   {
    delay(2000);
   }
}

//____________________Get Json Data From Coindesk__________________________
//********************************************************************************
void Get_JSON_Data(){
  
   // Connect to API
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(50);
  
  // Read all the lines of the reply from server and print them to Serial
  String answer;
  while(client.available()){
    String line = client.readStringUntil('\r');
    answer += line;
  }
  client.stop();


  // Convert to JSON
  for (int i = 0; i < answer.length(); i++) {
    if (answer[i] == '{') {
      jsonIndex = i;
      break;
    }
  }
  // Get JSON data
  jsonAnswer = answer.substring(jsonIndex);
  
}

//____________________Isolate Desired Data From JSON__________________________
//********************************************************************************
void Get_Data_From_JSON(){
  
  // Get rate as float
  rateIndex = jsonAnswer.indexOf("rate_float");
  priceString = jsonAnswer.substring(rateIndex + 12, rateIndex + 17);
  priceString.trim();
  price = priceString.toFloat();

  // Get time
  timeIndex = jsonAnswer.indexOf("time");
  timeString = jsonAnswer.substring(timeIndex + 18, timeIndex + 39);
  time24hr = jsonAnswer.substring(timeIndex + 30, timeIndex + 35);
  new_hour = jsonAnswer.substring(timeIndex + 33, timeIndex + 35);
  current_hour = (jsonAnswer.substring(timeIndex + 30, timeIndex + 31)).toInt();
  
}

//____________________Decision Tree For Displaying Data__________________________
//********************************************************************************
void Main_Logic_Tree(){

if(current_hour >= 1 && current_hour <= 7){
  myDisplay.setIntensity(0);
  pulse_brightness = false;
}
else{
  myDisplay.setIntensity(5);
   pulse_brightness = true;
}
  myDisplay.displayClear();
  check_zero = (int)price; //check if Coindesk returns a zero value and defer to previous non zero reply if true
    if(check_zero > 0){
      if(new_hour == "00"){
            Display_Scroll_Text(timeString, 65);           
      }
      myDisplay.displayClear();
      myDisplay.print(time24hr);
      Pulse_Display_Brightness();
      delay(250);
      previous_non_zero_time_and_date = timeString;     
      previous_non_zero_time_24 = time24hr;
      delay(2500);
      
      myDisplay.displayClear();
      Display_Scroll_Text("USD $" + priceString, 44);
      previous_non_zero_price = priceString;
      myDisplay.setTextAlignment(PA_CENTER);
      myDisplay.print(priceString);
    }
      else{
         if(new_hour == "00"){
             Display_Scroll_Text(previous_non_zero_time_and_date, 65);
         }
      myDisplay.displayClear();
      myDisplay.print(previous_non_zero_time_24);
      Pulse_Display_Brightness(); 
      delay(250);
      
      myDisplay.displayClear();
      Display_Scroll_Text("USD $ " + previous_non_zero_price, 44);
      myDisplay.setTextAlignment(PA_CENTER);
      myDisplay.displayClear();
      myDisplay.print(previous_non_zero_price);
      delay(1200);
    } 
   delay(50);
   
}
//********************************************************************************
