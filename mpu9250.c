/* Port done by Kyle Bradley in collaboration with Jesus Pintado
 *
 * I can be reached at kbradley09@gmail.com
 *
 * This c file and header file are a port of an arduino library
 * for the MPU9250. This chip has has an accelerometer, gyroscope,
 * and magnetometer. The main functionality of  this library is going to
 * be using the quaternion data from the DMP in order to get the pitch.
 * All functionality done in original library is not guaranteed unless I need
 * it.
*/

#include "driverlib.h"
#include <stdint.h>
#include "mpu9250.h"
#include "i2c.h"
#include "quaternionFilters.h"
#include <math.h>

void init_struct(mpu9250 * in){	
	in->Gscale = GFS_250DPS;
	in->Ascale = AFS_2G;
	in->Mscale = MFS_16BITS;
	in->Mmode 	= 2;	
	in->lastUpdate = 0;
}

void getMres(mpu9250 * foo){
	switch(foo->Mscale){
	    case MFS_14BITS:
			foo->mRes = 10.*4912./8190.; // Proper scale to return milliGauss
	        break;
	    case MFS_16BITS:
	          foo->mRes = 10.*4912./32760.0; // Proper scale to return milliGauss
	          break;
	}
}

void getGres(mpu9250 *foo){
	switch (foo->Gscale){
		case GFS_250DPS:
			  foo->gRes = 250.0/32768.0;
			  break;
		case GFS_500DPS:
			  foo->gRes = 500.0/32768.0;
			  break;
		case GFS_1000DPS:
			  foo->gRes = 1000.0/32768.0;
			  break;
		case GFS_2000DPS:
			  foo->gRes = 2000.0/32768.0;
			  break;
	}
}

void getAres(mpu9250 *foo){
	switch (foo->Ascale){
    case AFS_2G:
          foo->aRes = 2.0/32768.0;
          break;
    case AFS_4G:
          foo->aRes = 4.0/32768.0;
          break;
    case AFS_8G:
          foo->aRes = 8.0/32768.0;
          break;
    case AFS_16G:
          foo->aRes = 16.0/32768.0;
          break;
  }
}

void readAccelData(int16_t * destination){
	uint8_t rawData[6];
	read_multibyte_i2c(MPU9250_ADDRESS, ACCEL_XOUT_H, 6, rawData);
	destination[0] = ((int16_t)rawData[0] << 8) | rawData[1] ;  // Turn the MSB and LSB into a signed 16-bit value
	destination[1] = ((int16_t)rawData[2] << 8) | rawData[3] ;
	destination[2] = ((int16_t)rawData[4] << 8) | rawData[5] ;
}

void setAccelData(mpu9250 *foo){
	getAres(foo);
	readAccelData(foo->accelCount);
	foo->ax = (float)foo->accelCount[0]*foo->aRes;
	foo->ay = (float)foo->accelCount[1]*foo->aRes;
	foo->az = (float)foo->accelCount[2]*foo->aRes;
}

void readGyroData(int16_t * destination){
	uint8_t rawData[6];
	read_multibyte_i2c(MPU9250_ADDRESS, GYRO_XOUT_H, 6, rawData);
	destination[0] = ((int16_t)rawData[0] << 8) | rawData[1] ;  // Turn the MSB and LSB into a signed 16-bit value
	destination[1] = ((int16_t)rawData[2] << 8) | rawData[3] ;
	destination[2] = ((int16_t)rawData[4] << 8) | rawData[5] ;
}

void setGyroData(mpu9250 *foo){
	getGres(foo);
	readGyroData(foo->gyroCount);
	foo->gx = (float)foo->gyroCount[0]*foo->gRes;
	foo->gy = (float)foo->gyroCount[1]*foo->gRes;
	foo->gz = (float)foo->gyroCount[2]*foo->gRes;
}

void readMagData(int16_t *destination){
	uint8_t rawData[7];
	read_multibyte_i2c(AK8963_ADDRESS, AK8963_XOUT_L, 7, rawData);
	destination[0] = ((int16_t)rawData[1] << 8) | rawData[0];
	destination[1] = ((int16_t)rawData[3] << 8) | rawData[2];
	destination[2] = ((int16_t)rawData[5] << 8) | rawData[4];
}

void setMagData(mpu9250 *foo){
	getMres(foo);
	foo->mx = -470.;
	foo->my = -120.;
	foo->mz = -125.;
}

void init_offsets(void){
	write_i2c(MPU9250_ADDRESS, ZA_OFFSET_H, 36);
	write_i2c(MPU9250_ADDRESS, ZA_OFFSET_L, 221);
	write_i2c(MPU9250_ADDRESS, XA_OFFSET_H, 21);
	write_i2c(MPU9250_ADDRESS, XA_OFFSET_L, 117);
	write_i2c(MPU9250_ADDRESS, YA_OFFSET_H, 26);
	write_i2c(MPU9250_ADDRESS, YA_OFFSET_L, 114);
}


void updateTime(mpu9250* foo){
	uint16_t current_time = TA0R;
	foo->Now = current_time;

	if(foo->Now < foo->lastUpdate){ //overflow happened
		foo->lastUpdate = 0xFFFF - foo->lastUpdate;
		foo->deltat = ((foo->Now + foo->lastUpdate) / 1000000.0f);
	}
	else{
		foo->deltat = ((foo->Now - foo->lastUpdate) / 1000000.0f);
	}

	foo->lastUpdate = foo->Now;
	foo->sum += foo->deltat;
	foo->sumCount++;
}





