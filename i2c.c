/*
 * i2c.c
 *
 *  Created on: Oct 16, 2016
 *      Author: Kyle
 */
#include "driverlib.h"
#include <stdint.h>
#include "i2c.h"
#include "mpu9250.h"



//#define SLAVE_ADDRESS 0x68 //defining slave address as 0x68 for our MPU9250
const eUSCI_I2C_MasterConfig i2cConfig =
{
	EUSCI_B_I2C_CLOCKSOURCE_SMCLK, 		// SMCLK Clock Source
	24000000,		 					// SMCLK = 24MHz
	EUSCI_B_I2C_SET_DATA_RATE_400KBPS, 	// Desired I2C Clock of 400khz
	0, 									// No byte counter threshold
	EUSCI_B_I2C_NO_AUTO_STOP 			// No Autostop
};


uint8_t read_i2c(uint8_t slave_addr, uint8_t register_addr){
	/* Specify slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, slave_addr);

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
void read_multibyte_i2c(uint8_t slave_addr, uint8_t register_addr, uint8_t num_bytes, uint8_t * data){
	int i;
	for(i = 0; i < num_bytes; i++){
		data[i] = read_i2c(slave_addr, register_addr++);
	}
}

void write_i2c(uint8_t slave_addr, uint8_t register_addr, uint8_t register_data){
	/* Specify slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, slave_addr);

	MAP_I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	/* Wait until ready */
	MAP_I2C_masterSendStart(EUSCI_B0_BASE);

	/* Send start bit and register */
	MAP_I2C_masterSendMultiByteNext(EUSCI_B0_BASE, register_addr);

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
	MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, MPU9250_ADDRESS);

	/* Set master to transmit mode */
	MAP_I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	/* Enable I2C Module to start operations */
	MAP_I2C_enableModule(EUSCI_B0_BASE);

}



