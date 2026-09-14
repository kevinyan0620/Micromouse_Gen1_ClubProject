/*
 * pid.c
 */

#include "main.h"
#include "motors.h"
#include "encoders.h"
#include "delay.h"
#include "gyroscope.h"

// --- Tuning Parameters (adjust these) ---
#define KPW         0.0087   // Angle proportional 0.008, Works good: 0.009 Works even better: 0.0087
#define KDW         0.09    // Angle derivative 0.055, Works good: 0.065 Works even better:0.063

#define KIW         0.00005f         // Start incredibly small! It compounds 1000x a second. //0.0001
#define MAX_I_TERM  0.250f    // The absolute maximum PWM the I-term is allowed to contribute //0.15

//KDW 0.055
#define KPD         0.002  // Distance proportional
#define KDD         0.0063   // Distance derivative
#define DIST_THRESH 30       // Encoder counts "close enough" to goal originally: 25
#define ANG_THRESH  7		// Originally: 10
#define DONE_CYCLES 20     // Must be stable for this many cycles Originally: 50

// --- Acceleration Parameters ---
#define MIN_START_SPD   0.15f   // Minimum PWM to overcome motor friction
#define MAX_SPD         1.0f
#define MAX_TURN_SPD    0.8f    // Maximum cruise speed for pure turning
#define ACCEL_DIST      0.005f  // Add 0.005 PWM every 1ms for distance
#define ACCEL_ANGLE     0.01f   // Add 0.01 PWM every 1ms for turning

// --- Speed Limit State Variables ---
static float currentDistSpeedLimit  = MIN_START_SPD;
static float currentAngleSpeedLimit = MIN_START_SPD;

// --- Global Variables ---
extern BNO085_t mouse_imu;
extern volatile float initial_yaw;

// --- Goal Variables ---
static int32_t goalDistance = 0;
static float   goalAngle    = 0.0f;

// --- Error Variables ---
static float angleError       = 0;
static float oldAngleError    = 0;
static float distanceError    = 0;
static float oldDistanceError = 0;
static float angleErrorSum    = 0.0f;

// --- Done Tracking ---
static int     stableCount = 0;
static int8_t  isDone      = 0;

// --- SysTick Flag ---
static volatile uint8_t pidRunning = 0;

// -------------------------------------------------------

uint8_t getPidRunning(void)
{
    return pidRunning;
}

void resetPID()
{
    goalDistance     = 0;
    //goalAngle        = 0;
    angleError       = 0;
    oldAngleError    = 0;
    distanceError    = 0;
    oldDistanceError = 0;
    stableCount      = 0;
    isDone           = 0;

    setMotorLPWM(0);
    setMotorRPWM(0);
    resetEncoders();

    // Reset acceleration limits!
    currentDistSpeedLimit  = MIN_START_SPD;
    currentAngleSpeedLimit = MIN_START_SPD;

    angleErrorSum = 0.0f;
}

void setPIDGoalD(int16_t distance)
{
    goalDistance = distance;
    currentDistSpeedLimit = MIN_START_SPD;
}

void setPIDGoalA(int16_t angle)
{
	goalAngle = initial_yaw + (float)angle;
	currentAngleSpeedLimit = MIN_START_SPD;
}

void updatePID()
{
    int32_t leftCounts  = getLeftEncoderCounts();
    int32_t rightCounts = getRightEncoderCounts();
    int32_t avgCounts   = (leftCounts + rightCounts) / 2;

    // Get live world telemetry
    float currentYaw = BNO085_GetAngle(&mouse_imu, ANGLE_YAW);

    // --- Angle Error ---
    oldAngleError = angleError;
    angleError    = currentYaw - goalAngle;

    // --- Distance Error ---
    oldDistanceError = distanceError;
    distanceError    = (float)(goalDistance - avgCounts);

    // Normalize error boundary loop (-180 to +180)
    // This calculates the shortest steering rotation to hit the relative goal
    while (angleError > 180.0f)  angleError -= 360.0f;
    while (angleError < -180.0f) angleError += 360.0f;

    // --- PD Corrections ---
    float dAngleError = angleError - oldAngleError;

    // Normalize the derivative boundary loop so the math never spikes across the 180 line
    while (dAngleError > 180.0f)  dAngleError -= 360.0f;
    while (dAngleError < -180.0f) dAngleError += 360.0f;

    angleErrorSum += angleError;
	// Calculate what the I-term's PWM contribution currently is
	float i_term_pwm = KIW * angleErrorSum;

	// If it exceeds our safety limit, clamp the sum itself
	if (i_term_pwm > MAX_I_TERM) {
		angleErrorSum = MAX_I_TERM / KIW;
	} else if (i_term_pwm < -MAX_I_TERM) {
		angleErrorSum = -MAX_I_TERM / KIW;
	}

	// Optional but highly recommended: Zero the integral if we cross the target
	// to prevent overshooting caused by old accumulated error
	if ((angleError > 0 && oldAngleError < 0) || (angleError < 0 && oldAngleError > 0)) {
		angleErrorSum = 0.0f;
	}

	// --- 3. Final PID Equation ---
	float angleCorrection = (KPW * angleError) + (KIW * angleErrorSum) + (KDW * dAngleError);
    float distanceCorrection = KPD * distanceError + KDD * (distanceError - oldDistanceError);

    // ---------------------------------------------------------
    // --- ACCELERATION PROFILING (SLEW RATE LIMITING) ---
    // ---------------------------------------------------------

	// Ramp up Distance Speed Limit
	if (currentDistSpeedLimit < MAX_SPD) {
		currentDistSpeedLimit += ACCEL_DIST;
	}

	// Ramp up Angle Speed Limit
	if (currentAngleSpeedLimit < MAX_TURN_SPD) {
		currentAngleSpeedLimit += ACCEL_ANGLE;
	}

	// Clamp distanceCorrection to the ramping limit
	if (distanceCorrection >  currentDistSpeedLimit) distanceCorrection =  currentDistSpeedLimit;
	if (distanceCorrection < -currentDistSpeedLimit) distanceCorrection = -currentDistSpeedLimit;

	// Clamp angleCorrection to the ramping limit
	if (angleCorrection >  currentAngleSpeedLimit) angleCorrection =  currentAngleSpeedLimit;
	if (angleCorrection < -currentAngleSpeedLimit) angleCorrection = -currentAngleSpeedLimit;

	// ---------------------------------------------------------

    // --- Motor Mixing ---
    float leftPWM  = distanceCorrection + angleCorrection;
    float rightPWM = distanceCorrection - angleCorrection;


    setMotorLPWM(leftPWM);
    setMotorRPWM(rightPWM);

    // --- Done Detection ---
    if (distanceError > -DIST_THRESH && distanceError < DIST_THRESH &&
        angleError    > -ANG_THRESH  && angleError    < ANG_THRESH)
    {
        stableCount++;
        if (stableCount >= DONE_CYCLES)
            isDone = 1;
    }
    else
    {
        stableCount = 0;
        isDone      = 0;
    }
}

void startPID(void)
{
    pidRunning = 1;
}

void stopPID(void)
{
    pidRunning = 0;
}

int8_t PIDdone(void)
{
    return isDone;
}

void turnRelative(int16_t delta_angle)
{
    // Add the turn to your current target angle
    goalAngle += (float)delta_angle;

    // Keep the goal angle bounded between -180 and +180
    // so it matches the raw output range of the gyroscope
    while (goalAngle > 180.0f)  goalAngle -= 360.0f;
    while (goalAngle < -180.0f) goalAngle += 360.0f;

    angleErrorSum = 0.0f;
}
