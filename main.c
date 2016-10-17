#include "driverlib.h"
#include <stdint.h>
#include "i2c.h"
#include "mpu9250.h"



void init_clock();

int main(void)
{
	/*TODO: Need to toggle pins here, sometimes SDA being pulled low on restart*/
	//above could be occuring because of timing issue seen at 400 kbps?
	//timing issues occur at 400 kbps but not 100 kbps
	//transmit mode seems to handle what we need think it resets the bit we need

	init_clock();
	init_i2c();
//	uint32_t temp;
	uint8_t value1 = read_i2c(WHO_AM_I_MPU9250);
//	write_i2c(0x68,0x71);
//	uint8_t accel_data[6];
	mpu9250 my_MPU;
	init_struct(&my_MPU);
	if(value1 != 0x71){
		return 1;
	}
	while(1)
	{
		//slight bit of example code to show functionality
		setAccelData(&my_MPU);

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
