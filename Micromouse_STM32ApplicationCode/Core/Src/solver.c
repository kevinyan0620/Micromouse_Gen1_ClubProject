#include "solver.h"
#include "irs.h"
#include "controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Cell maze [MAZEROW][MAZECOL];
int isInitialized = false;
int queueCount;
struct Node* queueFront;
struct Node* queueBack;

int botLocX = 0;
int botLocY = 0;
char botOrientation = 'n';
int goalFound = false;

Cell mostRecentCells[3];

// --- HISTORY BUFFER FUNCTIONS ---

void initRecentCells() {
    for (int i = 0; i < 3; i++) {
        mostRecentCells[i].locX = -1;
        mostRecentCells[i].locY = -1;
    }
}

void logNewWall(int x, int y, char dir) {
    // 1. Shift the history buffer down
    mostRecentCells[2] = mostRecentCells[1];
    mostRecentCells[1] = mostRecentCells[0];

    // 2. Log the location
    mostRecentCells[0].locX = x;
    mostRecentCells[0].locY = y;

    // 3. Clear all walls first
    mostRecentCells[0].wallNorth = false;
    mostRecentCells[0].wallSouth = false;
    mostRecentCells[0].wallEast  = false;
    mostRecentCells[0].wallWest  = false;

    // 4. Set ONLY the wall that was just detected
    if (dir == 'n') mostRecentCells[0].wallNorth = true;
    if (dir == 's') mostRecentCells[0].wallSouth = true;
    if (dir == 'e') mostRecentCells[0].wallEast  = true;
    if (dir == 'w') mostRecentCells[0].wallWest  = true;
}

void undoRecentWalls() {
    for (int i = 0; i < 3; i++) {
        int rx = mostRecentCells[i].locX;
        int ry = mostRecentCells[i].locY;

        if (rx != -1 && ry != -1) {
            // Undo North Wall
            if (mostRecentCells[i].wallNorth) {
                maze[rx][ry].wallNorth = false;
                if (ry < MAZEROW - 1) maze[rx][ry + 1].wallSouth = false;
            }
            // Undo South Wall
            else if (mostRecentCells[i].wallSouth) {
                maze[rx][ry].wallSouth = false;
                if (ry > 0) maze[rx][ry - 1].wallNorth = false;
            }
            // Undo East Wall
            else if (mostRecentCells[i].wallEast) {
                maze[rx][ry].wallEast = false;
                if (rx < MAZECOL - 1) maze[rx + 1][ry].wallWest = false;
            }
            // Undo West Wall
            else if (mostRecentCells[i].wallWest) {
                maze[rx][ry].wallWest = false;
                if (rx > 0) maze[rx - 1][ry].wallEast = false;
            }
        }

        // Clear this slot in the buffer
        mostRecentCells[i].locX = -1;
        mostRecentCells[i].locY = -1;

        HAL_GPIO_WritePin(YellowLED_GPIO_Port, YellowLED_Pin, GPIO_PIN_SET);
        HAL_Delay(500);
        HAL_GPIO_WritePin(YellowLED_GPIO_Port, YellowLED_Pin, GPIO_PIN_RESET);
    }
}

void initialization(){
	initRecentCells();
    for (int i = 0; i < MAZEROW; i++){
        for (int j = 0; j < MAZECOL; j++){
            maze[i][j].weight = -1;
            maze[i][j].locX = i;
            maze[i][j].locY = j;
            if (j == 0){
                maze[i][j].wallSouth = true;
            }
            else{
                maze[i][j].wallSouth = false;
            }

            if (j == (MAZEROW - 1)){
                maze[i][j].wallNorth = true;
            }
            else{
                maze[i][j].wallNorth = false;
            }

            if (i == 0){
                maze[i][j].wallWest = true;
            }
            else{
                maze[i][j].wallWest = false;
            }

            if (i == (MAZECOL - 1)){
                maze[i][j].wallEast = true;
            }
            else{
                maze[i][j].wallEast = false;
            }
        }
    }
}

void refresh(){ // Keep all wall data but clear weights
    for (int i = 0; i < MAZEROW; i++){
        for (int j = 0; j < MAZECOL; j++){
            maze[i][j].weight = -1;
        }
    }
}

void queueInit(){
    queueFront = malloc(sizeof(Node));
    queueFront->cell.weight = -1;
    queueFront->cell.locX = -1;
    queueFront->cell.locY = -1;
    queueFront->cell.wallNorth = false;
    queueFront->cell.wallSouth = false;
    queueFront->cell.wallWest  = false;
    queueFront->cell.wallEast  = false;
    queueFront->next = NULL;
    queueBack = queueFront;
    queueCount = 0;
}


void push(Cell cell){
    Node* newNode = malloc(sizeof(Node));
    newNode->cell = cell;
    newNode->next = NULL;
    queueBack -> next = newNode;
    queueBack = newNode;
    queueCount ++;
}

Cell pop (){
    queueCount --;
    if (queueFront != queueBack) {
        Node* frontNode = queueFront -> next;
        Cell frontCell = frontNode -> cell;
        queueFront -> next = frontNode -> next;

        if (queueBack == frontNode){
            queueBack = queueFront;
        }

        free(frontNode);
        return frontCell;
    }
    else{
        return queueFront->cell;
    }
}

void dump() {
    for (Node* ptr = queueFront->next; ptr != NULL; ptr=ptr->next){
        fprintf(stderr, "Location (%d, %d); Weight %d \n", ptr->cell.locX, ptr->cell.locY, ptr->cell.weight);
    }
}

void queueDestructor (){
    Node* ptr = queueFront;
    Node* nextptr = ptr -> next;
    for (; nextptr != NULL; nextptr = nextptr->next){
        free(ptr);
        ptr = nextptr;
    }
    free(ptr);
}

Action solver() {
    if (!goalFound){
    	// CHANGE THIS!!!!
    	// return floodFill (-1, -1);
        return floodFill(-1, -1);
    }
    else{
        return floodFill (0, 0);
    }
}

// This is an example of a simple left wall following algorithm.
//Action leftWallFollower() {
//    if(API_wallFront()) {
//        if(API_wallLeft()){
//            return RIGHT;
//        }
//        return LEFT;
//    }
//    return FORWARD;
//}

void floodFillCalc(int locX, int locY){
    refresh();

    queueInit();
    if (locX == -1 && locY == -1){
        maze[7][7].weight = 0;
        push(maze[7][7]);
        maze[7][8].weight = 0;
        push(maze[7][8]);
        maze[8][7].weight = 0;
        push(maze[8][7]);
        maze[8][8].weight = 0;
        push(maze[8][8]);
    }
    else{
        maze[locX][locY].weight = 0;
        push(maze[locX][locY]);
    }

    while (queueCount != 0){
        Cell currentCell = pop();

        //char weight_str[10];
        if (currentCell.wallNorth == false && maze[currentCell.locX][currentCell.locY + 1].weight == -1){
            maze[currentCell.locX][currentCell.locY + 1].weight = currentCell.weight + 1;
            push(maze[currentCell.locX][currentCell.locY + 1]);
        }

        if (currentCell.wallSouth == false && maze[currentCell.locX][currentCell.locY - 1].weight == -1){
            maze[currentCell.locX][currentCell.locY - 1].weight = currentCell.weight + 1;
            push(maze[currentCell.locX][currentCell.locY - 1]);
        }

        if (currentCell.wallEast == false && maze[currentCell.locX + 1][currentCell.locY].weight == -1){
            maze[currentCell.locX + 1][currentCell.locY].weight = currentCell.weight + 1;
            push(maze[currentCell.locX + 1][currentCell.locY]);
        }

        if (currentCell.wallWest == false && maze[currentCell.locX - 1][currentCell.locY].weight == -1){
            maze[currentCell.locX - 1][currentCell.locY].weight = currentCell.weight + 1;
            push(maze[currentCell.locX - 1][currentCell.locY]);
        }
    }

    queueDestructor();
}


// Put your implementation of floodfill here!
Action floodFill(int locX, int locY) {
    if (!isInitialized){
        initialization();
        isInitialized = true;
    }

    if (maze[botLocX][botLocY].weight == 0){
        goalFound = !goalFound;
        refresh();
        return IDLE;
    }

    if (botOrientation == 'n'){
//        if (isWallFront() && botLocY != MAZEROW - 1){
//            maze[botLocX][botLocY].wallNorth = true;
//            maze[botLocX][botLocY + 1].wallSouth = true;
//        }
//
//        if (isWallRight() && botLocX != MAZECOL - 1){
//            maze[botLocX][botLocY].wallEast = true;
//            maze[botLocX + 1][botLocY].wallWest = true;
//        }
//
//        if (isWallLeft() && botLocX != 0){
//            maze[botLocX][botLocY].wallWest = true;
//            maze[botLocX - 1][botLocY].wallEast = true;
//        }
//
//        floodFillCalc(locX, locY);
//
//        if (maze[botLocX][botLocY].wallNorth == false && maze[botLocX][botLocY + 1].weight == maze[botLocX][botLocY].weight - 1){
//            botLocY ++;
//            return FORWARD;
//        }
//
//        if (maze[botLocX][botLocY].wallEast == false && maze[botLocX + 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
//            turn(90);
//            botOrientation = 'e';
//            botLocX ++;
//            return FORWARD;
//        }
//
//        if (maze[botLocX][botLocY].wallWest == false && maze[botLocX - 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
//            turn(-90);
//            botOrientation = 'w';
//            botLocX --;
//            return FORWARD;
//        }
//
//        // Only backward available
//        turn(90);
//        turn(90);
//        botOrientation = 's';
//        return IDLE;

    	if (isWallFront() && botLocY != MAZEROW - 1){
			if (maze[botLocX][botLocY].wallNorth == false) {
				maze[botLocX][botLocY].wallNorth = true;
				maze[botLocX][botLocY + 1].wallSouth = true;
				logNewWall(botLocX, botLocY, 'n');
			}
		}

		if (isWallRight() && botLocX != MAZECOL - 1){
			if (maze[botLocX][botLocY].wallEast == false) {
				maze[botLocX][botLocY].wallEast = true;
				maze[botLocX + 1][botLocY].wallWest = true;
				logNewWall(botLocX, botLocY, 'e');
			}
		}

		if (isWallLeft() && botLocX != 0){
			if (maze[botLocX][botLocY].wallWest == false) {
				maze[botLocX][botLocY].wallWest = true;
				maze[botLocX - 1][botLocY].wallEast = true;
				logNewWall(botLocX, botLocY, 'w');
			}
		}

		floodFillCalc(locX, locY);

		if (maze[botLocX][botLocY].wallNorth == false && maze[botLocX][botLocY + 1].weight == maze[botLocX][botLocY].weight - 1){
			botLocY ++;
			return FORWARD;
		}

		if (maze[botLocX][botLocY].wallEast == false && maze[botLocX + 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
			turn(90);
			botOrientation = 'e';
			botLocX ++;
			return FORWARD;
		}

		if (maze[botLocX][botLocY].wallWest == false && maze[botLocX - 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
			turn(-90);
			botOrientation = 'w';
			botLocX --;
			return FORWARD;
		}

		// Only backward available
		turn(90);
		turn(90);
		botOrientation = 's';
		return IDLE;
    }

    else if (botOrientation == 'e'){
//        if (isWallFront() && botLocX != MAZECOL - 1){
//            maze[botLocX][botLocY].wallEast = true;
//            maze[botLocX + 1][botLocY].wallWest = true;
//        }
//
//        if (isWallRight() && botLocY != 0){
//            maze[botLocX][botLocY].wallSouth = true;
//            maze[botLocX][botLocY - 1].wallNorth = true;
//        }
//
//        if (isWallLeft() && botLocY != MAZEROW - 1){
//            maze[botLocX][botLocY].wallNorth = true;
//            maze[botLocX][botLocY + 1].wallSouth = true;
//        }
//
//        floodFillCalc(locX, locY);
//
//        if (maze[botLocX][botLocY].wallEast == false && maze[botLocX + 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
//            botLocX ++;
//            return FORWARD;
//        }
//
//        if (maze[botLocX][botLocY].wallSouth == false && maze[botLocX][botLocY - 1].weight == maze[botLocX][botLocY].weight - 1){
//            turn(90);
//            botOrientation = 's';
//            botLocY --;
//            return FORWARD;
//        }
//
//        if (maze[botLocX][botLocY].wallNorth == false && maze[botLocX][botLocY + 1].weight == maze[botLocX][botLocY].weight - 1){
//            turn(-90);
//            botOrientation = 'n';
//            botLocY ++;
//            return FORWARD;
//        }
//
//        // Only backward available
//        turn(90);
//        turn(90);
//        botOrientation = 'w';
//        return IDLE;
    	if (isWallFront() && botLocX != MAZECOL - 1){
			if (maze[botLocX][botLocY].wallEast == false) {
				maze[botLocX][botLocY].wallEast = true;
				maze[botLocX + 1][botLocY].wallWest = true;
				logNewWall(botLocX, botLocY, 'e');
			}
		}

		if (isWallRight() && botLocY != 0){
			if (maze[botLocX][botLocY].wallSouth == false) {
				maze[botLocX][botLocY].wallSouth = true;
				maze[botLocX][botLocY - 1].wallNorth = true;
				logNewWall(botLocX, botLocY, 's');
			}
		}

		if (isWallLeft() && botLocY != MAZEROW - 1){
			if (maze[botLocX][botLocY].wallNorth == false) {
				maze[botLocX][botLocY].wallNorth = true;
				maze[botLocX][botLocY + 1].wallSouth = true;
				logNewWall(botLocX, botLocY, 'n');
			}
		}

		floodFillCalc(locX, locY);

		if (maze[botLocX][botLocY].wallEast == false && maze[botLocX + 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
			botLocX ++;
			return FORWARD;
		}

		if (maze[botLocX][botLocY].wallSouth == false && maze[botLocX][botLocY - 1].weight == maze[botLocX][botLocY].weight - 1){
			turn(90);
			botOrientation = 's';
			botLocY --;
			return FORWARD;
		}

		if (maze[botLocX][botLocY].wallNorth == false && maze[botLocX][botLocY + 1].weight == maze[botLocX][botLocY].weight - 1){
			turn(-90);
			botOrientation = 'n';
			botLocY ++;
			return FORWARD;
		}

		// Only backward available
		turn(90);
		turn(90);
		botOrientation = 'w';
		return IDLE;
    }
    else if (botOrientation == 'w'){
//        if (isWallFront() && botLocX != 0){
//            maze[botLocX][botLocY].wallWest = true;
//            maze[botLocX - 1][botLocY].wallEast = true;
//        }
//
//        if (isWallRight() && botLocY != MAZEROW - 1){
//            maze[botLocX][botLocY].wallNorth = true;
//            maze[botLocX][botLocY + 1].wallSouth = true;
//        }
//
//        if (isWallLeft() && botLocY != 0){
//            maze[botLocX][botLocY].wallSouth = true;
//            maze[botLocX][botLocY - 1].wallNorth = true;
//        }
//
//        floodFillCalc(locX, locY);
//
//        if (maze[botLocX][botLocY].wallWest == false && maze[botLocX - 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
//            botLocX --;
//            return FORWARD;
//        }
//
//        if (maze[botLocX][botLocY].wallNorth == false && maze[botLocX][botLocY + 1].weight == maze[botLocX][botLocY].weight - 1){
//            turn(90);
//            botOrientation = 'n';
//            botLocY ++;
//            return FORWARD;
//        }
//
//        if (maze[botLocX][botLocY].wallSouth == false && maze[botLocX][botLocY - 1].weight == maze[botLocX][botLocY].weight - 1){
//            turn(-90);
//            botOrientation = 's';
//            botLocY --;
//            return FORWARD;
//        }
//
//        // Only backward available
//        turn(90);
//        turn(90);
//        botOrientation = 'e';
//        return IDLE;
    	if (isWallFront() && botLocX != 0){
			if (maze[botLocX][botLocY].wallWest == false) {
				maze[botLocX][botLocY].wallWest = true;
				maze[botLocX - 1][botLocY].wallEast = true;
				logNewWall(botLocX, botLocY, 'w');
			}
		}

		if (isWallRight() && botLocY != MAZEROW - 1){
			if (maze[botLocX][botLocY].wallNorth == false) {
				maze[botLocX][botLocY].wallNorth = true;
				maze[botLocX][botLocY + 1].wallSouth = true;
				logNewWall(botLocX, botLocY, 'n');
			}
		}

		if (isWallLeft() && botLocY != 0){
			if (maze[botLocX][botLocY].wallSouth == false) {
				maze[botLocX][botLocY].wallSouth = true;
				maze[botLocX][botLocY - 1].wallNorth = true;
				logNewWall(botLocX, botLocY, 's');
			}
		}

		floodFillCalc(locX, locY);

		if (maze[botLocX][botLocY].wallWest == false && maze[botLocX - 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
			botLocX --;
			return FORWARD;
		}

		if (maze[botLocX][botLocY].wallNorth == false && maze[botLocX][botLocY + 1].weight == maze[botLocX][botLocY].weight - 1){
			turn(90);
			botOrientation = 'n';
			botLocY ++;
			return FORWARD;
		}

		if (maze[botLocX][botLocY].wallSouth == false && maze[botLocX][botLocY - 1].weight == maze[botLocX][botLocY].weight - 1){
			turn(-90);
			botOrientation = 's';
			botLocY --;
			return FORWARD;
		}

		// Only backward available
		turn(90);
		turn(90);
		botOrientation = 'e';
		return IDLE;
    }
    else if (botOrientation == 's'){
//        if (isWallFront() && botLocY != 0){
//            maze[botLocX][botLocY].wallSouth = true;
//            maze[botLocX][botLocY - 1].wallNorth = true;
//        }
//
//        if (isWallRight() && botLocX != 0){
//            maze[botLocX][botLocY].wallWest = true;
//            maze[botLocX - 1][botLocY].wallEast = true;
//        }
//
//        if (isWallLeft() && botLocX != MAZECOL - 1){
//            maze[botLocX][botLocY].wallEast = true;
//            maze[botLocX + 1][botLocY].wallWest = true;
//        }
//
//        floodFillCalc(locX, locY);
//
//        if (maze[botLocX][botLocY].wallSouth == false && maze[botLocX][botLocY - 1].weight == maze[botLocX][botLocY].weight - 1){
//            botLocY --;
//            return FORWARD;
//        }
//
//        if (maze[botLocX][botLocY].wallWest == false && maze[botLocX - 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
//            turn(90);
//            botOrientation = 'w';
//            botLocX --;
//            return FORWARD;
//        }
//
//        if (maze[botLocX][botLocY].wallEast == false && maze[botLocX + 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
//            turn(-90);
//            botOrientation = 'e';
//            botLocX ++;
//            return FORWARD;
//        }
//
//        // Only backward available
//        turn(90);
//        turn(90);
//        botOrientation = 'n';
//        return IDLE;
    	if (isWallFront() && botLocY != 0){
			if (maze[botLocX][botLocY].wallSouth == false) {
				maze[botLocX][botLocY].wallSouth = true;
				maze[botLocX][botLocY - 1].wallNorth = true;
				logNewWall(botLocX, botLocY, 's');
			}
		}

		if (isWallRight() && botLocX != 0){
			if (maze[botLocX][botLocY].wallWest == false) {
				maze[botLocX][botLocY].wallWest = true;
				maze[botLocX - 1][botLocY].wallEast = true;
				logNewWall(botLocX, botLocY, 'w');
			}
		}

		if (isWallLeft() && botLocX != MAZECOL - 1){
			if (maze[botLocX][botLocY].wallEast == false) {
				maze[botLocX][botLocY].wallEast = true;
				maze[botLocX + 1][botLocY].wallWest = true;
				logNewWall(botLocX, botLocY, 'e');
			}
		}

		floodFillCalc(locX, locY);

		if (maze[botLocX][botLocY].wallSouth == false && maze[botLocX][botLocY - 1].weight == maze[botLocX][botLocY].weight - 1){
			botLocY --;
			return FORWARD;
		}

		if (maze[botLocX][botLocY].wallWest == false && maze[botLocX - 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
			turn(90);
			botOrientation = 'w';
			botLocX --;
			return FORWARD;
		}

		if (maze[botLocX][botLocY].wallEast == false && maze[botLocX + 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
			turn(-90);
			botOrientation = 'e';
			botLocX ++;
			return FORWARD;
		}

		// Only backward available
		turn(90);
		turn(90);
		botOrientation = 'n';
		return IDLE;
	}

    //fprintf(stderr, "Some bad thing happened...");
    return IDLE;
}

void resetRobotPos (){
	botLocX = 0;
	botLocY = 0;
	botOrientation = 'n';
}

char* getOptimalPath(void) {
    // 512 is more than enough for a 16x16 maze optimal path
    static char path[512];
    int pIdx = 0;

    int currX = 0;
    int currY = 0;
    char currDir = 'n'; // Robot always starts at (0,0) facing North

    // 1. Force a clean weight calculation from the Center (-1, -1) to the Start
    floodFillCalc(-1, -1);

    // 2. Trace the path from (0,0) down the weight gradient to 0
    while (maze[currX][currY].weight > 0) {
        int curW = maze[currX][currY].weight;
        int nextX = currX;
        int nextY = currY;
        char targetDir = currDir;

        // --- STEP A: Find the next cell in the gradient (weight == curW - 1) ---
        if (maze[currX][currY].wallNorth == false && currY < MAZEROW - 1 && maze[currX][currY + 1].weight == curW - 1) {
            targetDir = 'n';
            nextY++;
        }
        else if (maze[currX][currY].wallEast == false && currX < MAZECOL - 1 && maze[currX + 1][currY].weight == curW - 1) {
            targetDir = 'e';
            nextX++;
        }
        else if (maze[currX][currY].wallSouth == false && currY > 0 && maze[currX][currY - 1].weight == curW - 1) {
            targetDir = 's';
            nextY--;
        }
        else if (maze[currX][currY].wallWest == false && currX > 0 && maze[currX - 1][currY].weight == curW - 1) {
            targetDir = 'w';
            nextX--;
        }
        else {
            // Failsafe: If the maze is broken and it gets stuck, terminate the string early
            break;
        }

        // --- STEP B: Calculate the turns needed to face the target cell ---
        if (currDir != targetDir) {
            // 90 Degree Right Turns
            if ((currDir == 'n' && targetDir == 'e') ||
                (currDir == 'e' && targetDir == 's') ||
                (currDir == 's' && targetDir == 'w') ||
                (currDir == 'w' && targetDir == 'n')) {
                path[pIdx++] = 'r';
            }
            // 90 Degree Left Turns
            else if ((currDir == 'n' && targetDir == 'w') ||
                     (currDir == 'w' && targetDir == 's') ||
                     (currDir == 's' && targetDir == 'e') ||
                     (currDir == 'e' && targetDir == 'n')) {
                path[pIdx++] = 'l';
            }
            // 180 Degree Turns (U-Turns)
            else {
                path[pIdx++] = 'r';
                path[pIdx++] = 'r';
            }
            currDir = targetDir; // Update our virtual robot's orientation
        }

        // --- STEP C: Move forward into the new cell ---
        path[pIdx++] = 'f';
        currX = nextX;
        currY = nextY;
    }

    // 3. Null-terminate the C-string
    path[pIdx] = '\0';

    return path;
}
