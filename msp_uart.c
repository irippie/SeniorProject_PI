/*
 * msp_uart.c
 *
 *  Created on: Oct 28, 2016
 *      Author: Kyle
 */


#include "driverlib.h"
#include "msp_uart.h"


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

	int i;
	for(i = 0; i < strlen(data); i++){
		MAP_UART_transmitData(EUSCI_A0_BASE, data[i]);
	}
	MAP_UART_transmitData(EUSCI_A0_BASE, 0xA);
	MAP_UART_transmitData(EUSCI_A0_BASE, 0xD);

}

void init_uart(){

	/* Selecting P3.2 and P3.3 in UART mode */
//	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
//			GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
//
//	MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig);
//	MAP_UART_enableModule(EUSCI_A2_BASE);

	/* used for msp to terminal comm */
	/* Selecting P1.2 and P1.3 in UART mode */
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
			GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

	MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
	MAP_UART_enableModule(EUSCI_A0_BASE);

}
