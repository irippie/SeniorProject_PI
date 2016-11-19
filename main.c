#include "main.h"

#define Kp_motor 13.0
#define Ki_motor 0.8
#define Kd_motor 1.0

float i_term;
float last_error = 0;

int main(void){
	/*TODO: Need to toggle pins here, sometimes SDA being pulled low on restart*/

	mpu9250 imu;
	if(!init_all(&imu))
		return 0;
//	stabilize_imu(&imu);
//
//	int pitch;
//	move_forward(35);
	MAP_GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0);
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);
	while(1){}
//	while(1){
//		pitch = get_pitch(&imu) + 1;
//
////	    pid_0(pitch);
//
//	    my_itoa(pitch);
//	}
}

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
	last_error = error;
	d_term = Kd_motor*d_term;
	if(d_term > 100){
		d_term = 100;
	}
	else if(d_term < -100){
		d_term = -100;
	}


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
		tx_data("> 100");
	}
}

void stabilize_imu(mpu9250* imu){
	int i;
	for(i = 0; i < 1000; i++){
		setAccelData(imu);
		setGyroData(imu);
		setMagData(imu);
		updateTime(imu);
		MadgwickQuaternionUpdate(imu->ax, imu->ay, imu->az, imu->gx*DEG_TO_RAD,
									 imu->gy*DEG_TO_RAD, imu->gz*DEG_TO_RAD, imu->my,
									 imu->mx,	imu->mz, imu->deltat);
		imu->pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ() * *(getQ()+2)));
		imu->pitch = (imu->pitch*RAD_TO_DEG);
		imu->sumCount = 0;
		imu->sum = 0;
	}
}

int get_pitch(mpu9250* imu){
	setAccelData(imu);
	setGyroData(imu);
	setMagData(imu);
	updateTime(imu);
	MadgwickQuaternionUpdate(imu->ax, imu->ay, imu->az, imu->gx*DEG_TO_RAD,
								 imu->gy*DEG_TO_RAD, imu->gz*DEG_TO_RAD, imu->my,
								 imu->mx,	imu->mz, imu->deltat);
	imu->pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ() * *(getQ()+2)));
	imu->pitch = (imu->pitch*RAD_TO_DEG);
	imu->sumCount = 0;
	imu->sum = 0;
	return imu->pitch;
}

int init_all(mpu9250* imu){

	init_clock();
	init_i2c();
	init_uart();

	init_struct(imu);
	uint8_t who_is_it = read_i2c(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
	if(who_is_it != 0x71)
		return 0;

	/* PID timer */
	MAP_Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);
	MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);

	/* PWM timer for motors */
	init_PWM_timers();

	return 1;
}



