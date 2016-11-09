#include "driverlib.h"
#include <stdint.h>
#include <stdlib.h>
#include "quaternionFilters.h"
#include <math.h>
#include "i2c.h"
#include "motor_driver.h"
#include "msp_uart.h"
#include "pid.h"

#include "mpu9250.h"

#define Kp_motor 13.0 //13.0
#define Ki_motor 0.8
#define Kd_motor 1.0

#define pitch_offset 3
/////////////////////////////////////////////////////////////////////
// @brief initliazes clock to 24 MHz
/////////////////////////////////////////////////////////////////////
void init_clock();
/////////////////////////////////////////////////////////////////////
// @brief sends out pitch data via UART
// @param1 float pitch angle casted to int (-90, 90)
/////////////////////////////////////////////////////////////////////
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

float i_term;



int main(void){ //changing to int main function to break if who_am_i doens't return valid value

	/*TODO: Need to toggle pins here, sometimes SDA being pulled low on restart*/
	//above could be occuring because of timing issue seen at 400 kbps?
	//timing issues occur at 400 kbps but not 100 kbps
	//transmit mode seems to handle what we need think it resets the bit we need

	init_clock();
	//TODO: remove from code, testing clocks
//	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
////	MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
//	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
//	while(1){
//
//	}

	init_i2c(); //could possibily pull this into the mpu function set in order to abstract some functions away
	init_uart();

	mpu9250 my_MPU;
	init_struct(&my_MPU);





	uint8_t who_is_it = read_i2c(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
	if(who_is_it != 0x71){ //if who am i doesn't return 71, program exits
		return 0;
	}


	//following 3 lines are for input capture for enconders
	//TODO: need to do input capture for 2.5
	//currently only using 2.4 to ensure works

//	init_encoder_capture();
//	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2,
//				GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
//	MAP_Timer_A_initCapture(TIMER_A0_BASE, &captureModeConfig);
	/* Configuring Continuous Mode */
	MAP_Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);
//	MAP_Interrupt_enableInterrupt(INT_TA0_N);
//	MAP_Interrupt_enableMaster();
	//init_TimerA();
	MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
	init_PWM_timers(); //sets up timers for PWM output for motors
	while(1){
		//updating MPU values and setting pitch angle
		setAccelData(&my_MPU);
		setGyroData(&my_MPU);
		setMagData(&my_MPU);
		updateTime(&my_MPU);
		MadgwickQuaternionUpdate(my_MPU.ax, my_MPU.ay, my_MPU.az, my_MPU.gx*DEG_TO_RAD,
									 my_MPU.gy*DEG_TO_RAD, my_MPU.gz*DEG_TO_RAD, my_MPU.my,
									 my_MPU.mx,	my_MPU.mz, my_MPU.deltat);
		my_MPU.pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ() *
		                    *(getQ()+2)));

	    my_MPU.pitch = (my_MPU.pitch*RAD_TO_DEG);
	    my_MPU.sumCount = 0;
	    my_MPU.sum = 0;

//	    move_forward(30);


	    int pitch = my_MPU.pitch + 5;

	    if(pitch > 0){
//	    	tx_data("fuck");
	    }
	    else{
//	    	tx_data("shit");
	    }
	    pid_0(pitch);
//	    move_forward(50);
	    //outputting current pitch via UART
	    my_itoa(pitch);
	}
}

float last_error = 0;
void pid_0(int pitch){
	float error = 0 - pitch;
	float p_term = Kp_motor*error;
	i_term += Ki_motor*(float)error;
	if(i_term > 100)
		i_term = 100;
	else if(i_term < -100)
		i_term = -100;
	if(p_term > 100){
		p_term = 100;
	}
	else if(p_term < -100){
		p_term = -100;
	}
	float d_term = error - last_error;
	d_term = Kd_motor*d_term;
	if(d_term > 100){
		d_term = 100;
	}
	else if(d_term < -100){
		d_term = -100;
	}
//	if(p_term < 0){
//		if(p_term <= -100){
//			p_term = 100;
//		}
//		else{
//			p_term *= -1;
//		}
////		move_forward(p_term);
//	}
//	else{
//		if(p_term > 100){
//			p_term = 100;
//		}
////		move_reverse(p_term);
//
//	}
	float output = (float)p_term + i_term + d_term;
	int motor_set = (int)output;
	if(motor_set < 0){
		if(motor_set < -100){
			motor_set = 100;
		}
		else{
			motor_set *= -1;
		}
		move_reverse(motor_set);
	}
	else{
		if(motor_set > 100){
			motor_set = 100;
		}
		move_forward(motor_set);


	}

}

void init_clock(){

	/* Halting WDT  */
	MAP_WDT_A_holdTimer();

	/* Enable floating point unit to set DCO frequency */
	MAP_FPU_enableModule();

	MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
	MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

	/* Increasing core voltage to handle higher frequencies */
	MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

	/* Setting DCO to 24MHz */
	MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24);

	/* Setting P4.3 to output MCLK frequency */
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

}

void my_itoa(int value){
//	int speed;
//	if(rising_VAL > falling_VAL){
//		speed = rising_VAL - falling_VAL;
//	}
//	else{
//		speed = falling_VAL - rising_VAL;
//	}
//	char s[5];
//	s[0] = (speed/1000) + 0x30;
//	speed = speed - (speed/10)*10;
//	s[1] = (speed/100) + 0x30;
//	speed = speed - (speed/10)*10;
//	s[2] = (speed/10) + 0x30;
//	speed = speed - (speed/10)*10;
//	s[3] = speed + 0x30;
//	tx_data(s);
	int temp = value;
	char c[3];
	if(value < 0){
		c[0] = '-';
		temp = -temp;
	}
	else{
		c[0] = '+';
	}
	c[1] = (temp/10) + 0x30;
	temp = temp - (temp/10)*10;
	c[2] = temp + 0x30;

	if(value > 0){
		tx_data(c);
	}
	else{
		tx_data(c);
	}
	if(value > 100){
		tx_data("shits_fucked");
	}

}

void TA0_N_IRQHandler(void){
	MAP_Timer_A_clearInterruptFlag(TIMER_A0_BASE);
	MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
	            TIMER_A_CAPTURECOMPARE_REGISTER_1);
	uint8_t pin_value = P2IN & 0x10;
	if(pin_value != 0){
		rising_VAL =  MAP_Timer_A_getCaptureCompareCount(TIMER_A0_BASE,
									TIMER_A_CAPTURECOMPARE_REGISTER_1);
	}
	else{
		falling_VAL =  MAP_Timer_A_getCaptureCompareCount(TIMER_A0_BASE,
											TIMER_A_CAPTURECOMPARE_REGISTER_1);
	}
}
