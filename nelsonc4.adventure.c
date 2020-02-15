#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// boolean enum
enum boolf { false, true };
// room types enum
enum roomType { START_ROOM, MID_ROOM, END_ROOM };

// room struct
struct Room {
  int id;
  char name[10];
  int numConnections;
  struct Room* outboundConnections[6];
  enum roomType roomType;
  int fileLineCount;  // holds the line count of the room file
};

// array to hold the rooms
struct Room rooms[7];

// pointers to the start and end room
// used to compare the location of player
struct Room* startRoom;
struct Room* endRoom;

// holds the directory to search for
// room files
char roomsDir[256];

// pthread_mutex_t* myMutex = PTHREAD_MUTEX_INITIALIZER;

// finds the newest directory of rooms
// and puts it in roomsDir
void FindNewestDir() {
  int newestDirTime = -1;  // will hold the time of the newest dir
  char targetDirPrefix[32] = "nelsonc4.rooms.";  // prefix of dir
  char newestDirName[256];                       // temp holder for the dir name
  memset(newestDirName, '\0', sizeof(newestDirName));

  DIR* dirToCheck;
  struct dirent* fileInDir;
  struct stat dirAttributes;

  dirToCheck = opendir(".");

  // if the dir opened correctly
  if (dirToCheck > 0) {
    // while there are files in the dir to check
    while ((fileInDir = readdir(dirToCheck)) != NULL) {
      // if the file contains the prefix
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) {
        // get the stats for the file, used for the time created
        stat(fileInDir->d_name, &dirAttributes);
        // if the time created is newer than the previously checked
        if ((int)dirAttributes.st_mtime > newestDirTime) {
          // set the newest time
          newestDirTime = (int)dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          // put the dir name in the temp holder
          strcpy(newestDirName, fileInDir->d_name);
        }
      }
    }
    // by the time this executes newestDirName will
    // hold the newest dir, copy that into the global variable
    strcpy(roomsDir, newestDirName);
  }
}

// this returns the number of lines in the room file passed
int getFileLineCount(char* fileName) {
  // this block opens a the file
  char relFilename[80];
  strcpy(relFilename, roomsDir);
  strcat(relFilename, "/");
  strcat(relFilename, fileName);

  FILE* roomFile = fopen(relFilename, "r");

  if (roomFile == NULL) {
    printf("Could not open file\n");
    return 0;
  }
  // end block

  // count the number of lines in the file
  int count = 0;
  int ch = 0;
  while (!feof(roomFile)) {
    ch = fgetc(roomFile);
    if (ch == '\n') {
      count++;
    }
  }
  fclose(roomFile);
  return count;
}

// this opens a room file, reads the name in the
// first line and assigns that name to the rooms
// array at the roomIdx passed in
void getRoomName(char* fileName, int roomIdx) {
  // this block opens the file
  char relFilename[80];
  strcpy(relFilename, roomsDir);
  strcat(relFilename, "/");
  strcat(relFilename, fileName);

  FILE* roomFile = fopen(relFilename, "r");

  if (roomFile == NULL) {
    printf("Could not open file\n");
    return;
  }
  // end block

  // read the first line and extract the name
  // then assign it to the appropriate rooms element
  char firstLine[20];
  fgets(firstLine, 20, roomFile);
  char roomName[10];
  sscanf(firstLine, "%*s %*s %s", roomName);
  strcpy(rooms[roomIdx].name, roomName);

  fclose(roomFile);
}

// this finds the room type from the room file
// and assigns the type to the appr rooms based on
// the roomIdx
void getRoomType(char* fileName, int roomIdx) {
  // this block opens the rooms file
  char relFilename[80];
  strcpy(relFilename, roomsDir);
  strcat(relFilename, "/");
  strcat(relFilename, fileName);

  FILE* roomFile = fopen(relFilename, "r");

  if (roomFile == NULL) {
    printf("Could not open file\n");
    return;
  }
  // end block

  // this block reads the last line of the file
  // which contains the room type, then assigns
  // the room type to the rooms element with roomIdx
  // the line count is used to find the last line
  char line[100];
  int lineCount = 1;
  while (fgets(line, sizeof line, roomFile) != NULL) {
    if (lineCount == rooms[roomIdx].fileLineCount) {
      char roomType[12];
      sscanf(line, "%*s %*s %s", roomType);
      if (strcmp(roomType, "START_ROOM") == 0) {
        rooms[roomIdx].roomType = START_ROOM;
        startRoom = &rooms[roomIdx];
      } else if (strcmp(roomType, "END_ROOM") == 0) {
        rooms[roomIdx].roomType = END_ROOM;
        endRoom = &rooms[roomIdx];
      } else if (strcmp(roomType, "MID_ROOM") == 0) {
        rooms[roomIdx].roomType = MID_ROOM;
      } else {
        printf("room type not read\n");
      }
    }
    lineCount++;
  }
  fclose(roomFile);
}

// this function finds all the room connections associated
// with the room of roomIdx, then finds the matching
// room from the rooms array and points the outConnection
// pointer to the match
void getRoomConnections(char* fileName, int roomIdx) {
  // this block opens the room file
  char relFilename[80];
  strcpy(relFilename, roomsDir);
  strcat(relFilename, "/");
  strcat(relFilename, fileName);

  FILE* roomFile = fopen(relFilename, "r");

  if (roomFile == NULL) {
    printf("Could not open file\n");
    return;
  }
  // end block

  int i = 0;
  // for each of the rooms connections
  while (i < rooms[roomIdx].numConnections) {
    char line[100];
    int lineCount = 1;
    rewind(roomFile);
    // get the line at i + 2, skipping over the name line
    while (fgets(line, sizeof line, roomFile) != NULL) {
      if (lineCount == i + 2) {
        char connection[12];
        // parse the connection line
        sscanf(line, "%*s %*s %s", connection);
        int j;
        // find the rooms with the same name of the
        // connection and assign the outboundConnection
        // pointer
        for (j = 0; j < 7; j++) {
          if (strcmp(connection, rooms[j].name) == 0) {
            rooms[roomIdx].outboundConnections[i] = &rooms[j];
          }
        }
      }
      lineCount++;
    }
    i++;
  }
  fclose(roomFile);
}

// this function is used for debugging,
// it prints the struct info for each room
// in the rooms array
void printRoomsInfo() {
  int i;
  for (i = 0; i < 7; i++) {
    printf("Room %d %s, linecount %d, con %d, type %d\n", rooms[i].id,
           rooms[i].name, rooms[i].fileLineCount, rooms[i].numConnections,
           rooms[i].roomType);
    int j;
    for (j = 0; j < rooms[i].numConnections; j++) {
      printf("Connection %d %s\n", j, rooms[i].outboundConnections[j]->name);
    }
  }
}

// this function populates the rooms array
void ReadRooms() {
  FindNewestDir();
  // once the newest dir is found,
  // open it
  DIR* dirToCheck;
  dirToCheck = opendir(roomsDir);
  struct dirent* fileInDir;

  // if the dir opened correctly
  if (dirToCheck > 0) {
    int index = 0;
    // for each file assign the attributes of the room struct
    while ((fileInDir = readdir(dirToCheck)) != NULL && index < 7) {
      if (strcmp(fileInDir->d_name, ".") != 0 &&
          strcmp(fileInDir->d_name, "..") != 0) {
        rooms[index].id = index;
        rooms[index].fileLineCount = getFileLineCount(fileInDir->d_name);
        getRoomName(fileInDir->d_name, index);
        // the number of connections is always
        // 2 less than the number of lines in the file
        rooms[index].numConnections = rooms[index].fileLineCount - 2;
        getRoomType(fileInDir->d_name, index);

        index++;
      }
    }
    // after all the other attributes of the rooms have been filled
    // we can then find the matching connections
    rewinddir(dirToCheck);
    index = 0;
    while ((fileInDir = readdir(dirToCheck)) != NULL && index < 7) {
      if (strcmp(fileInDir->d_name, ".") != 0 &&
          strcmp(fileInDir->d_name, "..") != 0) {
        getRoomConnections(fileInDir->d_name, index);

        index++;
      }
    }
  }
}

// print the current rooms name with correct formatting
void PrintCurrRoomName(struct Room* currRoom) {
  printf("CURRENT LOCATION: %s\n", currRoom->name);
}

// print the possible connections with correct formatting
void PrintCurrRoomConnections(struct Room* currRoom) {
  printf("POSSIBLE CONNECTIONS: ");
  int i;
  for (i = 0; i < currRoom->numConnections; i++) {
    printf("%s", currRoom->outboundConnections[i]->name);
    if (i == currRoom->numConnections - 1) {
      printf(".\n");
    } else {
      printf(", ");
    }
  }
}

// this function validates the input is valid and returns
// the index of the validated room, otherwise it returns
// -99 or not found
int ValidateRoomInput(char name[], struct Room* currRoom) {
  int i;
  for (i = 0; i < currRoom->numConnections; i++) {
    if (strcmp(name, currRoom->outboundConnections[i]->name) == 0) {
      return currRoom->outboundConnections[i]->id;
    }
  }
  return -99;
}

void* createTimeFile() {
  // pthread_mutex_lock(myMutex);
  char timeStr[200];
  time_t t;
  struct tm* tmp;
  char* format = "%I:%M%p, %A, %B %d, %Y";
  t = time(NULL);
  tmp = localtime(&t);
  if (tmp == NULL) {
    printf("local time error\n");
    exit(EXIT_FAILURE);
  }
  if (strftime(timeStr, sizeof(timeStr), format, tmp) == 0) {
    printf("strftime error\n");
    exit(EXIT_FAILURE);
  }
  char timeFileName[] = "currentTime.txt";
  FILE* timeFile = fopen(timeFileName, "w");

  if (timeFile == NULL) {
    printf("Could not open file\n");
    return NULL;
  }

  fprintf(timeFile, "%s\n", timeStr);
  // pthread_mutex_unlock(myMutex);
  fclose(timeFile);
  return NULL;
}

void GetTime() {
  pthread_t thread;
  pthread_create(&thread, NULL, createTimeFile, NULL);
  // pthread_mutex_unlock(myMutex);
  pthread_join(thread, NULL);
  // pthread_mutex_lock(myMutex);
  // read from file
  char timeFileName[] = "currentTime.txt";
  FILE* timeFile = fopen(timeFileName, "r");

  if (timeFile == NULL) {
    printf("Could not open file\n");
    return;
  }
  char timeString[100];
  fgets(timeString, 100, timeFile);

  printf("\n%s\n", timeString);
  fclose(timeFile);
}

// this function takes the current room, asks the user to
// input the next room to travel to, validates the input,
// and returns the validated 'next room'
struct Room* GetNextRoom(struct Room* currRoom) {
  printf("WHERE TO? >");
  char name[10];
  enum boolf timeCall = false;
  scanf("%s", name);
  if (strcmp(name, "time") == 0) {
    GetTime();
    timeCall = true;
  } else {
    timeCall = false;
  }
  int nextRoomIndex = ValidateRoomInput(name, currRoom);
  // loop until valid room is typed
  while (nextRoomIndex == -99) {
    if (timeCall == false) {
      printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
      PrintCurrRoomName(currRoom);
      PrintCurrRoomConnections(currRoom);
    }
    printf("WHERE TO? >");
    scanf("%s", name);
    if (strcmp(name, "time") == 0) {
      GetTime();
      timeCall = true;
    } else {
      timeCall = false;
    }
    nextRoomIndex = ValidateRoomInput(name, currRoom);
  }
  return &rooms[nextRoomIndex];
}

// prints the winning message with the steps taken
void PrintWinningMessage(int steps, struct Room* path[100]) {
  printf(
      "YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR "
      "PATH TO VICTORY WAS:\n",
      steps);
  int i;
  for (i = 0; i < steps; i++) {
    printf("%s\n", path[i]->name);
  }
}

// main loop for game
void PlayGame() {
  // start at the room assigned to startRoom
  struct Room* currRoom = startRoom;
  // pointer to hold the path taken
  struct Room* path[100];
  int steps = 0;
  // while the current room is not the room assigned to end room
  while (currRoom != endRoom) {
    PrintCurrRoomName(currRoom);
    PrintCurrRoomConnections(currRoom);
    currRoom = GetNextRoom(currRoom);
    printf("\n");
    path[steps] = currRoom;  // add the room to the list of paths
    steps++;
  }
  PrintWinningMessage(steps, path);
}

int main() {
  // pthread_mutex_lock(myMutex);
  ReadRooms();
  PlayGame();
  return 0;
}