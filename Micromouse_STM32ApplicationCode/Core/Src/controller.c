/*
 * controller.c
 */


#include "main.h"
#include "controller.h"
#include "pid.h"
#include "delay.h"
#include "gyroscope.h"
#include "solver.h"
#include "motors.h"

extern BNO085_t mouse_imu;
extern int isTraversing;
extern volatile float initial_yaw;

#define CELL_ENCODER_COUNT 628 // Originally: 620
#define SPEEDADDITIONAL 50

/*
 * We recommend you implement this function so that move(1) will move your rat 1 cell forward.
 */

extern int correctionCounter;

int detectButton (void) {
    if (HAL_GPIO_ReadPin(LeftButton_GPIO_Port, LeftButton_Pin) == GPIO_PIN_RESET && isTraversing == 1){
	    HAL_GPIO_WritePin(RedLED_GPIO_Port, RedLED_Pin, GPIO_PIN_SET);
	    isTraversing = 0;
	    stopPID();
	    undoRecentWalls();
	    HAL_Delay(1000);
	    return 1;
    }
    return 0;
}

void move(int8_t n) {
	resetPID();
	int goal = CELL_ENCODER_COUNT * n;
	if (n >= 3){
		goal = goal + (n - 3) * SPEEDADDITIONAL;
	}
	setPIDGoalD(goal);
	correctionCounter += n;

	startPID();         // SysTick starts calling updatePID()

	uint32_t start_time = HAL_GetTick();

	while (!PIDdone()) {
		BNO085_Update(&mouse_imu);
		if (detectButton()){
			break;
		}

		// 2. Check if 5000 ms (5 seconds) have elapsed
		if ((HAL_GetTick() - start_time) > 2000) {
		    break; // Timeout reached, escape the loop!
		}
	}

	stopPID();         // SysTick stops calling updatePID()
	//delayMicroseconds(50000);
	resetPID();
}

/*
 * We recommend you implement this function so that turn(1) turns your rat 90 degrees in your positive rotation
 * direction and turn(-1) turns the other way.
 */
void turn(int8_t n) {
	resetPID();

	turnRelative(-1 * n);
	correctionCounter ++;

	startPID();

	while (!PIDdone()) {
		BNO085_Update(&mouse_imu);
		if (detectButton()){
			break;
		}
	}

	stopPID();
	//delayMicroseconds(50000);
	resetPID();
}

void stupidPath(void){
	move(1);
	turn(90);
	move(1);
	turn(90);
	move(1);
	turn(-90);
	move(1);
	move(1);
	turn(-90);
	move(1);
	turn(-90);
	move(1);
	turn(90);
	turn(90);
	move(1);
	turn(90);
	move(1);
	turn(90);
	move(1);
	move(1);
	turn(90);
	move(1);
	turn(-90);
	move(1);
	turn(90);
	move(1);
	move(1);
	turn(90);
	move(1);
	turn(-90);
	move(1);
	turn(90);
	move(1);
	turn(90);
	move(1);
	turn(-90);
	move(1);
	turn(90);
	move(1);
	turn(90);
	move(1);
}

void adjustment(void){
    setMotorLPWM(0.25);
    setMotorRPWM(0.25);
    HAL_Delay(1000);
    resetMotors();
    setMotorLPWM(-0.25);
    setMotorRPWM(-0.25);
    HAL_Delay(200);
    resetMotors();

    correctionCounter = 0;
}
