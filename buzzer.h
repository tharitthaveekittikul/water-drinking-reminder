#define buzzer 13  // pin D13

int notes[] = { 261, 293, 329, 349, 392, 440, 493 };
int durations[] = { 500, 500, 500, 500, 500, 500, 500 };

// Non-blocking
// const unsigned long interval = 200;  // interval between notes in milliseconds
// unsigned long previousMillis = 0;    // variable to store the previous time
// int state = 0;                       // state variable for the state machine

// void playSound() {
//   while (true) {
//     switch (state) {
//       case 0:
//         tone(buzzer, 261);  //C
//         break;
//       case 1:
//         tone(buzzer, 293);  //D
//         break;
//       case 2:
//         tone(buzzer, 329);  //E
//         break;
//       case 3:
//         tone(buzzer, 349);  //F
//         break;
//       case 4:
//         tone(buzzer, 392);  //G
//         break;
//     }

//     // Check if it's time to switch to the next note
//     unsigned long currentMillis = millis();
//     if (currentMillis - previousMillis >= interval) {
//       previousMillis = currentMillis;
//       state++;
//     }
//   }
//   if (state > 4) {  // reset the state machine after playing all notes
//     state = 0;
//   }
//   noTone(buzzer);
// }

void playNotes(int pin, int numNotes) {
  static unsigned long previousTime = 0;
  static int noteIndex = 0;

  // Check if it's time to play the next note
  if (millis() - previousTime >= durations[noteIndex]) {
    previousTime = millis();
    tone(pin, notes[noteIndex], durations[noteIndex]);
    noteIndex++;

    // Reset the note index and stop playing the sound after playing all notes
    if (noteIndex >= numNotes) {
      noteIndex = 0;
      noTone(pin);
    }
  }
}