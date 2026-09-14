/*
 * irs.c
 */

#include "main.h"
#include "irs.h"
#include "delay.h"

// This is the buffer that will get filled up with all the measurements
uint16_t adc_buf[NUM_SAMPLES];
// "boolean" variable to keep say when the ADC has finished filling up the buffer
volatile uint8_t complete = 0;

/*
 This function should handle everything for reading a specific IR
 First turn on the correct IR emitter
 Wait for a small amount of time (at least 20 us) so the photodiode can react
 Then read the correct receiver
 Lastly turn off the emitter
 */
uint16_t readIR(IR ir)
{
	int value = 0;
	switch (ir){
	case IR_FRONT_LEFT:
		turnOnIR(ir);
		delayMicroseconds(20);
		value = readFrontLeftIR();
		turnOffIR(ir);
		break;
	case IR_FRONT_RIGHT:
		turnOnIR(ir);
		delayMicroseconds(20);
		value = readFrontRightIR();
		turnOffIR(ir);
		break;
	case IR_LEFT45:
		turnOnIR(ir);
		delayMicroseconds(20);
		value = readLeft45IR();
		turnOffIR(ir);
		break;
	case IR_RIGHT45:
		turnOnIR(ir);
		delayMicroseconds(20);
		value = readRight45IR();
		turnOffIR(ir);
		break;
	case IR_LEFT:
		turnOnIR(ir);
		delayMicroseconds(20);
		value = readLeftIR();
		turnOffIR(ir);
		break;
	case IR_RIGHT:
		turnOnIR(ir);
		delayMicroseconds(20);
		value = readRightIR();
		turnOffIR(ir);
		break;
	default:
		value = 67;
	}
	return value;
}

/*
 Implement the following 4 functions to read each IR
 Hint: Each function may just be one line
 */
uint16_t readLeftIR(void)
{
	return analogRead(IR_LEFT);
}

uint16_t readFrontLeftIR(void)
{
	return analogRead(IR_FRONT_LEFT);
}

uint16_t readFrontRightIR(void)
{
	return analogRead(IR_FRONT_RIGHT);
}


uint16_t readRightIR(void)
{
	return analogRead(IR_RIGHT);
}

uint16_t readLeft45IR(void)
{
	return analogRead(IR_LEFT45);
}

uint16_t readRight45IR(void)
{
	return analogRead(IR_RIGHT45);
}

/*
 This function reads the specific channel of the ADC corresponding to the correct IR
 You should not have to edit this function
 */
uint16_t analogRead(IR ir)
{
    ADC_ChannelConfTypeDef sConfig = {0}; //this initializes the IR ADC [Analog to Digital Converter]
    ADC_HandleTypeDef *hadc1_ptr = Get_HADC1_Ptr(); //this is a pointer to your hal_adc
    //this pointer will also be used to read the analog value, val = HAL_ADC_GetValue(hadc1_ptr);

    //this picks the IR direction to choose the right ADC.
    /*Left Receiver = 9
    Left 45 Receiver = 8
    Front Left = 14
    Front Right = 6
    Right 45 = 4
    Right = 3
    */
    switch(ir)
    {
        case IR_LEFT:
            sConfig.Channel = ADC_CHANNEL_9;
            break;
        case IR_FRONT_LEFT:
            sConfig.Channel = ADC_CHANNEL_14;
            break;
        case IR_FRONT_RIGHT:
            sConfig.Channel = ADC_CHANNEL_6;
            break;
        case IR_RIGHT:
            sConfig.Channel = ADC_CHANNEL_3;
            break;
        case IR_LEFT45:
        	sConfig.Channel = ADC_CHANNEL_8;
        	break;
        case IR_RIGHT45:
            sConfig.Channel = ADC_CHANNEL_4;
            break;
        default:
            return 0;
    }

    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

    // make sure everything was set up correctly
    if (HAL_ADC_ConfigChannel(hadc1_ptr, &sConfig) != HAL_OK)
    {
        return 0;
    }

    complete = 0;

    // start filling up the ADC buffer
    HAL_ADC_Start_DMA(hadc1_ptr, (uint32_t*)adc_buf, NUM_SAMPLES);

    // wait for the buffer to become full
    while (complete == 0)
    {
        continue;
    }

    uint32_t sum = 0;
    // calculate the sum of the measurements in order to calculate the average
    uint16_t measurement = 0;
    while(measurement < NUM_SAMPLES) //this takes multiple measurements
    {
        sum += adc_buf[measurement];
        ++measurement;
    }

    return sum/NUM_SAMPLES;
}

/*
 This function is called when the ADC buffer is filled
 It stops the ADC and changes our "complete" variable to be "true"
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    // stop the ADC
    HAL_ADC_Stop_DMA(hadc);
    complete = 1;
}

void turnOnIR(IR ir)
{
    switch (ir)
    {
		case IR_LEFT:
			HAL_GPIO_WritePin(LeftEmitter_GPIO_Port, LeftEmitter_Pin, GPIO_PIN_SET);
			break;
		case IR_RIGHT:
			HAL_GPIO_WritePin(RightEmitter_GPIO_Port, RightEmitter_Pin, GPIO_PIN_SET);
			break;
		case IR_FRONT_LEFT:
			HAL_GPIO_WritePin(FrontLeftEmitter_GPIO_Port, FrontLeftEmitter_Pin, GPIO_PIN_SET);
			break;
		case IR_FRONT_RIGHT:
			HAL_GPIO_WritePin(FrontRightEmitter_GPIO_Port, FrontRightEmitter_Pin, GPIO_PIN_SET);
			break;
		case IR_LEFT45:
			HAL_GPIO_WritePin(Left45Emitter_GPIO_Port, Left45Emitter_Pin, GPIO_PIN_SET);
			break;
		case IR_RIGHT45:
			HAL_GPIO_WritePin(Right45Emitter_GPIO_Port, Right45Emitter_Pin, GPIO_PIN_SET);
			break;
	}
    return;
}

void turnOffIR(IR ir)
{
    switch (ir)
	{
		case IR_LEFT:
			HAL_GPIO_WritePin(LeftEmitter_GPIO_Port, LeftEmitter_Pin, GPIO_PIN_RESET);
			break;
		case IR_RIGHT:
			HAL_GPIO_WritePin(RightEmitter_GPIO_Port, RightEmitter_Pin, GPIO_PIN_RESET);
			break;
		case IR_FRONT_LEFT:
			HAL_GPIO_WritePin(FrontLeftEmitter_GPIO_Port, FrontLeftEmitter_Pin, GPIO_PIN_RESET);
			break;
		case IR_FRONT_RIGHT:
			HAL_GPIO_WritePin(FrontRightEmitter_GPIO_Port, FrontRightEmitter_Pin, GPIO_PIN_RESET);
			break;
		case IR_LEFT45:
			HAL_GPIO_WritePin(Left45Emitter_GPIO_Port, Left45Emitter_Pin, GPIO_PIN_RESET);
			break;
		case IR_RIGHT45:
			HAL_GPIO_WritePin(Right45Emitter_GPIO_Port, Right45Emitter_Pin, GPIO_PIN_RESET);
			break;
	}
    return;
}

uint16_t isWallLeft(void){
	if (readIR(IR_LEFT) > LEFT_IR_TH)
		return 1;
	else
		return 0;
}

uint16_t isWallFront(void){
	if ((readIR(IR_FRONT_LEFT) > FRONTLEFT_IR_TH) || (readIR(IR_FRONT_RIGHT) > FRONTRIGHT_IR_TH))
		return 1;
	else
		return 0;
}

uint16_t isWallRight(void){
	if (readIR(IR_RIGHT) > RIGHT_IR_TH)
		return 1;
	else
		return 0;
}
