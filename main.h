/*
 * main.h
 *
 *  Created on: Nov 13, 2016
 *      Author: Jesus
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "driverlib.h"
#include <stdint.h>
#include <stdlib.h>
#include "quaternionFilters.h"
#include <math.h>
#include "i2c.h"
#include "motor_driver.h"
#include "msp_uart.h"
#include "mpu9250.h"

int init_all(mpu9250*);
void init_clock();
void stabilize_imu(mpu9250*);
int get_pitch(mpu9250*);
void my_itoa(int);

const Timer_A_ContinuousModeConfig continuousModeConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,           // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_24,       // SMCLK = 24 MHz/24 = 1MHz
        TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer ISR
        TIMER_A_SKIP_CLEAR                   // Skup Clear Counter
};

const Timer_A_CaptureModeConfig captureModeConfig =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_1,        // CC Register 1
        TIMER_A_CAPTUREMODE_RISING_AND_FALLING_EDGE,          // Rising Edge
        TIMER_A_CAPTURE_INPUTSELECT_CCIxA,        // CCIxA Input Select
        TIMER_A_CAPTURE_SYNCHRONOUS,              // Synchronized Capture
        TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,  // Enable interrupt
        TIMER_A_OUTPUTMODE_OUTBITVALUE            // Output bit value
};

#endif /* MAIN_H_ */
