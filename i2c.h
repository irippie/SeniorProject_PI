/*
 * i2c.h
 *
 *  Created on: Oct 16, 2016
 *      Author: Kyle
 */

#ifndef _I2C_H_
#define _I2C_H_




uint8_t read_i2c(uint8_t, uint8_t);
void read_multibyte_i2c(uint8_t, uint8_t, uint8_t, uint8_t *);
void write_i2c(uint8_t, uint8_t, uint8_t);
void init_i2c();


#endif /* I2C_H_ */
