#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <TridentTD_LineNotify.h>
#include <JC_Button.h>
#include "mqtt_secrets.h"
#include "secrets.h"
#include "RTC.h"
#include "buzzer.h"

#define I2C_SDA 21
#define I2C_SCL 22

// 3 Button Module
#define K1 34
#define K2 35
#define K3 32
// PIN, DEBOUNCE, PULLUP, INVERT
Button redButton(K1, 25, true, true);
Button yellowButton(K2, 25, true, true);
Button greenButton(K3, 25, true, true);
// default state 1 --> show Temperature
int buttonState = 1;

// Thingspeak MQTT & Wifi Setup
const char *ssid = SECRET_WIFI_SSID;
const char *pass = SECRET_WIFI_PASS;
const char *server = SECRET_THINGSPEAK_SERVER;
const char *channelID = SECRET_CHANNELID;
const char *mqttUserName = SECRET_MQTT_USERNAME;
const char *mqttPass = SECRET_MQTT_PASSWORD;
const char *clientID = SECRET_MQTT_CLIENT_ID;

// Line Notify
const char *NOTIFY_TOKEN = LINE_NOTIFY_TOKEN;
bool line_connected = false;

WiFiClient client;
PubSubClient mqtt(client);

// ****** OLED Configuration ******

// Define Screen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Define RESET PIN -1 using with ESP32 reset
#define OLED_RESET -1
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// *********************************

void setup() {
  Serial.begin(9600);
  // while(!Serial){
  //   delay(1);
  // }

  // hostname/IP server, port MQTT
  mqtt.setServer(server, 1883);

  // OLED begin at addr: 0x3C
  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
  } else {
    Serial.println("OLED Start Work !!!");
  }

  LINE.setToken(NOTIFY_TOKEN);

  // ****** RTC Setup ******
  Wire.begin(I2C_SDA, I2C_SCL);
  // seconds, minutes, hours, day, date, month, year
  // day of week (1=Sunday, 7=Saturday)
  // set date (1 to 31)
  // set year (2000+ (0-99)) ex: 2023
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // ************************

  // ****** Pinmode Setup ******
  pinMode(buzzer, OUTPUT);
  redButton.begin();
  yellowButton.begin();
  greenButton.begin();
  // pinMode(K1,INPUT_PULLUP);
  // pinMode(K2,INPUT_PULLUP);
  // pinMode(K3,INPUT_PULLUP);
  // ***************************
}

void loop() {
  checkStatusWifi();
  checkStatusMQTT();
  checkStatusNotify();
  // checkStatusMessageAPI();
  // showDisplayTemp();
  readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  // increase 1 for correct dayOfWeek
  if(firstTime){
    dayOfWeek++;
    setTime(second,minute,hour,dayOfWeek,dayOfMonth,month,year);
    firstTime = false;
  }
  showTime();  // show in Serial Monitor
  readButton();
  // showDisplayTime();
  alarm();
  // playSound();

  delay(1000);
}

void readButton() {
  if (greenButton.read()) {
    buttonState = 1;
  } else if (yellowButton.read()) {
    buttonState = 2;
  } else if (redButton.read()) {
    buttonState = 3;
  }
  switch (buttonState) {
    case 1:
      Serial.println("Show Temperature");
      showDisplayTemp();
      break;
    case 2:
      Serial.println("Show Humidity");
      showDisplayHumidity();
      break;
    case 3:
      Serial.println("Show Real Time Clock");
      showDisplayTime();
      break;
  }
}
//   pinMode(34,INPUT);
//   pinMode(35,INPUT);
//   pinMode(32,INPUT);
//   int red = digitalRead(34);
//   int yel = digitalRead(35);
//   int grn = digitalRead(32);
//   // Serial.println(red);
//   // Serial.println(yel);
//   // Serial.println(grn);
//   if(red == 0){
//     stateShow = "Temp: ";
//   }
//   if(yel == 0){
//     stateShow = "Humidity: ";
//   }
//   if(grn == 0){
//     stateShow = "Clock: ";
//   }
//   showDisplayTime();

// }

void checkStatusWifi() {
  // Check if WiFi is Connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to : ");
    Serial.print(ssid);
    // Connect ESP32 to network
    WiFi.begin(ssid, pass);

    // Wait until Connection is Complete
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi Connected.");
  }
}

void checkStatusMQTT() {
  //Check if MQTT Server is connected
  if (!mqtt.connected()) {
    Serial.print("Connecting to MQTT Broker.");
    mqtt.connect(clientID, mqttUserName, mqttPass);
    while (!mqtt.connected()) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nMQTT Connected.");
  }
}

void checkStatusNotify() {
  if (!line_connected) {
    Serial.print("Connecting to Line Notify.");
    line_connected = !line_connected;
    while (!line_connected) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nLine Notify Connected.");
    LINE.notify("\nLine Notify Ready.");
  }
}

// LINE MESSAGE API

// bool checkStatusMessageAPI(){
//   HTTPClient http;
//   http.begin("https://api.line.me/v2/bot/message/ping");
//   http.addHeader("Content-Type", "application/json");
//   http.addHeader("Authorization", "Bearer " + String(LINE_TOKEN));

//   int httpResponseCode = http.GET();
//   http.end();
//   if (httpResponseCode == 200){
//     Serial.println("Line Messaging API is up and running");
//     return true;
//   }
//   else{
//     Serial.print("Line Messaging API returned HTTP error code ");
//     Serial.println(httpResponseCode);
//     return false;
//   }
// }

// void checkStatusMessageAPI(){
//   if(!connected){
//     connected = client.connect("api.line.me",httpsPort);
//     Serial.print("Connecting to Line MessageAPI.");
//     while (!connected) {
//       delay(500);
//       Serial.print(".");
//     }
//     Serial.println("\nLine MessageAPI Connected.");
//     // Create the message payload
//     char *message_text = "Enjoy your water habit tracker!!";
//     DynamicJsonDocument payloads(256);
//     payloads["to"] = groupLine;
//     JsonArray messages = payloads.createNestedArray("messages");
//     JsonObject message = messages.createNestedObject();
//     message["type"] = "text";
//     message["text"] = message_text;

//     String payload;
//     serializeJson(payloads, payload);

//     // Create the request header
//     HTTPClient http;
//     http.begin(LINE_API);
//     http.addHeader("Content-Type", "application/json");
//     http.addHeader("Authorization", "Bearer " + String(LINE_TOKEN));

//     // Send the HTTP request and receive the response
//     int httpResponseCode = http.POST(payload);
//     Serial.print("HTTP response code: ");
//     Serial.println(httpResponseCode);

//     String responsePayload = http.getString(); // Get the response payload as a String
//     const char* response = responsePayload.c_str();

//     Serial.println(response);

//     http.end();

//     // Wait for 10 seconds
//     delay(10000);
//   }
// }

void alarm() {
  // if ((hour == 20 && minute == 45) && (second >= 0 && second < 5)) {
  if ((hour == 20 && minute == 45) && (second >= 0 && second < 5)) {
    Serial.println("Alarm");
    playSound();
  }
}

void postDataMQTT() {
  unsigned static int half_second_count = 0;

  //Post MQTT get to ThingSpeak Every 15 seconds
  if (half_second_count >= 30) {
    // Reset Timer
    half_second_count = 0;

    // String dataString = "&field1=" + String(temp);
    // String topicString = "channels/" + String(channelID) + "/publish";
    // mqtt.publish( topicString.c_str(), dataString.c_str());
    // Serial.println("Temp = " + String(temp));
    // digitalWrite(LED_IOT, !digitalRead(LED_IOT));
  }

  // Update Half Seconds every 500 ms
  delay(500);
  half_second_count++;
}

void showDisplayTemp() {
  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor(0, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println("TEMP:");
  // OLED display
  OLED.display();
}
void showDisplayHumidity() {
  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor(0, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println("Humidity:");
  // OLED display
  OLED.display();
}

void showDisplayTime() {
  // This function show Temperature, Humidity and Real time clock

  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor(0, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println("Clock:");

  OLED.setTextColor(WHITE, BLACK);
  if (hour < 10) {
    OLED.print("0");
  }
  OLED.print(hour, DEC);
  OLED.print(":");
  if (minute < 10) {
    OLED.print("0");
  }
  OLED.print(minute, DEC);
  OLED.print(":");
  if (second < 10) {
    OLED.print("0");
  }
  OLED.println(second, DEC);

  OLED.print(dayOfMonth, DEC);
  OLED.print("/");
  if (month < 10) {
    OLED.print("0");
  }
  OLED.print(month, DEC);
  OLED.print("/");
  OLED.println(year, DEC);
  switch (dayOfWeek) {
    case 1:
      OLED.println("SUN");
      break;
    case 2:
      OLED.println("MON");
      break;
    case 3:
      OLED.println("TUE");
      break;
    case 4:
      OLED.println("WED");
      break;
    case 5:
      OLED.println("THU");
      break;
    case 6:
      OLED.println("FRI");
      break;
    case 7:
      OLED.println("SAT");
      break;
  }

  // OLED display
  OLED.display();
}