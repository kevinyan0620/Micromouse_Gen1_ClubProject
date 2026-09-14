/*
 * irs.h
 */

#ifndef INC_IRS_H_
#define INC_IRS_H_

#include <stdint.h>

// The number of samples to take
#define NUM_SAMPLES 128

#define LEFT_IR_TH 387  //342
#define FRONTLEFT_IR_TH 1000  //1016
#define FRONTRIGHT_IR_TH 4013 //4016
#define RIGHT_IR_TH 540  //390

// Using this enumeration makes the code more readable
typedef enum
{
	IR_LEFT = 0,
	IR_FRONT_LEFT = 1,
	IR_FRONT_RIGHT = 2,
	IR_RIGHT = 3,
	IR_LEFT45 = 4,
	IR_RIGHT45 = 5
}IR;

uint16_t readIR(IR ir);
uint16_t readLeftIR(void);
uint16_t readFrontLeftIR(void);
uint16_t readFrontRightIR(void);
uint16_t readRightIR(void);
uint16_t readLeft45IR(void);
uint16_t readRight45IR(void);
uint16_t analogRead(IR ir);

void turnOnIR(IR ir);
void turnOffIR(IR ir);

uint16_t isWallLeft(void);
uint16_t isWallFront(void);
uint16_t isWallRight(void);

#endif /* INC_IRS_H_ */
