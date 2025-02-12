#include "main.h"

// player variables
int randNumber = 0;

// Variables for debouncing
Button selectButton = {playerCountSelect, BTN1_PIN, LOW, LOW, 0};
Button menuButton = {menuSelect, BTN2_PIN, LOW, LOW, 0};

// serial parameters
String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup() {
  Serial.begin(115200); 
  for (uint8_t t = 5; t > 0; t--) {
    Serial.println("[SETUP] WAIT");
    Serial.flush();
    delay(200);
  }

  // set up the buttons
  pinMode(selectButton.pin, INPUT);
  pinMode(menuButton.pin, INPUT);

  // configure the display
  initializeDisplay();
  resetDisplay();

  // initialize the pseudo-random number generator
  randomSeed(analogRead(0));

  // Load game data from the CSV file
  loadGames();
}

void loop() {
  randNumber = random(1, getPlayerCount()+1); // keep the random function in the loop to get a new number each time

  // Handle the press of the select button. 
  doButtonPress(&selectButton);

  // Handle the press of the menu button
  doButtonPress(&menuButton);

  // check for serial commands outside the loop so we can control even when not connected to wifi
  if (Serial.available() > 0) {
    serialEvent();
  }
  // print the string when a newline arrives:
  if (stringComplete) {
    processSerialCommand();
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

/**
 * Method to handle parsing and acting on any commands received via serial interface
 */
void processSerialCommand() {
  if (inputString.length() > 0) {
    if (inputString[0] == 'p' || inputString[0] == 'P') {
      if (inputString.endsWith("\n") || inputString.endsWith("\r")) {
        inputString.remove(inputString.length() - 1);
      }
      inputString.remove(0, 2);
      int val = inputString.toInt();
      setPlayerCount(val);
      Serial.println("Setting player count to: " + val);
      resetDisplay();
    } else if (inputString[0] == 'd' || inputString[0] == 'd') {
      dumpGameData();
    } else if (inputString[0] == 'g' || inputString[0] == 'G') {
      int bestCount = getPlayerCount();
      String results[RECOMMENDED_COUNT];
      getRecommendedGames(bestCount, results, RECOMMENDED_COUNT);
      displayRecommendedGames(results, RECOMMENDED_COUNT);
    } else if (inputString[0] == '?') {
      Serial.println("Valid commands and usage are:");
      Serial.println("  g - search for games with the current player count");
      Serial.println("  p xx - change the player count to xx");
    } else {
      Serial.print("Invalid command: ");
      Serial.println(inputString);
    }
  }
}
