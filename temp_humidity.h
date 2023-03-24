#include <SPI.h>
#include <Adafruit_BME280.h>

#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5

Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

float temp, humidity;

void getTemperature(){
  temp = bme.readTemperature();
}

void getHumidity(){
  humidity = bme.readHumidity();
}