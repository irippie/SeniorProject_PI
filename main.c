#include "main.h"

float kp = 32.5; 		// 32.5
float ki = 0.9;		// 0.9
float kd = 500;		// 7500
float* kptr = &kp;
float i_term;
float d_term;
float last_error = 0;
float setpoint = 0;

int main(void){
	/*TODO: Need to toggle pins here, sometimes SDA being pulled low on restart*/

	MAP_GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0);
	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);

	mpu9250 imu;
	if(!init_all(&imu))
		return 0;
	stabilize_imu(&imu);


	float pitch;
	char rx_data;
	uint32_t status;

	while(1){
		status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
		MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

		if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
		{
			rx_data = MAP_UART_receiveData(EUSCI_A2_BASE);
			char *test = &rx_data;
		}

		switch(rx_data){
			case 'p':
				kptr = &kp;
				tx_data("kp = ");
				my_itoa(kp);
				break;
			case 'i':
				kptr = &ki;
				tx_data("ki = ");
				my_itoa(ki);
				break;
			case 'd':
				kptr = &kd;
				tx_data("kd = ");
				my_itoa(kd);
				break;
			case '8':
				*kptr += 1.0;
				rx_data = 'x';
				break;
			case '7':
				*kptr -= 1.0;
				rx_data = 'x';
				break;
			case '5':
				*kptr += 0.1;
				rx_data = 'x';
				break;
			case '4':
				*kptr -= 0.1;
				rx_data = 'x';
				break;
			case '2':
				*kptr += 0.01;
				rx_data = 'x';
				break;
			case '1':
				*kptr -= 0.01;
				rx_data = 'x';
				break;

		}

		pitch = get_pitch(&imu);
		pid(pitch);
	}

}

void pid(float pitch){

	float error = setpoint - pitch;
	float p_term = kp * error;
	i_term += ki*error;

	if(i_term > 100)
		i_term = 100;
	else if(i_term < -100)
		i_term = -100;

	if(p_term > 100)
		p_term = 100;
	else if(p_term < -100)
		p_term = -100;

	d_term = error - last_error;
	last_error = error;
	d_term = kd * d_term;

	if(d_term > 100)
		d_term = 100;
	else if(d_term < -100)
		d_term = -100;

	float output = p_term + i_term + d_term;
	int motor_set = (int)output;

	if(motor_set < 0){
		motor_set *= -1;
		if(motor_set > 100)
			motor_set = 100;
		move_forward(motor_set);
	}
	else if(motor_set > 0){
		if(motor_set > 100){
			motor_set = 100;
		}
		move_reverse(motor_set);
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

void my_itoa(float value){

	float temp = value;
	char c[4];

	c[3] = (int)(temp/10) + 0x30; 			// tens
	temp = temp - (int)(temp/10)*10;		// ones
	c[2] = (int)temp + 0x30;
	temp = (temp - (int)(temp))*10;			// tenth
	c[1] = (int)temp + 0x30;
	temp = ((temp - (int)temp))*10;
	c[0] = (int)temp + 0x30;				// hundredth

	tx_char(c[3]);
	tx_char(c[2]);
	tx_char('.');
	tx_char(c[1]);
	tx_char(c[0]);
	tx_char('\r');

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

float get_pitch(mpu9250* imu){
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



