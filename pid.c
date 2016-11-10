/*
 * pid.c
 *
 *  Created on: Oct 29, 2016
 *      Author: Kyle
 */

#include "pid.h"


void PID_finally(int setpoint_pitch, int pitch){
	int error = setpoint_pitch - pitch;
	int p_term = error;

}
