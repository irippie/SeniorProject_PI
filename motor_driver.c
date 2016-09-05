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

void move_forward(void);
void move_reverse(void);
void UART_transmit_data(const char* data);
void init_clock(void);
void set_MOTOR_SPEED(volatile uint16_t * motor_forward, volatile uint16_t * motor_reverse,
									uint8_t duty_cycle, uint8_t direction);

#define TIMER_PERIOD 120
#define DUTY_CYCLE1 0
#define DUTY_CYCLE2 60
#define FORWARD 1
#define REVERSE 0

static volatile uint16_t adc_result;
static volatile uint16_t * L_MOTOR_FORWARD = &TA1CCR1;
static volatile uint16_t * L_MOTOR_REV = &TA1CCR2;
static volatile uint16_t * R_MOTOR_FORWARD = &TA1CCR3;
static volatile uint16_t * R_MOTOR_REV = &TA1CCR3;


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
        DUTY_CYCLE1                                 // 32 Duty Cycle
};

const Timer_A_CompareModeConfig compareConfig_PWM2 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_2,          // Use CCR2
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
        DUTY_CYCLE2                                 // 96 Duty Cycle
};

const Timer_A_CompareModeConfig compareConfig_PWM3 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_3,          // Use CCR2
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
        DUTY_CYCLE2                                 // 96 Duty Cycle
};

const Timer_A_CompareModeConfig compareConfig_PWM4 =
{
        TIMER_A_CAPTURECOMPARE_REGISTER_4,          // Use CCR2
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,   // Disable CCR interrupt
        TIMER_A_OUTPUTMODE_TOGGLE_SET,              // Toggle output but
        DUTY_CYCLE2                                 // 96 Duty Cycle
};

// This code is going to be operating off the TIMER_A1 Module
// As I want to keep the A0 Module free for the encoders

//*****************************************************************************
// Things to keep in mind; setting CCR to 0 will have an always logic
// high output. Setting them to period will keep them low.
//****************************************************************************
int main(void){

    /* Halting WDT  */
    MAP_WDT_A_holdTimer();

    init_clock(); //contains commands needed to initalize clock to 24 MHz

    /* Setting 7.4 - 7.7 as the outputs for PWM */
    //7.7 == CCR1 ; 7.6 == CCR2; 7.5 == CCR3; 7.4 == CCR4
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN5 +
    		GPIO_PIN4+ GPIO_PIN6 + GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring Timer_A1 for UpDown Mode and starting */
    MAP_Timer_A_configureUpDownMode(TIMER_A1_BASE, &upDownConfig);

    MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UPDOWN_MODE);

    /* Selecting P1.2 and P1.3 in UART mode */
//	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
//			GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

	MAP_UART_initModule(EUSCI_A1_BASE, &uartConfig);
	MAP_UART_enableModule(EUSCI_A1_BASE);


	/* Initialize compare registers to generate PWM1 */
	MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_PWM1);
	MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_PWM2);
	MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_PWM3);
	MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig_PWM4);


	//using button to move between different PWM
	MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

	while(1){
		uint8_t button = P1IN & 0x02;
		if(button == 0x02)
			move_forward();
		else
			move_reverse();
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

/* For now, direction will be 1 == foward, 0 == reverse */
// created defines for direction
void move_forward(){
	set_MOTOR_SPEED(L_MOTOR_FORWARD, L_MOTOR_REV, 20, FORWARD);
}
void move_reverse(){
	set_MOTOR_SPEED(L_MOTOR_FORWARD, L_MOTOR_REV, 20, REVERSE);
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


/* ADC Interrupt Handler. This handler is called whenever there is a conversion
 * that is finished for ADC_MEM0.
 */
void ADC14_IRQHandler(void){
    uint64_t status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);

    if (ADC_INT0 & status){
        adc_result = MAP_ADC14_getResult(ADC_MEM0);
        float temp = (float)adc_result /(float)0x3FFF;
        float percent = temp * 100;
        uint8_t dutycycle = percent;

        TA1CCR1 = dutycycle;

        MAP_ADC14_toggleConversionTrigger();
    }
}
