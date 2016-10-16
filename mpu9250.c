/*
* Port done by Kyle Bradley.
*
* This c file and header file are a port of an arduino library 
* for the MPU9250. This chip has has an accelerometer, gyroscope,
* and magnetometer. The main functionality of  this libaray is going to
* be using the quaternion data from the DMP in order to get the pitch.
* All functionality done in original library is not guaranteed unless I need
* it.
*/
#include <stdint.h>
#include "mpu9250.h"




void init_struct(mpu9250 * in){	
	// memset(&in, 0, sizeof(in));
	in->Gscale = GFS_250DPS;
	in->Ascale = AFS_2G;
	in->Mscale = MFS_16BITS;
	in->Mmode 	= 2;	
}

void getMres(mpu9250 * foo){
	switch(foo->Mscale){

  	// Possible magnetometer scales (and their register bit settings) are:
  	// 14 bit resolution (0) and 16 bit resolution (1)
	    case MFS_14BITS:
			foo->mRes = 10.*4912./8190.; // Proper scale to return milliGauss
	        break;
	    case MFS_16BITS:
	          foo->mRes = 10.*4912./32760.0; // Proper scale to return milliGauss
	          break;
  }
}
