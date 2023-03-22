# Water Habit Tracker

The project made to remind to drink water.
This project for Embedded Systems Design Laboratory.

## Requirements

- ESP-WROOM-32
- BME280 (SPI) for Temperature, Humidity
- DS3231 (I2C) for Real Time Clock
- SSD1306 (I2C) for OLED

## Schema

![alt text](./image/schematic.jpg)

## Setup

1. Connect SSD1306 to the ESP32. <br>
   VCC - 3V3 <br>
   GND - GND <br>
   SCL - D22 (SCL) <br>
   SDA - D21 (SDA) <br>

2. Connect DS3231 to the ESP32. <br>
   VCC - 3V3 <br>
   GND - GND <br>
   SCL - D22 (SCL) <br>
   SDA - D21 (SDA) <br>

## Installation

1. Install library "Adafruit SSD1306 by Adafruit" for OLED

2. Install library "Adafruit GFX Library by Adafruit" for OLED

3. Install library "PubSubClient by Nick O'Leary" for MQTT Client

4. Install library "ArduinoJson by Benoit Blanchon" for using JSON serialization

5. Install library "TridentTD_Linenotify by TridentTD" for using LINE Notify

## Usage

1. Setup Thingspeak
   [Link Guide](https://app.tango.us/app/workflow/Setup-Water-Habit-Tracker-with-Thingspeak-using-MQTT-d4d8b49085ea492ab4a4902507486bd5)
2. Setup Line Message API service
   [Link Guide](https://app.tango.us/app/workflow/Line-Message-API-Bot-d784c38c28724f939111aaeadc92980f)

3. Setup Line Notify [Link Guide](https://app.tango.us/app/workflow/Setup-Line-Notify-376edee9e87d41818e2ef3297e6651a5)

## Troubleshooting

- Get text from line message API service but cannot send it to ESP32 (I try both MQTT and ThingHTTP)
