#include "graphics.h"

int playerCount = 4;

// center dot radius to use for offset as well
int centerDotRadius = 3;

// Initialize the TFT display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// tolerance for the angle check 
float tolerance = 0.005;

// menu variables
String topMenuItems[] = {"time", "Recommend", "Player #"};
String menuItems[] = {"2 Players", "3 Players", "4 Players", "5 Players", "6 Players", "7 Players", "8 Players"};
Menu tMenu = {topMenu, topMenuItems, 2, 0, false};
Menu playerCountMenu = {playerMenu, menuItems, 7, 2, false}; // default to 4 players
bool recommendedDisplayed = false;

// Initialize the display
void initializeDisplay() {
  tft.initR(INITR_144GREENTAB);  // For Adafruit 1.44" TFT
  tft.setRotation(0);            // Set rotation to match the desired orientation 2=upside down
  tft.fillScreen(ST77XX_BLACK);  // Clear screen to black
}

// reset the display
void resetDisplay() {
  tft.fillScreen(ST77XX_BLACK);
  drawCenter();
  drawClockNumbers(playerCount);
  drawHand(0.0, ST77XX_WHITE);
}

// function to return the menu displayed state
bool isMenuDisplayed() {
  return tMenu.menuDisplayed || playerCountMenu.menuDisplayed;
}

// Increment the selected item in the menu
void incrementSelectedItem(Menu &activeMenu) {
  activeMenu.selectedItem++;
  if (activeMenu.selectedItem >= activeMenu.menuSize) {
    activeMenu.selectedItem = 0;
  }
}

// Get the selected item in the menu
int getPlayerCount() {
  return playerCountMenu.selectedItem + 2;
}

// Set the player count
void setPlayerCount(int count) {
  playerCount = count;
}

// Function to update the spinner hand
void spinWheel() {
  float angle = 0.0;
  float distance = 0.0;

  int rotations = (int)random(1, 4); // 1-3 times around
  int player = random(1, getPlayerCount()+1); // random player to land on

  // reset the display to clear old results
  resetDisplay();

  float totalDistance = toRadians(rotations * 360 + (360 / playerCount) * (player-1));
  float angleOffset = toRadians(360.0 / (playerCount * 4));
  
  Serial.println("Rotations: " + String(rotations));
  Serial.println("Player: " + String(player));
  Serial.println("Total Distance: " + String(toDegrees(totalDistance)));
  Serial.println("Angle Offset: " + String(toDegrees(angleOffset)));

  while ((totalDistance-distance) > tolerance) {
    // Erase the previous hand by redrawing it in black
    drawHand(angle, ST77XX_BLACK);

    // Update the angle of the spinner hand
    angle += angleOffset;
    if (angle >= 2*PI) {
      angle -= 2*PI;
    }

    // Draw the updated spinner hand
    drawHand(angle, ST77XX_WHITE);

    // Update the total distance remaining
    distance += angleOffset;

    int delayTime = map(distance, 0, totalDistance, 50, 200);
    delayTime = constrain(delayTime, 50, 200);
    delay(delayTime);
  }
}

// Function to draw the clock hand spinner
void drawHand(float rotation, uint16_t color) {
  float angle = rotation - PI / 2;
  int centerX = WIDTH / 2;
  int centerY = HEIGHT / 2;
  int radius = 35;  // Length of the hand

  // Calculate the starting point as the offset to avoid overwriting the dot
  int startX = centerX + (int)(centerDotRadius * 1.5 * cos(angle));
  int startY = centerY + (int)(centerDotRadius * 1.5 * sin(angle));

  // Calculate the end point of the hand (tip of the hand)
  int handX = centerX + radius * cos(angle);
  int handY = centerY + radius * sin(angle);

  // for some reason the angle is off when displaying at the 90 increments, so adjust by 1 pixel
  if( abs(handX - centerX) < 2 ) {
    handX = centerX;
  }
  if( abs(handY - centerY) < 2 ) {
    handY = centerY;
  }

  // Draw the clock hand (from start point to the end point)
  tft.drawLine(startX, startY, handX, handY, color);

  // Calculate the coordinates for the arrowhead (a triangle at the end)
  int arrowWidth = 3;  // Width of the arrowhead at the base
  int arrowLength = 5; // Length of the arrowhead

  // Calculate the two points of the arrow base
  int arrowBaseX1 = handX + arrowWidth * cos(angle - PI / 2);
  int arrowBaseY1 = handY + arrowWidth * sin(angle - PI / 2);

  int arrowBaseX2 = handX + arrowWidth * cos(angle + PI / 2);
  int arrowBaseY2 = handY + arrowWidth * sin(angle + PI / 2);

  // Calculate the arrow tip (slightly beyond the hand's end point)
  int arrowTipX = handX + arrowLength * cos(angle);
  int arrowTipY = handY + arrowLength * sin(angle);
  //tft.drawLine(handX, handY, arrowTipX, arrowTipY, ST77XX_RED);

  // Draw the arrowhead as a filled triangle
  tft.fillTriangle(handX, handY, arrowBaseX1, arrowBaseY1, arrowBaseX2, arrowBaseY2, color);
  tft.fillTriangle(arrowTipX, arrowTipY, arrowBaseX1, arrowBaseY1, arrowBaseX2, arrowBaseY2, color);
}

void drawCenter() {
  int centerX = WIDTH / 2;
  int centerY = HEIGHT / 2;
  
  // Optionally, draw a small circle at the center
  tft.fillCircle(centerX, centerY, centerDotRadius, ST77XX_RED);
}

// Function to draw clock numbers at fixed positions
void drawClockNumbers(int count) {
  int centerX = WIDTH / 2 - 5;
  int centerY = HEIGHT / 2 - 5;
  int textRadius = 50;  // Distance from the center to the numbers

  // Set text size and color
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);

  // The numbers drawn depend on the input count number of people
  if (count == 2 ) {
    tft.setCursor(centerX - 3, centerY - textRadius);  // Adjust cursor position
    tft.print("1");
    tft.setCursor(centerX - 3, centerY + textRadius - 5);  // Adjust cursor position
    tft.print("2");
  } else { 
    float txtAngle = 0.0;
    float angleInc = 360.0 / count;
    for (int idx=0; idx<count; idx++) {
      float rads = toRadians(txtAngle);
      tft.setCursor(centerX + sin(rads)*(textRadius), centerY - cos(rads)*(textRadius));
      tft.print(idx+1);
      txtAngle += angleInc;
    }
  }
}

// function to handle button presses
void doButtonPress(Button *button) {
  // Read the state of the button
  bool reading = digitalRead(button->pin);

  // Check if the button state has changed
  if (reading != button->lastState) {
    // Reset the debounce timer
    button->lastDebounceTime = millis();
  }

  // Check if enough time has passed to consider it a stable button press
  if ((millis() - button->lastDebounceTime) > DEBOUNCE_DELAY) {
    // If the button state has changed
    if (reading != button->state) {
      button->state = reading;
     if(button->state == HIGH && button->type == playerCountSelect) {
        if ( tMenu.menuDisplayed ) {
          // if the menu is displayed, we need to check which item is selected
          if (tMenu.selectedItem == 0) {
            // display the recommended games
            String gameResults[RECOMMENDED_COUNT];
            getRecommendedGames(playerCount, gameResults, RECOMMENDED_COUNT);
            displayRecommendedGames(gameResults, RECOMMENDED_COUNT);
            tMenu.menuDisplayed = false;
          } else if (tMenu.selectedItem == 1) {
            // display the player count menu
            tMenu.menuDisplayed = false;
            drawMenu(playerCountMenu);
          }
        } else if (playerCountMenu.menuDisplayed) {
          // if the player count menu is displayed, we need to set the player count
          playerCount = (int)playerCountMenu.selectedItem + 2;
          playerCountMenu.menuDisplayed = false;
          resetDisplay();
        } else if (recommendedDisplayed) {
          // if the recommended games are displayed, we need to reset the display
          resetDisplay();
          recommendedDisplayed = false;
        } else {
          spinWheel();
        }
      } else if (button->state == HIGH && button->type == menuSelect) {
        if ( tMenu.menuDisplayed ) {
          // if the menu is displayed, we need to increment the selected item
          incrementSelectedItem(tMenu);
          drawMenu(tMenu);
        } else if (playerCountMenu.menuDisplayed) {
          // if the player count menu is displayed, we need to increment the selected item
          incrementSelectedItem(playerCountMenu);
          drawMenu(playerCountMenu);
        } else {
          // if the menu is not displayed, we need to display the top menu
          drawMenu(tMenu);
        }
      }
    }
  }
  button->lastState = reading;
}

// draw the menu when it is requested to be displayed
void drawMenu(Menu &activeMenu) {
  int line_space = 17;
  int text_size = 2;
  // if the menu isn't currently displayed, we want to draw the entire menu. Otherwise just adjust the selected item
  if (activeMenu.menuDisplayed) {
    // Draw the selected item
    tft.setTextSize(text_size); // Text size
    // reset the prior item to white text on black background
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK); // Black text on white background
    int priorItem = activeMenu.selectedItem - 1;
    if (priorItem < 0) {
      priorItem = activeMenu.menuSize - 1;
    }
    tft.setCursor(X_INDENT, Y_INDENT + priorItem * line_space); // Position the selected item
    tft.print(activeMenu.menuItems[priorItem]);

    // now print the highlighted item
    tft.setCursor(X_INDENT, Y_INDENT + activeMenu.selectedItem * line_space); // Position the selected item
    tft.setTextColor(ST7735_BLACK, ST7735_WHITE); // White text on black background
    tft.print(activeMenu.menuItems[activeMenu.selectedItem]);
  } else {
    tft.fillScreen(ST7735_BLACK);  // Clear the screen
    // Draw each menu item
    for (int i = 0; i < activeMenu.menuSize; i++) {
      tft.setTextSize(text_size); // Text size
      tft.setCursor(X_INDENT, Y_INDENT + i * line_space); // Position each item

      if (i == activeMenu.selectedItem) {
        // Highlight the selected item
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE); // White text on black background
      } else {
        // Normal menu item
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK); // Black text on white background
      }

      tft.print(activeMenu.menuItems[i]);
    }
    activeMenu.menuDisplayed = true;
   }
}

// Function to display the recommended games with line wrapping and spacing
void displayRecommendedGames(String results[], int count) {
  int idx = 0;
  int line = 0;        // Track vertical line position
  int wrapLimit = 20;  // Character limit for wrapping
  int spacePos;
  int spacing = 0;
  int line_space = 12;

  // Clear the screen
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(X_INDENT, Y_INDENT);
  tft.print("Games:");
  tft.setTextSize(1);
  line++;  // Move to the next line
  spacing += 6;

  // Loop through the results array
  while (idx < count) {
    String result = results[idx];
    int startPos = 0;
    int printedChars = 0;

    // Handle wrapping if the result is longer than wrapLimit
    while (startPos < result.length()) {
      // Find the length to wrap at
      if ((int)result.length() > startPos + wrapLimit) {
        int endPos = startPos + wrapLimit;

        // Look for the last space within the wrap limit to avoid cutting words
        spacePos = result.lastIndexOf(' ', endPos - 1);
        if (spacePos == -1 || spacePos < startPos) {
          spacePos = endPos;  // No space found, split at wrapLimit
        }

        // Extract the substring to print
        String subStr = result.substring(startPos, spacePos);

        // Print the substring
        tft.setCursor(X_INDENT, Y_INDENT + line * line_space + spacing);  // Half-line spacing (12 px per line)
        tft.print(subStr);

        // Update the printed character count
        printedChars += subStr.length();

        // Move the startPos past the printed part
        startPos = spacePos + 1;  // Skip space after the word
        line++;                   // Move to the next line for continuation
      } else {
         // Print the substring
        tft.setCursor(X_INDENT, Y_INDENT + line * line_space + spacing);  // Half-line spacing (12 px per line)
        tft.print(result.substring(startPos));
        startPos = result.length();  // Exit the loop
      }
    }

    // Add extra half-line of spacing after each game
    line++;
    idx++;
    // space between items
    spacing += 4;
  }

  recommendedDisplayed = true;
}

// function to convert an angle to radians for use in trig functions
float toRadians(float degrees) {
  return  degrees * (PI / 180.0);
}

// function to convert an angle to degrees for use in trig functions
float toDegrees(float radians) {
  return radians * (180.0 / PI);
}
