#define buzzer 13  // pin D13

int notes[] = { 261, 293, 329, 349, 392 };
int currentNote = 0;
// unsigned long prevMillis = 0;
// const long interval = 200;

void playSound() {
  int currentNote = 0;
  unsigned long lastMillis = millis();
  unsigned long interval = 200;
  unsigned long currentMillis = millis();
  // Serial.print("Check : ");
  // Serial.println(currentNote);
  
  while(currentNote<=4){
    lastMillis = millis();
    // Serial.print(currentMillis);
    // Serial.print(" - ");
    // Serial.print(lastMillis);
    // Serial.print(" = ");
    // Serial.println(lastMillis - currentMillis);
    if(lastMillis - currentMillis <= interval){
      tone(buzzer, notes[currentNote]);
      // Serial.println("B");
    }      
    if(lastMillis - currentMillis > interval){
      currentNote++;
      // Serial.print("Check : ");
      Serial.println(currentNote);
      currentMillis = millis();
      lastMillis = millis();
      noTone(buzzer);
    }
  }
  noTone(buzzer);

  // delay(5000);
}

  // unsigned long currentMillis = millis();
  // if (currentMillis - prevMillis >= interval) {
  //   prevMillis = currentMillis;
  //   if (digitalRead(buzzer) == LOW) {
  //     // currentNote = (currentNote + 1) % (sizeof(notes) / sizeof(notes[0]));
  //     tone(buzzer, notes[currentNote]);
  //     currentNote++;
  //     if (currentNote == 5) {
  //       currentNote = 0;
  //     }
  //   } else {
  //     noTone(buzzer);
  //   }
  //   noTone(buzzer);
  // }

  // // Sounds the buzzer at the frequency relative to the note C in Hz
  // tone(buzzer, 261);
  // // Waits some time to turn off
  // delay(200);
  // //Turns the buzzer off
  // noTone(buzzer);
  // // Sounds the buzzer at the frequency relative to the note D in Hz
  // tone(buzzer, 293);
  // delay(200);
  // noTone(buzzer);
  // // Sounds the buzzer at the frequency relative to the note E in Hz
  // tone(buzzer, 329);
  // delay(200);
  // noTone(buzzer);
  // // Sounds the buzzer at the frequency relative to the note F in Hz
  // tone(buzzer, 349);
  // delay(200);
  // noTone(buzzer);
  // // Sounds the buzzer at the frequency relative to the note G in Hz
  // tone(buzzer, 392);
  // delay(200);
  // noTone(buzzer);
// }