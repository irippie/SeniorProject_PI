//*****************************************************************************
//
// MSP432 main.c template - Empty main
// Author: Jesus Pintado and Kyle Bradley
//
//****************************************************************************

/*
 * Uses pins 3.2 and 3.3 for Xbee communication via UART
 * Router Xbee uses one 3.3V and GND pin on the MSP432
 */

#include "driverlib.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define TIMER_PERIOD 120

void init_clock();
void init_uart();
void init_timer();
void UART_transmit_data(const char* data);

void move_forward(uint8_t);
void move_reverse(uint8_t);
void stop_motors();
uint8_t speed_calc(uint8_t);

const eUSCI_UART_Config uartConfig =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,     		   // SMCLK Clock Source
    13,                                			   // BRDIV
    0,                                       	   // UCxBRF
    37,                                      	   // UCxBRS
    EUSCI_A_UART_NO_PARITY,                  	   // No Parity
    EUSCI_A_UART_LSB_FIRST,                  	   // MSB First
    EUSCI_A_UART_ONE_STOP_BIT,              	   // One stop bit
    EUSCI_A_UART_MODE,                             // UART mode
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};

const Timer_A_UpDownModeConfig upDownConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock SOurce
        TIMER_A_CLOCKSOURCE_DIVIDER_4,          // SMCLK/1 = 3MHz
        TIMER_PERIOD,                           // 127 tick period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,    // Disable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};

const Timer_A_CompareModeConfig compareConfig_PWM1 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_1,          // Use CCR1
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
        TIMER_PERIOD								// duty cycle initialized to 0
};

const Timer_A_CompareModeConfig compareConfig_PWM2 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_2,          // Use CCR2
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
		TIMER_PERIOD								// duty cycle initialized to 0
};

const Timer_A_CompareModeConfig compareConfig_PWM3 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_3,          // Use CCR2
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
		TIMER_PERIOD								// duty cycle initialized to 0
};

const Timer_A_CompareModeConfig compareConfig_PWM4 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_4,          // Use CCR2
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
		TIMER_PERIOD								// duty cycle initialized to 0
};

int main(void)
{
	init_clock();
	init_uart();
//	init_timer();

	// used to store the data received via UART
    uint8_t received_data;

    while(1)
    {
    	received_data = MAP_UART_receiveData( EUSCI_A2_BASE );

//    	uint8_t duty_tracker = 0;
//    	switch(received_data){
//
//			case 'f':
//				MAP_UART_transmitData(EUSCI_A2_BASE, '\r');
//				UART_transmit_data("going forward");
//				MAP_UART_transmitData(EUSCI_A2_BASE, '\r');
//				move_forward(duty_tracker);
//				break;
//			case 'r':
//				MAP_UART_transmitData(EUSCI_A2_BASE, '\r');
//				UART_transmit_data("going reverse");
//				MAP_UART_transmitData(EUSCI_A2_BASE, '\r');
//				move_reverse(duty_tracker);
//				break;
//			case 'u':
//				MAP_UART_transmitData(EUSCI_A2_BASE, '\r');
//				UART_transmit_data("going faster");
//				MAP_UART_transmitData(EUSCI_A2_BASE, '\r');
//				if(duty_tracker == 100)
//					duty_tracker = 100;
//				else
//					duty_tracker += 10;
//				break;
//			case 'd':
//				MAP_UART_transmitData(EUSCI_A2_BASE, '\r');
//				UART_transmit_data("going slower");
//				MAP_UART_transmitData(EUSCI_A2_BASE, '\r');
//				if(duty_tracker == 0)
//					duty_tracker = 0;
//				else
//					duty_tracker -= 10;
//				break;
//    	}

    	/* echoes user input back to terminal */
    	MAP_UART_transmitData( EUSCI_A2_BASE, 'x' );

    	received_data = 0;

    }
}

void UART_transmit_data(const char* data){

	int i;
	for(i = 0; i < strlen(data); i++){ MAP_UART_transmitData(EUSCI_A2_BASE, data[i]); }
	MAP_UART_transmitData(EUSCI_A2_BASE, '\r');

//	for(i = 0; i < strlen(data); i++){ MAP_UART_transmitData(EUSCI_A0_BASE, data[i]); }
//	MAP_UART_transmitData(EUSCI_A0_BASE, '\r');

}

void init_uart(){

	/* Selecting P3.2 and P3.3 in UART mode */
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
			GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

	MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig);
	MAP_UART_enableModule(EUSCI_A2_BASE);

	/* used for msp to terminal comm */
//	/* Selecting P1.2 and P1.3 in UART mode */
//	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
//			GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
//
//	MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
//	MAP_UART_enableModule(EUSCI_A0_BASE);

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

}

void init_timer(){

	/* Setting 7.4 - 7.7 as the outputs for PWM */

	//7.7 == CCR1 ; 7.6 == CCR2; 7.5 == CCR3; 7.4 == CCR4
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN5 +
			GPIO_PIN4+ GPIO_PIN6 + GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

	/* Configuring Timer_A1 for UpDown Mode and starting */
	MAP_Timer_A_configureUpDownMode(TIMER_A1_BASE, &upDownConfig);

	MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UPDOWN_MODE);

	/* Initialize compare registers to generate PWM1 */
	MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_PWM1);
	MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_PWM2);
	MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_PWM3);
	MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_PWM4);
}

void move_forward(uint8_t duty_cycle){

	TA1CCR1 = speed_calc(duty_cycle);
	TA1CCR2 = TIMER_PERIOD;
	TA1CCR3 = speed_calc(duty_cycle);
	TA1CCR4 = TIMER_PERIOD;
}

void move_reverse(uint8_t duty_cycle){

	TA1CCR1 = TIMER_PERIOD;
	TA1CCR2 = speed_calc(duty_cycle);
	TA1CCR3 = TIMER_PERIOD;
	TA1CCR4 = speed_calc(duty_cycle);
}

void stop_motors(){

	TA1CCR1 = TIMER_PERIOD;
	TA1CCR2 = TIMER_PERIOD;
	TA1CCR3 = TIMER_PERIOD;
	TA1CCR4 = TIMER_PERIOD;
}

uint8_t speed_calc(uint8_t duty_cycle){
	return duty_cycle*-1.2+120;
}
