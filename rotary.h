// Rotary
#define SW_PIN 33
#define DT_PIN 25
#define CLK_PIN 26

// volatile int hourTimer = 0;
volatile int minTimer = 1;
// volatile int secondTimer = 0;
volatile int lastDTstate = LOW;
bool canInterrupt = false;

int rotaryState = 0;
// setDefault and can change
int hotTemp = 28;
int dryAirHumidity = 40;
int minCoolDown = 5;

void handleDTInterrupt() {
  int currentDTstate = digitalRead(DT_PIN);
  if (currentDTstate != lastDTstate) {
    if (digitalRead(CLK_PIN) == currentDTstate) {
      if(rotaryState == 1){
        minTimer++;
      }
      else if(rotaryState == 2){
        hotTemp++;
      }
      else if(rotaryState == 3){
        dryAirHumidity++;
      }
      else if(rotaryState == 4){
        minCoolDown++;
      }
      
    } else {
      if(rotaryState == 1){
        minTimer--;
      }
      else if(rotaryState == 2){
        hotTemp--;
      }
      else if(rotaryState == 3){
        dryAirHumidity--;
      }
      else if(rotaryState == 4){
        minCoolDown--;
      }
      if (minTimer <= 0) {
        minTimer = 1;
      }
      if (minCoolDown <= 0){
        minCoolDown = 1;
      }
    }
  }
  lastDTstate = currentDTstate;
}

// bool isPressSwitchRT() {
//   return digitalRead(SW_PIN) == LOW;
// }
