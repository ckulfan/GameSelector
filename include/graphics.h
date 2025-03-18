#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "gamedata.h"

#define RECOMMENDED_COUNT 3

// Define TFT display connections
#define TFT_CS     10  // reset pin
#define TFT_RST    9  // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC     8 // A0 pin
#define BTN1_PIN    3
#define BTN2_PIN    2
#define DEBOUNCE_DELAY 50  // Debounce delay in milliseconds

// override the height and width values since the library is not correct
#define HEIGHT 128
#define WIDTH 128

// define indents for menus and text display
#define X_INDENT 5
#define Y_INDENT 10

// define a structure of buttons to know which one is pressed
enum ButtonType {
  playerCountSelect,
  menuSelect
};
struct Button {
  ButtonType type;
  int pin;
  bool state;
  bool lastState;
  unsigned long lastDebounceTime;
};

enum MenuType {
  topMenu,
  playerMenu,
  configMenu
};

// define a Menu structure
struct Menu {
  MenuType type;
  String *menuItems;
  int menuSize;
  int selectedItem;
  bool menuDisplayed;
};


void initializeDisplay();
void resetDisplay();
void spinWheel();
void drawHand(float angle, uint16_t color);
void drawCenter();
void drawMenu(Menu &activeMenu);
void drawClockNumbers(int count);
float toRadians(float degrees);
float toDegrees(float radians);
bool isMenuDisplayed();
int getPlayerCount();
void setPlayerCount(int count); // only used for serial input
void displayRecommendedGames(String results[], int count);
void doButtonPress(Button *button);

#endif //GRAPHICS_H