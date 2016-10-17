#include "driverlib.h"
#include <stdint.h>
#include "mpu9250.h"

#define SLAVE_ADDRESS 0x68

void init_i2c();
void init_clock();
uint8_t read_i2c(uint8_t);
void write_i2c(uint8_t, uint8_t);
//TODO: need multi-byte read????
void read_multibyte_i2c(uint8_t, uint8_t, uint8_t*);



/* I2C Master Configuration Parameter */
const eUSCI_I2C_MasterConfig i2cConfig =
{
	EUSCI_B_I2C_CLOCKSOURCE_SMCLK, 		// SMCLK Clock Source
	24000000,		 					// SMCLK = 24MHz
	EUSCI_B_I2C_SET_DATA_RATE_400KBPS, 	// Desired I2C Clock of 400khz
	0, 									// No byte counter threshold
	EUSCI_B_I2C_NO_AUTO_STOP 			// No Autostop
};

void main(void)
{
	/*TODO: Need to toggle pins here, sometimes SDA being pulled low on restart*/
	//above could be occuring because of timing issue seen at 400 kbps?
	//timing issues occur at 400 kbps but not 100 kbps
	//transmit mode seems to handle what we need think it resets the bit we need

	init_clock();
	init_i2c();
//	uint32_t temp;
	uint8_t value1;
//	write_i2c(0x68,0x71);
	uint8_t accel_data[6];

	while(1)
	{
		//slight bit of example code to show functionality
		value1 = read_i2c(WHO_AM_I_MPU9250); //I think
		read_multibyte_i2c(ACCEL_XOUT_H, 6, accel_data);
	}
}

uint8_t read_i2c(uint8_t register_addr){

	MAP_I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	/* Wait until ready */
	MAP_I2C_masterSendStart(EUSCI_B0_BASE);

	/* Send start bit and register */
	MAP_I2C_masterSendMultiByteNext(EUSCI_B0_BASE,register_addr);

	/* Wait for tx to complete */
	while(!(MAP_I2C_getInterruptStatus(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0) &
		EUSCI_B_I2C_TRANSMIT_INTERRUPT0));

	/* Turn off TX and generate RE-Start */
	MAP_I2C_masterReceiveStart(EUSCI_B0_BASE);

	/* Wait for start bit to complete */
	while(MAP_I2C_masterIsStartSent(EUSCI_B0_BASE));

	uint8_t data = MAP_I2C_masterReceiveMultiByteFinish(EUSCI_B0_BASE);
	/* Read one or more bytes */

	MAP_I2C_clearInterruptFlag(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

	return data;
}

//TODO: needs testing
void read_multibyte_i2c(uint8_t register_addr, uint8_t num_bytes, uint8_t * data){
	int i;
	for(i = 0; i < num_bytes; i++){
		data[i] = read_i2c(register_addr++);
	}
}

void write_i2c(uint8_t register_addr, uint8_t register_data){

	MAP_I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	/* Wait until ready */
	MAP_I2C_masterSendStart(EUSCI_B0_BASE);

	/* Send start bit and register */
	MAP_I2C_masterSendMultiByteNext(EUSCI_B0_BASE,register_addr);

	/* Send byte of data to write to register */
	MAP_I2C_masterSendMultiByteFinish(EUSCI_B0_BASE,register_data);

	MAP_I2C_clearInterruptFlag(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
}

void init_i2c(){

	MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN7);
	MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN7);

	/* Setting P1.6 & P1.7 as I2C */
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1,
				GPIO_PIN6 + GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

	/* Disabling I2C module to clear all I2C registers before initializing */
	MAP_I2C_disableModule(EUSCI_B0_BASE);

	/* Initializing I2c Master to SMCLK at 400kbs with no autostop */
	MAP_I2C_initMaster(EUSCI_B0_BASE, &i2cConfig);

//	EUSCI_B0_UCBCTLW0_SPI = 0x003B;

	/* Specify slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, SLAVE_ADDRESS);

	/* Set master to transmit mode */
	MAP_I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	/* Enable I2C Module to start operations */
	MAP_I2C_enableModule(EUSCI_B0_BASE);

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
