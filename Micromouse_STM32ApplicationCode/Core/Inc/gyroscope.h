/*
 * gyroscope.h
 *
 *  Created on: May 22, 2026
 *      Author: guanlinyan565
 */

#ifndef INC_GYROSCOPE_H_
#define INC_GYROSCOPE_H_

#include "main.h" // Gives access to HAL and I2C handles

// Enum to easily select which axis angle you want
typedef enum {
    ANGLE_YAW = 0,
    ANGLE_PITCH = 1,
    ANGLE_ROLL = 2
} AngleType_t;

// The "Class" structure containing the object variables
typedef struct {
    I2C_HandleTypeDef *hi2c;  // Pointer to the STM32 I2C peripheral
    uint16_t dev_address;     // I2C address of the BNO085
    float yaw;
    float pitch;
    float roll;
    uint32_t last_poll_time;  // For non-blocking 20ms polling
} BNO085_t;

// "Methods" (Functions associated with the BNO085 class)
void BNO085_Init(BNO085_t *imu, I2C_HandleTypeDef *hi2c_handle, uint16_t address);
void BNO085_Update(BNO085_t *imu);
float BNO085_GetAngle(BNO085_t *imu, AngleType_t axis);

#endif /* INC_GYROSCOPE_H_ */
