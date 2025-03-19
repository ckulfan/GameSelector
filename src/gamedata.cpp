#include "gameData.h"

Game games[MAX_GAMES];  // Array to store game data (max 20 games)
int gameCount = 0;
int gameFileCount = 0;
String* gameFiles = nullptr;

// Vector to hold indices of games for each "Best" player count (1 to 12 players)
std::vector<int> bestPlayerLookup[MAX_BEST];  // 13 to account for indices 1 to 12

// Function to parse the "Best" field and handle a list of comma-separated values like "3, 4, 6"
void addGameToLookup(int gameIndex, String bestPlayers) {
  bestPlayers.trim();  // Remove any leading/trailing whitespace
  // Remove quotes if the first and last characters are quotes and there's a comma in the list
  if (bestPlayers.startsWith("\"") && bestPlayers.endsWith("\"") && bestPlayers.indexOf(',') != -1) {
    bestPlayers = bestPlayers.substring(1, bestPlayers.length() - 1);
  }

  int startIndex = 0;
  while (startIndex < bestPlayers.length()) {
    int commaPos = bestPlayers.indexOf(',', startIndex);

    // If no comma is found, we are at the last value in the list
    if (commaPos == -1) {
      commaPos = bestPlayers.length();
    }

    // Extract and add the current best player count to the lookup table
    String bestPlayer = bestPlayers.substring(startIndex, commaPos);
    int best = bestPlayer.toInt();

    // if the best count is greater than 12, ignore it
    if (best > 12) {
      startIndex = commaPos + 1;
      continue;
    }

    // Store the index of this game under the respective "Best" player count
    bestPlayerLookup[best].push_back(gameIndex);

    // Move the startIndex past the comma for the next iteration
    startIndex = commaPos + 1;
  }
}

// Pull the list of files from an array initialized the first time this method is called.
String* getGameFiles() {
  if (gameFileCount == 0) {
    gameFiles = getFilesInRoot(gameFileCount);  // Returns the array and sets gameFileCount
  }
  return gameFiles;
}

// Pull the list of files from the root directory that end in ".csv". These are the game files.
String* getFilesInRoot(int &fileCount) {
  std::vector<String> fileList;  // Vector to store file names

  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return nullptr;
  }

  // Open the root directory
  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open root directory");
  }

  // Open the first file in the directory
  File file = root.openNextFile();

  // Loop through all the files in the directory
  while (file && !file.isDirectory()) {
    String fileName = String(file.name());

    // Check if the filename ends with ".csv" and remove the extension
    if (fileName.endsWith(".csv")) {
      fileName = fileName.substring(0, fileName.length() - 4);  // Remove ".csv"
      fileList.push_back(fileName);  // Add the file name to the vector
      Serial.print("Adding file: ");
      Serial.println(fileName);
    } else {
      Serial.print("Ignoring non-csv file: ");
      Serial.println(fileName);
    }
    file = root.openNextFile();    // Move to the next file
  }

  // If no files were found, return nullptr
  fileCount = fileList.size();
  if (fileCount == 0) {
    Serial.println("No CSV files found.");
    return nullptr;
  }

  // Convert the vector to a String array
  String* filesArray = new String[fileCount];  // Allocate memory for the array
  for (int i = 0; i < fileCount; ++i) {
    filesArray[i] = fileList[i];
  }

  Serial.println("Done loading files");
  return filesArray;  // Return the dynamically allocated array
}

// Initialize the game data by loading the first file in the list
void initializeGameData() {
  String* files = getGameFiles();
  if (gameFileCount > 0) {
    Serial.print("loading games from: ");
    Serial.println(files[0] + ".csv");
    loadGames("/" + files[0] + ".csv");
    Serial.println("done loading games");
  } else {
    Serial.println("ERROR unable to load games. No files found.");
  }
}

// Function to load data from the CSV file
void loadGames(String filename) {
  Serial.println("Removing games from cache");
  for (int i = 0; i < MAX_BEST; i++) {
    bestPlayerLookup[i].clear();
  }
  gameCount = 0;

  Serial.print("loading games from file: ");
  Serial.println(filename);

  // Open the file on the filesystem
  File dataFile = SPIFFS.open(filename);
  if (!dataFile) {
    Serial.print("Error opening file: ");
    Serial.println(filename);
    return;
  }

  while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n');

    // Skip the header row
    if (line.startsWith("Text")) {
      continue;
    }

    // Split the line into name, min, max, and best fields
    int firstComma = line.indexOf(',');
    int secondComma = line.indexOf(',', firstComma + 1);
    int thirdComma = line.indexOf(',', secondComma + 1);

    // Store the game data in the array
    games[gameCount].title = line.substring(0, firstComma);
    games[gameCount].minPlayers = line.substring(firstComma + 1, secondComma).toInt();
    games[gameCount].maxPlayers = line.substring(secondComma + 1, thirdComma).toInt();
    games[gameCount].bestPlayers = line.substring(thirdComma + 1);

    // Add the game to the lookup table for fast retrieval
    addGameToLookup(gameCount, games[gameCount].bestPlayers);

    gameCount++;

    if (gameCount >= MAX_GAMES) {
      break;  // Avoid overflow if the dataset is larger than expected
    }
  }

  dataFile.close();
  Serial.println("Data loaded successfully.");
  Serial.print("Total games: ");
  Serial.println(gameCount);
}

// Function to search for games by "Best" player count
void getRecommendedGames(int bestCount, String results[], int resultCount) {
  int prevCount[resultCount];
  int count = 0;
  // Check if the best count is within the valid range
  if (bestCount < 1 || bestCount > 12) {
    Serial.println("Invalid player count. Please enter a number between 1 and 12.");
    return;
  }

  while (count < resultCount) {
    int gameIndex = random(0, bestPlayerLookup[bestCount].size());
    // check the prevCount array to see if we have already displayed this game
    bool found = false;
    for (int i = 0; i < count; i++) {
      if (prevCount[i] == gameIndex) {
        found = true;
        break;
      }
    }
    if (found) {
      continue;
    } else {
      int bestGameNum = bestPlayerLookup[bestCount][gameIndex];
      prevCount[count] = gameIndex;
      results[count] = games[bestGameNum].title;  // Assuming each game has a 'title' field
      count++;
    }
  }
}

// Function to dump the game data to the serial console
void dumpGameData() {
  // std::vector<int> bestPlayerLookup[MAX_BEST];  // 13 to account for indices
  // 1 to 12

  for (int i = 0; i < MAX_BEST; i++) {
    Serial.println("Best games for:" + String(i));
    for (int j = 0; j < bestPlayerLookup[i].size(); j++) {
      Serial.print(" Game: ");
      Serial.print(games[bestPlayerLookup[i][j]].title);
      Serial.print(" Min: ");
      Serial.print(games[bestPlayerLookup[i][j]].minPlayers);
      Serial.print(" Max: ");
      Serial.print(games[bestPlayerLookup[i][j]].maxPlayers);
      Serial.print(" Best: ");
      Serial.println(games[bestPlayerLookup[i][j]].bestPlayers);
    }
  }
}