#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include "graphics.h"
#include "gamedata.h"

void setup();
void loop();
void serialEvent();
void processSerialCommand();

#endif // MAIN_H