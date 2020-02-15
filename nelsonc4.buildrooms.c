#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// enums of boolean and room types
enum boolf { false, true };
enum roomType { START_ROOM, MID_ROOM, END_ROOM };

// prefix for rooms dir
char roomsDir[50] = "nelsonc4.rooms.";
// list of possible rooms
char roomNames[10][10] = {"garage",   "kitchen", "bedroom", "bathroom",
                          "basement", "patio",   "famroom", "office",
                          "guest",    "dining"};

// struct of a room
struct Room {
  int id;
  char name[10];
  int numConnections;
  struct Room* outboundConnections[6];  // pointer to room connnections
  enum roomType roomType;
};

// array to hold all rooms
struct Room rooms[7];

// this function creates the dir to hold the room files
void CreateRoomsDir() {
  char procID[10];
  sprintf(procID, "%d", getpid());
  strcat(roomsDir, procID);
  mkdir(roomsDir, 0755);
}

// this function fills the rooms array
void GenerateRoomsArray() {
  srand(time(NULL));
  // a list of rooms already created so that,
  // we do not create duplicates
  char roomsCreated[7][10];

  // file the roomsCreated array with null
  int name;
  for (name = 0; name < 7; name++) {
    int letter;
    for (letter = 0; letter < 10; letter++) {
      roomsCreated[name][letter] = '\0';
    }
  }

  // for each of the rooms
  int i;
  for (i = 0; i < 7; i++) {
    // loop until an unique room is created
    enum boolf roomCreated = false;
    while (roomCreated == false) {
      // get a random room name
      int randnum = (rand() % 10);
      char roomName[10];
      strcpy(roomName, roomNames[randnum]);
      enum boolf roomAlreadyCreated = false;

      // check if the random room name is
      // already created, break if so
      int j;
      for (j = 0; j < 7; j++) {
        if (strcmp(roomName, roomsCreated[j]) == 0) {
          roomAlreadyCreated = true;
          break;
        }
      }

      // intialize room properties
      if (roomAlreadyCreated == false) {
        rooms[i].id = i;
        rooms[i].numConnections = 0;
        strcpy(rooms[i].name, roomName);
        if (i == 0) {
          rooms[i].roomType = START_ROOM;
        } else if (i == 1) {
          rooms[i].roomType = END_ROOM;
        } else {
          rooms[i].roomType = MID_ROOM;
        }
        roomCreated = true;
        strcpy(roomsCreated[i], roomName);
      }
    }
  }
}

// print the room info for debugging
void PrintRooms() {
  int i;
  for (i = 0; i < 7; i++) {
    printf("ROOM NAME: %s, ID: %d, CONNECTIONS: %d\n", rooms[i].name,
           rooms[i].id, rooms[i].numConnections);
    int j;
    for (j = 0; j < rooms[i].numConnections; j++) {
      printf("CONNECTION %d: %s, ID: %d\n", j + 1,
             rooms[i].outboundConnections[j]->name,
             rooms[i].outboundConnections[j]->id);
    }
    if (rooms[i].roomType == START_ROOM) {
      printf("ROOM TYPE: START_ROOM\n");
    } else if (rooms[i].roomType == END_ROOM) {
      printf("ROOM TYPE: END_ROOM\n");
    } else {
      printf("ROOM TYPE: MID_ROOM\n");
    }
  }
  printf("\n");
}

// this function takes the rooms array
// and creates files for each room
void GenerateRoomsFiles() {
  // for each room in the array
  int i;
  for (i = 0; i < 7; i++) {
    // this block creates a file
    // with the name formatted correctly
    // matching the room name
    char filename[15];
    char relFilename[80];
    strcpy(filename, rooms[i].name);
    strcat(filename, "_room");
    strcpy(relFilename, roomsDir);
    strcat(relFilename, "/");
    strcat(relFilename, filename);
    FILE* roomFile = fopen(relFilename, "w");
    // end block

    // add first line with the room name to file
    fprintf(roomFile, "ROOM NAME: %s\n", rooms[i].name);
    // add each connection the file
    int j;
    for (j = 0; j < rooms[i].numConnections; j++) {
      fprintf(roomFile, "CONNECTION %d: %s\n", j + 1,
              rooms[i].outboundConnections[j]->name);
    }
    // lastly, add the room type to file
    if (rooms[i].roomType == START_ROOM) {
      fprintf(roomFile, "ROOM TYPE: START_ROOM\n");
    } else if (rooms[i].roomType == END_ROOM) {
      fprintf(roomFile, "ROOM TYPE: END_ROOM\n");
    } else {
      fprintf(roomFile, "ROOM TYPE: MID_ROOM\n");
    }
    fclose(roomFile);
  }
}

// Returns a random Room, does NOT validate if connection can be added
struct Room* GetRandomRoom() {
  int randnum = rand() % 7;
  return &rooms[randnum];
}

// Returns true if a connection can be added from Room x (< 6 outbound
// connections), false otherwise
enum boolf CanAddConnectionFrom(struct Room* x) {
  if (x->numConnections < 6) {
    return true;
  }
  return false;
}
// Returns true if a connection from Room x to Room y already exists, false
// otherwise
enum boolf ConnectionAlreadyExists(struct Room* x, struct Room* y) {
  int i;
  for (i = 0; i < x->numConnections; i++) {
    if (x->outboundConnections[i]->id == y->id) {
      return true;
    }
  }
  return false;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct Room* x, struct Room* y) {
  x->outboundConnections[x->numConnections] = y;
  x->numConnections++;
}

// Returns true if Rooms x and y are the same Room, false otherwise
enum boolf IsSameRoom(struct Room* x, struct Room* y) {
  if (x->id == y->id) {
    return true;
  }
  return false;
}

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
enum boolf IsGraphFull() {
  int i;
  for (i = 0; i < 7; i++) {
    if (rooms[i].numConnections < 3) {
      return false;
    }
  }
  return true;
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection() {
  struct Room* A;
  struct Room* B;

  while (true) {
    A = GetRandomRoom();

    if (CanAddConnectionFrom(A) == true) break;
  }

  do {
    B = GetRandomRoom();
  } while (CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true ||
           ConnectionAlreadyExists(A, B) == true);

  ConnectRoom(A, B);
  ConnectRoom(B, A);
}

int main() {
  CreateRoomsDir();
  GenerateRoomsArray();

  // Create all connections in graph
  while (IsGraphFull() == false) {
    AddRandomConnection();
  }
  // PrintRooms();
  GenerateRoomsFiles();
  return 0;
}