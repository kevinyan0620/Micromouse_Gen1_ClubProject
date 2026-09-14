#ifndef SOLVER_H
#define SOLVER_H
#include <stdbool.h>

#define MAZEROW 16
#define MAZECOL 16

typedef enum Heading {NORTH, EAST, SOUTH, WEST} Heading;
typedef enum Action {LEFT, FORWARD, RIGHT, IDLE} Action;

Action solver();
Action leftWallFollower();
Action floodFill(int locX, int locY); // Set locX, locY == -1 for Center

void initialization ();

typedef struct {
    int weight;
    int locX;
    int locY;
    bool wallNorth;
    bool wallSouth;
    bool wallWest;
    bool wallEast;
} Cell;

typedef  struct Node {
    Cell cell;
    struct Node* next;
} Node;

#endif
