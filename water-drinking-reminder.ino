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
#include "temp_humidity.h"
#include "rotary.h"

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
Button rotaryButton(SW_PIN, 25, true, true);
// default state 1 --> show Temperature
int buttonState = 1;

// Variables for using default
int countDrinkPerDay = 0;

// Thingspeak MQTT & Wifi Setup
const char *ssid = SECRET_WIFI_SSID;
const char *pass = SECRET_WIFI_PASS;
// const char *server = SECRET_THINGSPEAK_SERVER;
const char *server = "mqtt3.thingspeak.com";
const char *channelID = SECRET_CHANNELID;
const char *mqttUserName = SECRET_MQTT_USERNAME;
const char *mqttPass = SECRET_MQTT_PASSWORD;
const char *clientID = SECRET_MQTT_CLIENT_ID;
const char *thingSpeakAPI = THING_SPEAK_API;

// Line Notify
const char *NOTIFY_TOKEN = LINE_NOTIFY_TOKEN;
bool line_connected = false;

// Waiting after notify
bool isCoolDownTemp = false;
bool isCoolDownHumidity = false;
// uint tempTime, humidityTime;

// MQTT POST
unsigned long previousMillis = 0;            // variable to store the previous millis() value
const unsigned long interval = 500;          // interval at which to update the half second count (in ms)
const unsigned int publishInterval = 15000;  // interval at which to publish data (in ms)
unsigned long lastPublishTime = 0;           // variable to store the last time data was published
int round_mqtt = 1;

WiFiClient client;
PubSubClient mqtt(client);
HTTPClient http;

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
  while (!Serial)
    ;

  // hostname/IP server, port MQTT
  mqtt.setServer(server, 1883);

  // OLED begin at addr: 0x3C
  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
  } else {
    Serial.println("OLED Start Work !!!");
  }

  // ****** BME280 Setup ******
  SPI.begin();
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  // ************************

  LINE.setToken(NOTIFY_TOKEN);

  // ****** RTC Setup ******
  Wire.begin(I2C_SDA, I2C_SCL);
  // seconds, minutes, hours, day, date, month, year
  // day of week (1=Sunday, 7=Saturday)
  // set date (1 to 31)
  // set year (2000+ (0-99)) ex: 2023
  // rtc.begin();
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // rtcInit();
  // ************************


  // ****** Pinmode Setup ******
  pinMode(buzzer, OUTPUT);
  redButton.begin();
  yellowButton.begin();
  greenButton.begin();
  rotaryButton.begin();
  // pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(CLK_PIN, INPUT_PULLUP);
  // ***************************

  // interrupt for DT PIN
  attachInterrupt(digitalPinToInterrupt(DT_PIN), handleDTInterrupt, CHANGE);
}

void loop() {
  checkStatusWifi();
  rtcInit();
  checkStatusMQTT();
  checkStatusNotify();
  readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  // increase 1 for correct dayOfWeek
  // if (firstTime) {
  //   if (dayOfWeek == 7) {
  //     dayOfWeek = 1;
  //   }
  //   dayOfWeek++;
  //   setTime(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
  //   firstTime = false;
  // }
  // showTime();  // show in Serial Monitor
  getTemperature();
  getHumidity();
  readButton();
  postDataMQTT();
  resetCoolDown();
  if (completeOneDay()) {
    countDrinkPerDay = 0;
  }
  // delay(1000);
}

void readButton() {
  greenButton.read();
  yellowButton.read();
  redButton.read();
  rotaryButton.read();
  if (greenButton.wasPressed()) {
    buttonState = 1;
    rotaryState = 0;
    canInterrupt = false;
  }
  if (yellowButton.wasPressed()) {
    buttonState = 2;
    rotaryState = 0;
    canInterrupt = false;
  }
  if (redButton.wasPressed()) {
    buttonState = 3;
    rotaryState = 0;
    canInterrupt = false;
  }
  if (rotaryButton.wasPressed()) {
    buttonState = 4;
    rotaryState++;
    if (rotaryState > 4) {
      rotaryState = 1;
    }
    canInterrupt = true;
  }
  switch (buttonState) {
    case 1:
      // Serial.println("Show Temperature");
      showDisplayTemp();
      break;
    case 2:
      // Serial.println("Show Humidity");
      showDisplayHumidity();
      break;
    case 3:
      // Serial.println("Show Real Time Clock");
      showDisplayTime();
      break;
    case 4:
      // Serial.println("Show Set Timer");
      switch (rotaryState) {
        case 1:
          showDisplaySetTimer();
          break;
        case 2:
          showDisplaySetTempDefault();
          break;
        case 3:
          showDisplaySetHumidityDefault();
          break;
        case 4:
          showDisplaySetCoolDown();
          break;
      }
      // showDisplaySetTimer();
      break;
  }
}

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

void alarm() {
  if ((hour == 1 && minute == 31) && (second >= 0 && second < 5)) {
    Serial.println("Alarm");
    playNotes(buzzer, 7);
  }
}

bool completeOneDay() {
  if ((hour == 0 && minute == 0)) {
    LINE.notify("\nวันนี้ดื่มน้ำไปทั้งหมด " + String(countDrinkPerDay) + "ครั้ง");
    Serial.println("COMPLETE-ONE-DAY");
    return true;
  }
  return false;
}

void notifyLine(String type, float value) {
  if (type == "Temp" && !isCoolDownTemp) {
    if (value >= hotTemp) {
      countDrinkPerDay = countDrinkPerDay + 1;
      playNotes(buzzer, 7);
      LINE.notify("\nTemperature : " + String(value, 2) + " *C\n" + "ดื่มน้ำหน่อยอากาศร้อนมาก!!");
      isCoolDownTemp = true;
      // CountDown 30 minutes --> change state time to false
      showTime();
    }
  } else if (type == "Humidity" && !isCoolDownHumidity) {
    if (value < dryAirHumidity) {
      countDrinkPerDay = countDrinkPerDay + 1;
      playNotes(buzzer, 7);
      LINE.notify("\nRelative Humidity : " + String(value, 2) + " %\n" + "ดื่มน้ำหน่อยอากาศแห้งเดียวเจ็บคอ");
      isCoolDownHumidity = true;
      showTime();
    }
  }
  // else if Time also
  else if (type == "Timer") {
    readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
    if (minute == prevMin + value) {
      prevMin = minute;
      countDrinkPerDay = countDrinkPerDay + 1;
      playNotes(buzzer, 7);
      LINE.notify("\nครบ " + String(minTimer) + " นาทีแล้วดื่มน้ำหน่อยนะ");
      showTime();
    }
  }
}

void resetCoolDown() {
  static uint32_t lastPrintTime = 0;
  // timeIntervalReset can Change
  const unsigned int timeIntervalReset = minCoolDown * 60000;  // 5 min --> 1 min = 60000
  uint32_t currentTime = millis();
  if (currentTime - lastPrintTime >= timeIntervalReset) {
    isCoolDownTemp = false;
    isCoolDownHumidity = false;

    // Update the last print time
    lastPrintTime = currentTime;
  }
}

void postDataMQTT() {
  notifyLine("Temp", temp);
  notifyLine("Humidity", humidity);
  notifyLine("Timer", minTimer);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // half_second_count++;
  }

  // Publish data every `publishInterval` milliseconds
  if (currentMillis - lastPublishTime >= publishInterval) {
    lastPublishTime = currentMillis;

    String topicString = "channels/" + String(channelID) + "/publish";
    String dataString;
    if (round_mqtt == 1) {
      dataString = "&field1=" + String(temp);
      mqtt.publish(topicString.c_str(), dataString.c_str());
    } else if (round_mqtt == 2) {
      dataString = "&field2=" + String(humidity);
      mqtt.publish(topicString.c_str(), dataString.c_str());
    } else {
      dataString = "&field3=" + String(countDrinkPerDay);
      mqtt.publish(topicString.c_str(), dataString.c_str());
      round_mqtt = 0;
    }
    round_mqtt++;
  }
  mqtt.loop();
}

void showDisplayTemp() {
  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor((SCREEN_WIDTH - 8 * 8) / 2, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println(" TEMP ");

  OLED.println("\n");
  OLED.setTextColor(WHITE, BLACK);
  OLED.setTextSize(2);
  OLED.setCursor((SCREEN_WIDTH - 9 * 9) / 2, (SCREEN_HEIGHT) / 2);
  OLED.print(temp);
  OLED.println(" *C");

  // OLED display
  OLED.display();
}
void showDisplayHumidity() {
  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor((SCREEN_WIDTH - 7 * 7) / 2, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println(" RH ");

  OLED.println();
  OLED.setTextColor(WHITE, BLACK);
  OLED.setTextSize(3);
  OLED.print(humidity);
  OLED.println(" %");
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
  OLED.setCursor((SCREEN_WIDTH - 10 * 10) / 2, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println(" Clock ");

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

void showDisplaySetTimer() {
  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor(0, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println("Set Timer");
  OLED.setTextColor(WHITE, BLACK);
  OLED.println("\n");
  OLED.setCursor(0, (SCREEN_HEIGHT) / 2);
  OLED.setTextSize(3);


  // if (hourTimer < 10) {
  //   OLED.print("0");
  // }
  // OLED.print(hourTimer, DEC);
  // OLED.print(":");
  if (minTimer < 10) {
    OLED.print("0");
  }
  OLED.print(minTimer, DEC);
  OLED.println(" min");
  // OLED.print(":");
  // if (secondTimer < 10) {
  //   OLED.print("0");
  // }
  // OLED.println(secondTimer, DEC);

  // OLED display
  OLED.display();
}

void showDisplaySetTempDefault() {
  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor(0, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println("Set Temp");
  OLED.setTextColor(WHITE, BLACK);
  OLED.println("\n");
  OLED.setCursor((SCREEN_WIDTH - 6 * 6) / 2, (SCREEN_HEIGHT) / 2);
  OLED.setTextSize(3);


  // if (hourTimer < 10) {
  //   OLED.print("0");
  // }
  // OLED.print(hourTimer, DEC);
  // OLED.print(":");
  if (hotTemp < 10) {
    OLED.print("0");
  }
  OLED.setTextSize(2);
  OLED.print(hotTemp, DEC);
  OLED.println(" *C");
  // OLED.print(":");
  // if (secondTimer < 10) {
  //   OLED.print("0");
  // }
  // OLED.println(secondTimer, DEC);

  // OLED display
  OLED.display();
}

void showDisplaySetHumidityDefault() {
  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor(0, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println("Set Humid");
  OLED.setTextColor(WHITE, BLACK);
  OLED.println("\n");
  OLED.setCursor((SCREEN_WIDTH - 6 * 6) / 2, (SCREEN_HEIGHT) / 2);
  OLED.setTextSize(3);

  OLED.print(dryAirHumidity, DEC);
  OLED.println(" %");
  // OLED.print(":");
  // if (secondTimer < 10) {
  //   OLED.print("0");
  // }
  // OLED.println(secondTimer, DEC);

  // OLED display
  OLED.display();
}

void showDisplaySetCoolDown() {
  // Clear Screen
  OLED.clearDisplay();
  // Define textColor BLACK and Background WHITE
  OLED.setTextColor(BLACK, WHITE);
  // Define position x,y
  OLED.setCursor(0, 0);
  // Set text size
  OLED.setTextSize(2);
  // Show text
  OLED.println("Cooldown");
  OLED.setTextColor(WHITE, BLACK);
  OLED.println("\n");
  OLED.setCursor(0, (SCREEN_HEIGHT) / 2);
  OLED.setTextSize(3);

  if (minCoolDown < 10) {
    OLED.print("0");
  }
  OLED.print(minCoolDown, DEC);
  OLED.println(" min");
  // OLED.print(":");
  // if (secondTimer < 10) {
  //   OLED.print("0");
  // }
  // OLED.println(secondTimer, DEC);

  // OLED display
  OLED.display();
}