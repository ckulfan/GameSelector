#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "FS.h"
#include "SPIFFS.h"
#include <vector>  // Include the vector library for dynamic arrays

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true
#define MAX_GAMES 260
#define MAX_BEST 13  // 13 to account for indices 1 to 12

// Structure to hold game data
struct Game {
  String title;
  int minPlayers;
  int maxPlayers;
  String bestPlayers;  // Best player count can be a single number or range
};

// Array of files on the device
extern String* gameFiles;
extern int gameFileCount;

void addGameToLookup(int gameIndex, String bestPlayers);
void initializeGameData();
String* getGameFiles();
void loadGames(String filename);
void getRecommendedGames(int bestCount, String results[], int resultsCount);
void dumpGameData();
String* getFilesInRoot(int &fileCount);


#endif // GAMEDATA_H