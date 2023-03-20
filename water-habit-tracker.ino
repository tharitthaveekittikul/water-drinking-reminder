#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include "mqtt_secrets.h"
#include "secrets.h"

// Thingspeak MQTT & Wifi Setup
const char *ssid = SECRET_WIFI_SSID;
const char *pass = SECRET_WIFI_PASS;
const char *server = SECRET_SERVER;
const char *channelID = SECRET_CHANNELID;
const char *mqttUserName = SECRET_MQTT_USERNAME;
const char *mqttPass = SECRET_MQTT_PASSWORD;
const char *clientID = SECRET_MQTT_CLIENT_ID;

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
  mqtt.setServer(server,1883);

  // OLED begin at addr: 0x3C 
  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
  } else {
    Serial.println("OLED Start Work !!!");
  }
}

void loop() {
  checkStatusWifi();
  checkStatusMQTT();
  showDisplayOLED();
  delay(500);
}

void checkStatusWifi(){
  // Check if WiFi is Connected
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Connecting to : ");
    Serial.print(ssid);
    // Connect ESP32 to network
    WiFi.begin(ssid, pass);

    // Wait until Connection is Complete
    while( WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi Connected.");
  }
}

void checkStatusMQTT(){
  //Check if MQTT Server is connected
  if(!mqtt.connected()){
    Serial.print("Connecting to MQTT Broker.");
    mqtt.connect(clientID, mqttUserName, mqttPass);
    while(!mqtt.connected()){
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nMQTT Connected.");
  }
}

void postDataMQTT(){
  unsigned static int half_second_count = 0;

  //Post MQTT get to ThingSpeak Every 15 seconds
  if(half_second_count >= 30){
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

void showDisplayOLED(){
  // This function show Temperature, Humidity and Real time clock
  
  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor(0, 0);
  // Set text size
  OLED.setTextSize(3);
  // Show text
  OLED.println("TEMP:");
  // OLED display
  OLED.display();
}