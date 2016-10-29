/*
 * motor_driver.h
 *
 *  Created on: Sep 14, 2016
 *      Author: Kyle
 */

#ifndef MOTOR_DRIVER_H_
#define MOTOR_DRIVER_H_

#define TIMER_PERIOD 1000
#define DUTY_CYCLE1 0
#define DUTY_CYCLE2 60
#define FORWARD 1
#define REVERSE 0



void move_reverse(uint8_t);
void stop_motors();
uint32_t speed_calc(uint8_t);
void init_PWM_timers();

void move_forward(uint8_t);

static volatile uint16_t * L_MOTOR_FORWARD = &TA1CCR1;
static volatile uint16_t * L_MOTOR_REV = &TA1CCR2;
static volatile uint16_t * R_MOTOR_FORWARD = &TA1CCR3;
static volatile uint16_t * R_MOTOR_REV = &TA1CCR3;




#endif /* MOTOR_DRIVER_H_ */
