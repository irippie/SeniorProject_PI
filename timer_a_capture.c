//*****************************************************************************
//
// MSP432 timer_a_capture.c template - Empty main
// Author: Jesus Pintado and Kyle Bradley
//
//****************************************************************************
//*****************************************************************************
//	UART 115200 Baud rate
// 	MSP432 set for 48 Mhz
// TODO: 	PINS USED:
//

//****************************************************************************

#include "driverlib.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

uint32_t rising_VAL, falling_VAL;


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

//TODO: figure out correct clock divide for 48 mhz
const Timer_A_ContinuousModeConfig continuousModeConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,           // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_48,       // SMCLK = 48 MHz/48 = 1MHz
        TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer ISR
        TIMER_A_SKIP_CLEAR                   // Skup Clear Counter
};

const Timer_A_CaptureModeConfig captureModeConfig =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_1,        // CC Register 1
        TIMER_A_CAPTUREMODE_RISING_AND_FALLING_EDGE,          // Rising Edge
        TIMER_A_CAPTURE_INPUTSELECT_CCIxA,        // CCIxA Input Select
        TIMER_A_CAPTURE_SYNCHRONOUS,              // Synchronized Capture
        TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE,  // Enable interrupt
        TIMER_A_OUTPUTMODE_OUTBITVALUE            // Output bit value
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

	/*Building test model using DAD board square wave input*/
	/*Setting P2.4 as TA0 input capture*/
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2,
			GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
	/* Configuring Capture Mode */
	MAP_Timer_A_initCapture(TIMER_A0_BASE, &captureModeConfig);

	/* Configuring Continuous Mode */
	MAP_Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

	MAP_Interrupt_enableInterrupt(INT_TA0_N);
	MAP_Interrupt_enableMaster();
	//init_TimerA();
	MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);

	while(1){
		//nothing
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
/******************************************************************************
*
******************************************************************************/
void init_TimerA(void){


}
/******************************************************************************
*Inside this ISR we will figure out the time between pulses
******************************************************************************/
void TA0_N_IRQHandler(void){
	MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
	            TIMER_A_CAPTURECOMPARE_REGISTER_1);
	uint8_t pin_value = P2IN & 0x10;
	if(pin_value != 0){
		rising_VAL =  MAP_Timer_A_getCaptureCompareCount(TIMER_A0_BASE,
									TIMER_A_CAPTURECOMPARE_REGISTER_1);
	}
	else{
		falling_VAL =  MAP_Timer_A_getCaptureCompareCount(TIMER_A0_BASE,
											TIMER_A_CAPTURECOMPARE_REGISTER_1);
	}
}

void output_time_delta(uint32_t rising, uint32_t falling){


}
