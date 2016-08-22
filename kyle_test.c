//*****************************************************************************
//
// MSP432 main.c template - Empty main
// Author: Kyle "small dick" Bradley
//****************************************************************************

#include "driverlib.h"
#include <stdint.h>
#include <stdbool.h>




const eUSCI_UART_Config uartConfig =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
    78,                                     // BRDIV = 78
    2,                                       // UCxBRF = 2
    0,                                       // UCxBRS = 0
    EUSCI_A_UART_NO_PARITY,                  // No Parity
    EUSCI_A_UART_LSB_FIRST,                  // MSB First
    EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
    EUSCI_A_UART_MODE,                       // UART mode
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};

int main(void)
{
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();

    /* Selecting P1.2 and P1.3 in UART mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Setting DCO to 12MHz */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);
//    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_TRANSMIT_INTERRUPT);
//    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
//    MAP_Interrupt_enableSleepOnIsrExit();
//
//    MAP_Interrupt_enableMaster();


    while(1){

    	UART_send_string("test");

    	MAP_UART_transmitData(EUSCI_A0_BASE, 0x0D);
    	MAP_UART_transmitData(EUSCI_A0_BASE, 0x0A); //creates new line

//    	UART_clear_screen();
//    	MAP_PCM_gotoLPM0();
    }
}
void EUSCIA0_IRQHandler(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        MAP_UART_transmitData(EUSCI_A0_BASE, 0x31);
    }

}
void UART_send_string(char* in){
	int i;
	for(i = 0; i < strlen(in); i++){
		MAP_UART_transmitData(EUSCI_A0_BASE, in[i]);
	}
}
void UART_clear_screen(){
	MAP_UART_transmitData(EUSCI_A0_BASE, 0x1b);
}
