//*****************************************************************************
//
// MSP432 main.c template - Empty main
// Author: Jesus Pintado and Kyle Bradley
//
// TODO: MOVE BULLSHIT TO HEADER FILE
//****************************************************************************

#include "driverlib.h"

#include <stdint.h>
#include <string.h>
#include "motor_driver.h"





const Timer_A_UpDownModeConfig upDownConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock SOurce
        TIMER_A_CLOCKSOURCE_DIVIDER_24,          // SMCLK/4 = 3MHz
        TIMER_PERIOD,                           // 127 tick period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,    // Disable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};
//PIN 7.7 TA1.1
const Timer_A_CompareModeConfig compareConfig_PWM1 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_1,          // Use CCR1
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
        TIMER_PERIOD								// duty cycle initialized to 0
};
//PIN 7.6 TA1.2
const Timer_A_CompareModeConfig compareConfig_PWM2 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_2,          // Use CCR2
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
		TIMER_PERIOD								// duty cycle initialized to 0
};
//PIN 7.5 TA1.3
const Timer_A_CompareModeConfig compareConfig_PWM3 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_3,          // Use CCR3
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
		TIMER_PERIOD								// duty cycle initialized to 0
};
//PIN 7.4 TA1.4
const Timer_A_CompareModeConfig compareConfig_PWM4 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_4,          // Use CCR4
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
		TIMER_PERIOD								// duty cycle initialized to 0
};
/* For now, direction will be 1 == foward, 0 == reverse */
// created defines for direction
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

uint32_t speed_calc(uint8_t duty_cycle){
	return duty_cycle*-10+1000;
}

void init_PWM_timers(){

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

void set_MOTOR_SPEED(volatile uint16_t * motor_forward,volatile uint16_t * motor_reverse,
									uint8_t duty_cycle, uint8_t direction){
	if(direction == 1){ //foward
		*motor_forward = duty_cycle;
		*motor_reverse = TIMER_PERIOD; //setting to 0
	}
	else{ //reverse
		*motor_reverse = duty_cycle;
		*motor_forward = TIMER_PERIOD; //setting to 0
	}
}


void init_adc(void){

	MAP_ADC14_enableModule();
	MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4,
			0);

	/* Configuring GPIOs (5.5 A0) */
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,
	GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory */
    MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A0, false);
    /* Configuring Sample Timer */
   MAP_ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

   /* Enabling/Toggling Conversion */
   MAP_ADC14_enableConversion();
   MAP_ADC14_toggleConversionTrigger();

   /* Enabling interrupts */
   MAP_ADC14_enableInterrupt(ADC_INT0);
   MAP_Interrupt_enableInterrupt(INT_ADC14);
   MAP_Interrupt_enableMaster();

}

