#include <Arduino.h>
#include <SevSeg.h>
SevSeg sevseg;

// put function declarations here:
void playPassiveBuzzer(int buzzerPin, int frequency);
int getTimeToPrint(int timeLeft);
void decrementTime();
bool pollButton();

// initialize variables & constants
const int FIGHT_TIME = 300;
const int BREAK_TIME = 60;
const int TOTAL_ROUNDS = 3;
int state = 0;  // FSM STATE
/* STATE ENCODING:
    0 - WAIT
    1 - FIGHT
    2 - BREAK
    3 - PAUSE FIGHT
    4 - PAUSE BREAK
*/
int buzzerPin = 14;
int buttonPin = 15;
int buttonVal = 0;
int timeLeft = 0;  // time left in the round in milliseconds (fight or break)
int roundNumber = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

void setup() {
  // put your setup code here, to run once:
  // initialize sevseg display
  byte numDigits = 4;
  byte digitPins[] = {10, 11, 12, 13};
  byte segmentPins[] = {9, 2, 3, 5, 6, 8, 7, 4};
  bool resistorsOnSegments = true;
  bool updateWithDelaysIn = true;
  byte hardwareConfig = COMMON_CATHODE;
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins,
               resistorsOnSegments);
  sevseg.setBrightness(90);
  sevseg.setNumber(0, 2);
  sevseg.refreshDisplay();
  // initialize button pin
  pinMode(buttonPin, INPUT);
  // initialize buzzer pin
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (state == 0) {  // WAIT
    if (pollButton()) {
      timeLeft = FIGHT_TIME;  // reset clock
      roundNumber = 1;        // reset round
      state = 1;              // start fight clock
      playPassiveBuzzer(buzzerPin, 587);
    } else {
      state = 0;  // keep waiting
    }

  } else if (state == 1) {  // FIGHT
    if (pollButton()) {
      state = 3;
    } else if (timeLeft <= 0 &&
               roundNumber < TOTAL_ROUNDS) {  // the round has ended
      playPassiveBuzzer(buzzerPin, 494);
      timeLeft = BREAK_TIME;     // reset timer to break time
      state = 2;                 // begin break
    } else if (timeLeft <= 0) {  // fight is over
      state = 0;
    } else {  // round is still on
      // decrement timeLeft
      decrementTime();
    }

  } else if (state == 2) {  // BREAK
    if (pollButton()) {
      state = 4;
    } else if (timeLeft <= 0) {  // the break has ended
      playPassiveBuzzer(buzzerPin, 587);
      timeLeft = FIGHT_TIME;  // reset timer to fight time
      roundNumber++;          // increment the round
      state = 1;              // resume fight
    } else {                  // round is still on
      // decrement timeLeft
      decrementTime();
    }
  } else if (state == 3) {  // PAUSE FIGHT
    if (pollButton()) state = 1;
  } else if (state == 4) {  // PAUSE BREAK
    if (pollButton()) state = 2;
  }
  sevseg.setNumber(getTimeToPrint(timeLeft), 2);
  sevseg.refreshDisplay();
}

// put function definitions here:
void playPassiveBuzzer(int buzzerPin, int frequency) {
  tone(buzzerPin, frequency);
  delay(500);
  noTone(buzzerPin);
  delay(200);
  tone(buzzerPin, frequency);
  delay(500);
  noTone(buzzerPin);
  delay(200);
  tone(buzzerPin, frequency);
  delay(500);
  noTone(buzzerPin);
  delay(200);
}

// get integer to show on sev seg display
int getTimeToPrint(int timeLeft) {
  int minutesLeft = timeLeft / 60;
  int secondsLeft = timeLeft % 60;
  return (minutesLeft * 100 + secondsLeft);
}

// return true if a second has passed and decrement timeLeft
void decrementTime() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    timeLeft -= 1;
  }
}

// if the button is pressed, will wait until released then return true; else do
// nothing and return false
bool pollButton() {
  buttonVal = digitalRead(buttonPin);
  if (buttonVal == HIGH) {
    while (buttonVal == HIGH) {
      buttonVal = digitalRead(buttonPin);
    }
    return true;
  }
  return false;
}
