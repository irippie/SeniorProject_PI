#include "driverlib.h"
#include <stdint.h>
#include "quaternionFilters.h"
#include <math.h>
#include "i2c.h"

#include "mpu9250.h"



void init_clock();

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

	mpu9250 my_MPU;
	init_struct(&my_MPU);

	uint8_t who_is_it;

//	uint8_t accel_data[6];
	//slight bit of example code to show functionality
	who_is_it = read_i2c(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
	if(who_is_it != 0x71){
		return 0;
	}
//	write_i2c(MPU9250_ADDRESS, 0x37, 0x02);
//	write_i2c(MPU9250_ADDRESS, 0x6A, 0x01);
//	who_is_it = read_i2c(0x0C, 0);
//	float test[3];
//	selfTestMPU9250(&my_MPU.SelfTest);

//	calibrateMPU(&my_MPU.accelBias, &my_MPU.gyroBias);
//	write_i2c(MPU9250_ADDRESS, 0x37, 0x02);
//	write_i2c(MPU9250_ADDRESS, 0x6A, 0x01);
//	initMAG(&my_MPU, test);

	// TODO: move this into i2c header file to initialize offsets
	write_i2c(MPU9250_ADDRESS, ZA_OFFSET_H, 36);
	write_i2c(MPU9250_ADDRESS, ZA_OFFSET_L, 221);
	write_i2c(MPU9250_ADDRESS, XA_OFFSET_H, 21);
	write_i2c(MPU9250_ADDRESS, XA_OFFSET_L, 117);
	write_i2c(MPU9250_ADDRESS, YA_OFFSET_H, 26);
	write_i2c(MPU9250_ADDRESS, YA_OFFSET_L, 114);

	/* Configuring Continuous Mode */
	MAP_Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

	//init_TimerA();
	MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);

	uint8_t tempZH, tempZL, tempXH, tempXL, tempYH, tempYL;
	uint8_t more_temps[6];
	int16_t temp[3];
	while(1){

//		if(read_i2c(MPU9250_ADDRESS, INT_STATUS) & 0x01){
//			tempZH = read_i2c(MPU9250_ADDRESS, ACCEL_ZOUT_H);
//			tempZL = read_i2c(MPU9250_ADDRESS, ACCEL_ZOUT_L);
//			temp[0] = ((int16_t)tempZH << 8) | tempZL;
//			tempXH = read_i2c(MPU9250_ADDRESS, ACCEL_XOUT_H);
//			tempXL = read_i2c(MPU9250_ADDRESS, ACCEL_XOUT_L);
//			temp[1] = ((int16_t)tempXH << 8) | tempXL;
//			tempYH = read_i2c(MPU9250_ADDRESS, ACCEL_YOUT_H);
//			tempYL = read_i2c(MPU9250_ADDRESS, ACCEL_YOUT_L);
//			temp[2] = ((int16_t)tempYH << 8) | tempYL;
//			more_temps[0] = read_i2c(MPU9250_ADDRESS, XA_OFFSET_H);
//			more_temps[1] = read_i2c(MPU9250_ADDRESS, XA_OFFSET_L);
//			more_temps[2] = read_i2c(MPU9250_ADDRESS, YA_OFFSET_H);
//			more_temps[3] = read_i2c(MPU9250_ADDRESS, YA_OFFSET_L);
//			more_temps[4] = read_i2c(MPU9250_ADDRESS, ZA_OFFSET_H);
//			more_temps[5] = read_i2c(MPU9250_ADDRESS, ZA_OFFSET_L);
//		}
		setAccelData(&my_MPU);
		setGyroData(&my_MPU);
//		uint8_t test = read_i2c(AK8963_ADDRESS, 0); //should be 0x48
		setMagData(&my_MPU);

//		uint16_t current_time = TA0R;
		updateTime(&my_MPU);
//		MahonyQuaternionUpdate(my_MPU.ax, my_MPU.ay, my_MPU.az, my_MPU.gx,
//								 my_MPU.gy, my_MPU.gz, my_MPU.my,
//								 my_MPU.mx,	my_MPU.mz, my_MPU.deltat);
		MadgwickQuaternionUpdate(my_MPU.ax, my_MPU.ay, my_MPU.az, my_MPU.gx*DEG_TO_RAD,
									 my_MPU.gy*DEG_TO_RAD, my_MPU.gz*DEG_TO_RAD, my_MPU.my,
									 my_MPU.mx,	my_MPU.mz, my_MPU.deltat);
//		my_MPU.yaw   = atan2(2.0f * (*(getQ()+1) * *(getQ()+2) + *getQ() *
//		                    *(getQ()+3)), *getQ() * *getQ() + *(getQ()+1) * *(getQ()+1)
//		                    - *(getQ()+2) * *(getQ()+2) - *(getQ()+3) * *(getQ()+3));
		my_MPU.pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ() *
		                    *(getQ()+2)));
//		my_MPU.roll  = atan2(2.0f * (*getQ() * *(getQ()+1) + *(getQ()+2) *
//					*(getQ()+3)), *getQ() * *getQ() - *(getQ()+1) * *(getQ()+1)
//					- *(getQ()+2) * *(getQ()+2) + *(getQ()+3) * *(getQ()+3));

	    my_MPU.pitch = my_MPU.pitch*RAD_TO_DEG;
//	    my_MPU.yaw   *= RAD_TO_DEG;
//	    my_MPU.roll  *= RAD_TO_DEG;
	    my_MPU.sumCount = 0;
	    my_MPU.sum = 0;

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
