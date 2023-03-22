#define DS3231_I2C_ADDR 0x68

uint second, minute, hour, dateOfWeek, dayOfMonth, month, year;

byte decToBcd(uint val) {
  return ((val / 10 * 16) + (val % 10));
}

uint bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}

void setTime(uint second, uint minute, uint hour, uint dateOfWeek, uint dayOfMonth, uint month, uint year) {
  Wire.beginTransmission(DS3231_I2C_ADDR);
  // all 8 bits set to 0
  Wire.write(0);
  // set seconds
  Wire.write(decToBcd(second) | 0b10000000);
  // set minutes
  Wire.write(decToBcd(minute));
  // set hours
  Wire.write(decToBcd(hour));
  // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dateOfWeek));
  // set date (1 to 31)
  Wire.write(decToBcd(dayOfMonth));
  // set month
  Wire.write(decToBcd(month));
  // set year (0 to 99)
  Wire.write(decToBcd(year - 2000U));  //2000U -> 2000 format unsigned int
  Wire.endTransmission();
}

void readTime(uint *second, uint *minute, uint *hour, uint *dateOfWeek, uint *dayOfMonth, uint *month, uint *year) {
  Wire.beginTransmission(DS3231_I2C_ADDR);
  Wire.write(0);  // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDR, 7);
  // 0x7f --> 0b 0111 1111
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dateOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read()) + 2000U;
}

void showTime() {
  readTime(&second, &minute, &hour, &dateOfWeek, &dayOfMonth, &month, &year);
  Serial.print(hour, DEC);
  Serial.print(":");
  if (minute < 10) {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second < 10) {
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print(" Day of week: ");
  switch (dateOfWeek) {
    case 1:
      Serial.println("Sunday");
      break;
    case 2:
      Serial.println("Monday");
      break;
    case 3:
      Serial.println("Tuesday");
      break;
    case 4:
      Serial.println("Wednesday");
      break;
    case 5:
      Serial.println("Thursday");
      break;
    case 6:
      Serial.println("Friday");
      break;
    case 7:
      Serial.println("Saturday");
      break;
  }

  // When 20:45
  // if (hour == 20 && minute == 45) {
  //   Serial.print("20:45");
  // }
}