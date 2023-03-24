// Rotary
#define SW_PIN 33
#define DT_PIN 25
#define CLK_PIN 26

// volatile int hourTimer = 0;
volatile int minTimer = 1;
// volatile int secondTimer = 0;
volatile int lastDTstate = LOW;
bool canInterrupt = false;

void handleDTInterrupt() {
  int currentDTstate = digitalRead(DT_PIN);
  if (currentDTstate != lastDTstate) {
    if (digitalRead(CLK_PIN) == currentDTstate) {
      // secondTimer++;
      // if (secondTimer >= 60) {
      //   secondTimer = 0;
      //   minTimer++;
      //   if (minTimer >= 60) {
      //     minTimer = 0;
      //     if (hourTimer >= 24) {
      //       hourTimer = 0;
      //     }
      //   }
      // }
      minTimer++;
    } else {
      // secondTimer--;
      // if (secondTimer < 0) {
      //   secondTimer = 59;
      //   minTimer--;
      //   if (minTimer < 0) {
      //     minTimer = 59;
      //     hourTimer--;
      //     if (hourTimer < 0) {
      //       hour = 23;
      //     }
      //   }
      // }
      minTimer--;
      if (minTimer < 0) {
        minTimer = 0;
      }
    }
  }
  lastDTstate = currentDTstate;
}

bool isPressSwitchRT() {
  return digitalRead(SW_PIN) == LOW;
}
