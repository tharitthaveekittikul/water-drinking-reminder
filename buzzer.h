#define buzzer 13  // pin D13

int notes[] = { 261, 293, 329, 349, 392, 440, 493 };
int durations[] = { 500, 500, 500, 500, 500, 500, 500 };

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