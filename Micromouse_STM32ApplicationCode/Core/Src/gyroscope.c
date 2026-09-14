/*
 * gyroscope.c
 *
 *  Created on: May 22, 2026
 *      Author: guanlinyan565
 */


#include "gyroscope.h"
#include <math.h>

/**
 * @brief Initializes the BNO085 instance and sends the SHTP activation command.
 */
void BNO085_Init(BNO085_t *imu, I2C_HandleTypeDef *hi2c_handle, uint16_t address) {
    // Assign object properties
    imu->hi2c = hi2c_handle;
    imu->dev_address = address;
    imu->yaw = 0.0f;
    imu->pitch = 0.0f;
    imu->roll = 0.0f;
    imu->last_poll_time = HAL_GetTick();

    // SHTP Command: Enable Game Rotation Vector (Report ID 0x08) at 50Hz (20,000us)
    uint8_t enable_cmd[21] = {
        21, 0, 2, 0,            // SHTP Header (Length 21, Channel 2)
        0xFD, 0x08,             // Set Feature Command (0xFD), Feature ID: Game Rotation Vector (0x08)
        0x00, 0x00, 0x00,       // Feature flags
        0x20, 0x4E, 0x00, 0x00, // Report Interval (20,000 microseconds in Hex, LSB first)
        0x00, 0x00, 0x00, 0x00, // Sensor-specific config
        0x00, 0x00, 0x00, 0x00
    };

    // Send configuration packet to the physical sensor
    HAL_I2C_Master_Transmit(imu->hi2c, imu->dev_address, enable_cmd, 21, 100);
}

/**
 * @brief Non-blocking check for new data. Reads and processes packets every 20ms.
 */
void BNO085_Update(BNO085_t *imu) {
    if (HAL_GetTick() - imu->last_poll_time >= 20) {
        imu->last_poll_time = HAL_GetTick();

        uint8_t raw_buffer[32] = {0};

        // Single transaction read to ensure data alignment without desync risks
        if (HAL_I2C_Master_Receive(imu->hi2c, imu->dev_address, raw_buffer, 32, 5) == HAL_OK) {
            uint8_t channel = raw_buffer[2];

            // Verify it is Channel 3 (Sensor Hub Reports)
            if (channel == 3) {
                // Scan buffer safely for the Report ID 0x08
                for (int i = 4; i < 20; i++) {
                    if (raw_buffer[i] == 0x08) {

                        // Raw 16-bit Quaternions live 4 bytes ahead of the Report ID
                        int16_t i_raw = (int16_t)(raw_buffer[i + 4] | (raw_buffer[i + 5] << 8));
                        int16_t j_raw = (int16_t)(raw_buffer[i + 6] | (raw_buffer[i + 7] << 8));
                        int16_t k_raw = (int16_t)(raw_buffer[i + 8] | (raw_buffer[i + 9] << 8));
                        int16_t r_raw = (int16_t)(raw_buffer[i + 10] | (raw_buffer[i + 11] << 8));

                        // Convert fixed Q-point 14 data to floating point numbers
                        float q_i = i_raw / 16384.0f;
                        float q_j = j_raw / 16384.0f;
                        float q_k = k_raw / 16384.0f;
                        float q_r = r_raw / 16384.0f;

                        // Math optimizations
                        float sqi = q_i * q_i;
                        float sqj = q_j * q_j;
                        float sqk = q_k * q_k;
                        float sqr = q_r * q_r;

                        // Calculate Euler angles in degrees and store inside the object
                        imu->yaw   = atan2f(2.0f * (q_i * q_j + q_k * q_r), (sqi - sqj - sqk + sqr)) * (180.0f / 3.14159265f);
                        imu->pitch = asinf(-2.0f * (q_i * q_k - q_j * q_r)) * (180.0f / 3.14159265f);
                        imu->roll  = atan2f(2.0f * (q_j * q_k + q_i * q_r), (-sqi - sqj + sqk + sqr)) * (180.0f / 3.14159265f);

                        break; // Successfully extracted payload, exit the search loop
                    }
                }
            }
        }
    }
}

/**
 * @brief Getter function to extract targeted axis angles.
 */
float BNO085_GetAngle(BNO085_t *imu, AngleType_t axis) {
    switch (axis) {
        case ANGLE_YAW:   return imu->yaw;
        case ANGLE_PITCH: return imu->pitch;
        case ANGLE_ROLL:  return imu->roll;
        default:          return 0.0f;
    }
}
