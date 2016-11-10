/*
 * msp_uart.h
 *
 *  Created on: Oct 28, 2016
 *      Author: Kyle
 */

#ifndef MSP_UART_H_
#define MSP_UART_H_

/* @brief transmits string via UART module
 * @TODO: need to pass in module instance for portability
 * @param1 string to be sent
 */

void tx_data(const char*);

// @brief initializes UART

void init_uart();

#endif /* MSP_UART_H_ */
