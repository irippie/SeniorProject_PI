#include "driverlib.h"
#include <stdint.h>
#include "i2c.h"
#include "mpu9250.h"



void init_clock();

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
	write_i2c(MPU9250_ADDRESS, 0x37, 0x02);
	write_i2c(MPU9250_ADDRESS, 0x6A, 0x01);
	who_is_it = read_i2c(0x0C, 0);
	float test[3];
//	calibrateMPU(&my_MPU);
//	initMAG(&my_MPU, test);
	while(1){


		setAccelData(&my_MPU);
		setGyroData(&my_MPU);
//		uint8_t test = read_i2c(AK8963_ADDRESS, 0); //should be 0x48
		setMagData(&my_MPU);

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
