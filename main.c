#include "driverlib.h"
#include <stdint.h>
#include <stdlib.h>
#include "quaternionFilters.h"
#include <math.h>
#include "i2c.h"
#include "motor_driver.h"
#include "msp_uart.h"

#include "mpu9250.h"


#define pitch_offset 3

// @brief initliazes clock to 24 MHz
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

int main(void){ //changing to int main function to break if who_am_i doens't return valid value

	/*TODO: Need to toggle pins here, sometimes SDA being pulled low on restart*/
	//above could be occuring because of timing issue seen at 400 kbps?
	//timing issues occur at 400 kbps but not 100 kbps
	//transmit mode seems to handle what we need think it resets the bit we need

	init_clock();
	init_i2c(); //could possibily pull this into the mpu function set in order to abstract some functions away
	init_uart();

	mpu9250 my_MPU;
	init_struct(&my_MPU);





	uint8_t who_is_it = read_i2c(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
	if(who_is_it != 0x71){ //if who am i doesn't return 71, program exits
		return 0;
	}


	/* Configuring Continuous Mode */
	MAP_Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

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

	    my_MPU.pitch = (my_MPU.pitch*RAD_TO_DEG) - pitch_offset;
//	    my_MPU.yaw   *= RAD_TO_DEG;
//	    my_MPU.roll  *= RAD_TO_DEG;
	    my_MPU.sumCount = 0;
	    my_MPU.sum = 0;

	    if(my_MPU.pitch > 0){ //hopefully this works :)
	    	move_forward(40);
	    }
	    else{
	    	move_reverse(40);
	    }
	    int pitch = (int)my_MPU.pitch;
	    //outputting current pitch via UART
	    my_itoa(pitch);
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
		tx_data("shits_fucked");
	}

}
