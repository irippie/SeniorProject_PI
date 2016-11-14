/*
 * msp_uart.c
 *
 *  Created on: Oct 28, 2016
 *      Author: Kyle
 */


#include "driverlib.h"
#include "msp_uart.h"

//initializing UART for 9600 BR

const eUSCI_UART_Config uartConfig = {
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,     		   // SMCLK Clock Source
    156,                                			   // BRDIV
    4,                                       	   // UCxBRF
    0,                                      	   // UCxBRS
    EUSCI_A_UART_NO_PARITY,                  	   // No Parity
    EUSCI_A_UART_LSB_FIRST,                  	   // MSB First
    EUSCI_A_UART_ONE_STOP_BIT,              	   // One stop bit
    EUSCI_A_UART_MODE,                             // UART mode
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};

void tx_data(const char* data){

	//transmitting each character of the screen via uart_a0
	int i;
	for(i = 0; i < 3; i++){
		MAP_UART_transmitData(EUSCI_A0_BASE, data[i]);
	}

	//sending break line
	// USB UART
	MAP_UART_transmitData(EUSCI_A0_BASE, 0xA);
	MAP_UART_transmitData(EUSCI_A0_BASE, 0xD);

	// BT UART
//	MAP_UART_transmitData(EUSCI_A2_BASE, 0xA);
//	MAP_UART_transmitData(EUSCI_A2_BASE, 0xD);

}

void init_uart(){

	/* used for msp to terminal comm */
	/* Selecting P1.2 and P1.3 in UART mode */
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
			GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

	MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
	MAP_UART_enableModule(EUSCI_A0_BASE);


	/* BLUETOOTH USB */
//	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
//			GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
//
//	MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig);
//	MAP_UART_enableModule(EUSCI_A2_BASE);

}
