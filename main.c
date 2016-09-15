//*****************************************************************************
//
// MSP432 main.c template - Empty main
// Author: Jesus Pintado and Kyle Bradley
//
//****************************************************************************

#include "driverlib.h"
#include "motor_driver.h"
#include <stdint.h>
#include <string.h>

void init_clock();
void UART_transmit_data(char*);

const eUSCI_UART_Config uartConfig =
{
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

int main(void)
{
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();

    /* Selecting P1.2 and P1.3 in UART mode */
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
			GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

	MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
	MAP_UART_enableModule(EUSCI_A0_BASE);



	init_clock(); //contains commands needed to initalize clock to 24 MHz
	init_timers();

	//using button to move between different PWM
	MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
	uint8_t duty_cycle = 40;
	while(1){
		uint8_t button = P1IN & 0x02;
//		move_foward
		if(button == 0x02)
			move_forward(duty_cycle);
		else{
			move_forward(50);
		}



	}
}

void UART_transmit_data(char* data){

	int i;
	for(i = 0; i < strlen(data); i++){ MAP_UART_transmitData(EUSCI_A0_BASE, data[i]); }

	MAP_UART_transmitData(EUSCI_A0_BASE, '\r');
	MAP_UART_transmitData(EUSCI_A0_BASE, '\n');

}
void init_clock(void){

    /* Enable floating point unit to set DCO frequency */
    MAP_FPU_enableModule();

    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Increasing core voltage to handle higher frequencies */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);



    /* Setting DCO to 48MHz */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);

    /* Setting P4.3 to output MCLK frequency */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
}
