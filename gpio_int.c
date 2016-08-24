//*****************************************************************************
//
// MSP432 gpio.c template - Empty main
// Author: Jesus Pintado and Kyle Bradley
//
//****************************************************************************
//*****************************************************************************
//	UART 115200 Baud rate
// 	MSP432 set for 48 Mhz
// 	PINS USED:
//	P1.1/2/3

//****************************************************************************

#include "driverlib.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

const char * val1 = "not triggered";
const char * val2 = "triggered";
const char * send_val;

void UART_transmit_data(const char* data);

const eUSCI_UART_Config uartConfig = {
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
    26,                                     // BRDIV = 26
    0,                                       // UCxBRF = 0
    111,                                       // UCxBRS = 111
    EUSCI_A_UART_NO_PARITY,                  // No Parity
    EUSCI_A_UART_LSB_FIRST,                  // MSB First
    EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
    EUSCI_A_UART_MODE,                       // UART mode
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};

int main(void){
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();

    /* Enable floating point unit to set DCO frequency */
    MAP_FPU_enableModule();

    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Increasing core voltage to handle higher frequencies */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Setting DCO to 48MHz */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);



    /* Selecting P1.2 and P1.3 in UART mode */
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
			GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

	MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
	MAP_UART_enableModule(EUSCI_A0_BASE);


	/*/////////////////////////////////////////////////////////////////////////
	 * LOW-HIGH TRANSITION PORT INTERRUPT
	 * building test module using one of the buttons on the MSP432 Launchpad
	 * sets P1.1 as an input pin with a pull up resistor with a low to high
	 * edge triggered interrupt
	 *
	 * TODO: not sure what value of button is by default
	 */////////////////////////////////////////////////////////////////////////

	MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
	MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
	MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1, GPIO_HIGH_TO_LOW_TRANSITION);

	MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
	MAP_Interrupt_enableInterrupt(INT_PORT1);

	MAP_Interrupt_enableMaster();

	//TODO: not using the LPM because I want to see the behavior of interrupts
	//without them.
	/**************************************************************************
	 * TODO:
	 * LPM MODE NOTE FOR JESUS
	 * basically shuts the processor down and only interrupts can bring it out
	 * of LPM
	 *************************************************************************/
	send_val = val1;
	UART_transmit_data(send_val);
    while(1){
    	MAP_PCM_gotoLPM3();
    }
}

void UART_transmit_data(const char* data){

	int i;
	for(i = 0; i < strlen(data); i++){
		MAP_UART_transmitData(EUSCI_A0_BASE, data[i]);
	}

	MAP_UART_transmitData(EUSCI_A0_BASE, '\r');
	MAP_UART_transmitData(EUSCI_A0_BASE, '\n');

}

//TODO: come up with better method of doing a double edge triggered interrupt


/******************************************************************************
* Interrupt first triggers on a high to low transition based upon the button
* being high by default and low when pressed
*
* When its first pressed it sends out triggered and changes the interrupt
* config to be a low to high (button released) indicated on the terminal
* by not triggered
*
*****************************************************************************/
void PORT1_IRQHandler(void){

	uint32_t status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
	MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);
	uint8_t pin_val = P1IN & 0x02;

	if(status & GPIO_PIN1){ //if pin1 cause interrupt
							//TODO: not needed
		if(pin_val == 0){ // start of triggering
			send_val = val2;
			UART_transmit_data(send_val);
			MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1, GPIO_LOW_TO_HIGH_TRANSITION);
		}
		else{
			send_val = val1;
			UART_transmit_data(send_val);
			MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1, GPIO_HIGH_TO_LOW_TRANSITION);
		}
	}
}
