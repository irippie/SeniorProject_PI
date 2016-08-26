//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "driverlib.h"
#include <stdint.h>

#define SLAVE_ADDRESS 0x68
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48

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

	/* Setting P1.6 & P1.7 as I2C */
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1,
	            GPIO_PIN6 + GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

	/* Disabling I2C module to clear all I2C registers before initializing */
	MAP_I2C_disableModule(EUSCI_B0_BASE);

	/* Initializing I2c Master to SMCLK at 400kbs with no autostop */
	MAP_I2C_initMaster(EUSCI_B0_BASE, &i2cConfig);

	/* Specify slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, SLAVE_ADDRESS);

	/* Set master to transmit mode */
	MAP_I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	/* Enable I2C Module to start operations */
	MAP_I2C_enableModule(EUSCI_B0_BASE);

//	MAP_I2C_disableInterrupt(EUSCI_B0_BASE);

	while(1)
	{
		MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, GYRO_ZOUT_H);
		MAP_I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_MODE);
		__delay_cycles(100000);
		__delay_cycles(100000);
		MAP_I2C_masterReceiveSingleByte(EUSCI_B0_BASE);
		__delay_cycles(100000);
		__delay_cycles(100000);
		MAP_I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
		__delay_cycles(100000);
		__delay_cycles(100000);
	}
}
