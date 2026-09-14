#include "solver.h"
#include "API.h"
#include <stdio.h>
#include <stdlib.h>

Cell maze [MAZEROW][MAZECOL];
bool isInitialized = false;
int queueCount;
struct Node* queueFront;
struct Node* queueBack;

int botLocX = 0;
int botLocY = 0;
char botOrientation = 'n';
bool goalFound = false;

void initialization(){
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
        return floodFill(-1, -1);
    }
    else{
        return floodFill (0, 0);
    }
}

// This is an example of a simple left wall following algorithm.
Action leftWallFollower() {
    if(API_wallFront()) {
        if(API_wallLeft()){
            return RIGHT;
        }
        return LEFT;
    }
    return FORWARD;
}

void floodFillCalc(int locX, int locY){
    refresh();
    
    queueInit();
    if (locX == -1 && locY == -1){
        API_clearColor(0, 0);
        maze[7][7].weight = 0;
        push(maze[7][7]);
        API_setColor(7, 7, 'g');
        maze[7][8].weight = 0;
        push(maze[7][8]);
        API_setColor(7, 8, 'g');
        maze[8][7].weight = 0;
        push(maze[8][7]);
        API_setColor(8, 7, 'g');
        maze[8][8].weight = 0;
        push(maze[8][8]);
        API_setColor(8, 8, 'g');
    }
    else{
        API_clearColor(7, 7);
        API_clearColor(7, 8);
        API_clearColor(8, 7);
        API_clearColor(8, 8);
        maze[locX][locY].weight = 0;
        push(maze[locX][locY]);
        API_setColor(locX, locY, 'g');
    }
    
    while (queueCount != 0){
        Cell currentCell = pop();
        
        char weight_str[10];
        snprintf(weight_str, sizeof(weight_str), "%d", currentCell.weight);
        API_setText(currentCell.locX, currentCell.locY, weight_str);
        fprintf(stderr, "Popped: Location (%d, %d); Weight %d \n", currentCell.locX, currentCell.locY, currentCell.weight);
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
        
        // Set Walls (Not Needed)
        if (currentCell.wallNorth == true){
            API_setWall(currentCell.locX, currentCell.locY, 'n');
        }
        if (currentCell.wallSouth == true){
            API_setWall(currentCell.locX, currentCell.locY, 's');
        }
        if (currentCell.wallWest == true){
            API_setWall(currentCell.locX, currentCell.locY, 'w');
        }
        if (currentCell.wallEast == true){
            API_setWall(currentCell.locX, currentCell.locY, 'e');
        }
    }
    
    fprintf(stderr, "Amount of nodes in queue: %d \n", queueCount);
    queueDestructor();
}


// Put your implementation of floodfill here!
Action floodFill(int locX, int locY) {
    if (!isInitialized){
        initialization();
        isInitialized = true;
    }
    
    if (maze[botLocX][botLocY].weight == 0){
        fprintf(stderr, "Success! \n");
        goalFound = !goalFound;
        refresh();
        return IDLE;
    }
    
    if (botOrientation == 'n'){
        if (API_wallFront() && botLocY != MAZEROW - 1){
            maze[botLocX][botLocY].wallNorth = true;
            maze[botLocX][botLocY + 1].wallSouth = true;
        }
        
        if (API_wallRight() && botLocX != MAZECOL - 1){
            maze[botLocX][botLocY].wallEast = true;
            maze[botLocX + 1][botLocY].wallWest = true;
        }
        
        if (API_wallLeft() && botLocX != 0){
            maze[botLocX][botLocY].wallWest = true;
            maze[botLocX - 1][botLocY].wallEast = true;
        }
        
        floodFillCalc(locX, locY);
        
        if (maze[botLocX][botLocY].wallNorth == false && maze[botLocX][botLocY + 1].weight == maze[botLocX][botLocY].weight - 1){
            botLocY ++;
            return FORWARD;
        }
        
        if (maze[botLocX][botLocY].wallEast == false && maze[botLocX + 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
            API_turnRight();
            botOrientation = 'e';
            botLocX ++;
            return FORWARD;
        }
        
        if (maze[botLocX][botLocY].wallWest == false && maze[botLocX - 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
            API_turnLeft();
            botOrientation = 'w';
            botLocX --;
            return FORWARD;
        }
        
        // Only backward available
        API_turnRight();
        API_turnRight();
        botOrientation = 's';
        return IDLE;
    }
    
    else if (botOrientation == 'e'){
        if (API_wallFront() && botLocX != MAZECOL - 1){
            maze[botLocX][botLocY].wallEast = true;
            maze[botLocX + 1][botLocY].wallWest = true;
        }
        
        if (API_wallRight() && botLocY != 0){
            maze[botLocX][botLocY].wallSouth = true;
            maze[botLocX][botLocY - 1].wallNorth = true;
        }
        
        if (API_wallLeft() && botLocY != MAZEROW - 1){
            maze[botLocX][botLocY].wallNorth = true;
            maze[botLocX][botLocY + 1].wallSouth = true;
        }
        
        floodFillCalc(locX, locY);
        
        if (maze[botLocX][botLocY].wallEast == false && maze[botLocX + 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
            botLocX ++;
            return FORWARD;
        }
        
        if (maze[botLocX][botLocY].wallSouth == false && maze[botLocX][botLocY - 1].weight == maze[botLocX][botLocY].weight - 1){
            API_turnRight();
            botOrientation = 's';
            botLocY --;
            return FORWARD;
        }
        
        if (maze[botLocX][botLocY].wallNorth == false && maze[botLocX][botLocY + 1].weight == maze[botLocX][botLocY].weight - 1){
            API_turnLeft();
            botOrientation = 'n';
            botLocY ++;
            return FORWARD;
        }
        
        // Only backward available
        API_turnRight();
        API_turnRight();
        botOrientation = 'w';
        return IDLE;
    }
    else if (botOrientation == 'w'){
        if (API_wallFront() && botLocX != 0){
            maze[botLocX][botLocY].wallWest = true;
            maze[botLocX - 1][botLocY].wallEast = true;
        }
            
        if (API_wallRight() && botLocY != MAZEROW - 1){
            maze[botLocX][botLocY].wallNorth = true;
            maze[botLocX][botLocY + 1].wallSouth = true;
        }
        
        if (API_wallLeft() && botLocY != 0){
            maze[botLocX][botLocY].wallSouth = true;
            maze[botLocX][botLocY - 1].wallNorth = true;
        }
        
        floodFillCalc(locX, locY);
        
        if (maze[botLocX][botLocY].wallWest == false && maze[botLocX - 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
            botLocX --;
            return FORWARD;
        }
        
        if (maze[botLocX][botLocY].wallNorth == false && maze[botLocX][botLocY + 1].weight == maze[botLocX][botLocY].weight - 1){
            API_turnRight();
            botOrientation = 'n';
            botLocY ++;
            return FORWARD;
        }
        
        if (maze[botLocX][botLocY].wallSouth == false && maze[botLocX][botLocY - 1].weight == maze[botLocX][botLocY].weight - 1){
            API_turnLeft();
            botOrientation = 's';
            botLocY --;
            return FORWARD;
        }
        
        // Only backward available
        API_turnRight();
        API_turnRight();
        botOrientation = 'e';
        return IDLE;
    }
    else if (botOrientation == 's'){
        if (API_wallFront() && botLocY != 0){
            maze[botLocX][botLocY].wallSouth = true;
            maze[botLocX][botLocY - 1].wallNorth = true;
        }
        
        if (API_wallRight() && botLocX != 0){
            maze[botLocX][botLocY].wallWest = true;
            maze[botLocX - 1][botLocY].wallEast = true;
        }
        
        if (API_wallLeft() && botLocX != MAZECOL - 1){
            maze[botLocX][botLocY].wallEast = true;
            maze[botLocX + 1][botLocY].wallWest = true;
        }
        
        floodFillCalc(locX, locY);
        
        if (maze[botLocX][botLocY].wallSouth == false && maze[botLocX][botLocY - 1].weight == maze[botLocX][botLocY].weight - 1){
            botLocY --;
            return FORWARD;
        }
        
        if (maze[botLocX][botLocY].wallWest == false && maze[botLocX - 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
            API_turnRight();
            botOrientation = 'w';
            botLocX --;
            return FORWARD;
        }
        
        if (maze[botLocX][botLocY].wallEast == false && maze[botLocX + 1][botLocY].weight == maze[botLocX][botLocY].weight - 1){
            API_turnLeft();
            botOrientation = 'e';
            botLocX ++;
            return FORWARD;
        }
        
        // Only backward available
        API_turnRight();
        API_turnRight();
        botOrientation = 'n';
        return IDLE;
    }
    
    fprintf(stderr, "Some bad thing happened...");
    return IDLE;
}
