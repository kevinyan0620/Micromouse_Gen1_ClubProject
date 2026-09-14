/*
 * solver.h
 *
 *  Created on: May 22, 2026
 *      Author: guanlinyan565
 */

#ifndef INC_SOLVER_H_
#define INC_SOLVER_H_

//#define MAZEROW 16
//#define MAZECOL 16

#define MAZEROW 16
#define MAZECOL 16

#define true 1
#define false 0

typedef enum Heading {NORTH, EAST, SOUTH, WEST} Heading;
typedef enum Action {LEFT, FORWARD, RIGHT, IDLE} Action;

Action solver();
Action leftWallFollower();
Action floodFill(int locX, int locY); // Set locX, locY == -1 for Center
void refresh();

void initialization ();

typedef struct {
    int weight;
    int locX;
    int locY;
    int wallNorth;
    int wallSouth;
    int wallWest;
    int wallEast;
} Cell;

typedef  struct Node {
    Cell cell;
    struct Node* next;
} Node;


// New history buffer functions
void initRecentCells(void);
void logNewWall(int x, int y, char dir);
void undoRecentWalls(void);

void resetRobotPos(void);
char* getOptimalPath(void);

#endif /* INC_SOLVER_H_ */
