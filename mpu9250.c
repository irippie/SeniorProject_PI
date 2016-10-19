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
#include "driverlib.h"
#include <stdint.h>
#include "mpu9250.h"
#include "i2c.h"
#include "quaternionFilters.h"



void init_struct(mpu9250 * in){	
	// memset(&in, 0, sizeof(in));
	in->Gscale = GFS_250DPS;
	in->Ascale = AFS_2G;
	in->Mscale = MFS_16BITS;
	in->Mmode 	= 2;	
	in->lastUpdate = 0;
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
void getGres(mpu9250 *foo){
  switch (foo->Gscale){
  // Possible gyro scales (and their register bit settings) are:
  // 250 DPS (00), 500 DPS (01), 1000 DPS (10), and 2000 DPS  (11).
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
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
  // Possible accelerometer scales (and their register bit settings) are:
  // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
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
//	int16_t accelData[3];
	readAccelData(foo->accelCount);
//	int i;
//	for(i = 0; i < 3; i++) foo->accelCount[i] = accelData[i];
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
//	int16_t gyroData[3];
	getGres(foo);
	readGyroData(foo->gyroCount);
//	int i;
//	for(i = 0; i < 3; i++) foo->gyroCount[i] = gyroData[i];
	foo->gx = (float)foo->gyroCount[0]*foo->gRes;
	foo->gy = (float)foo->gyroCount[1]*foo->gRes;
	foo->gz = (float)foo->gyroCount[2]*foo->gRes;
}
void readMagData(int16_t *destination){
	uint8_t rawData[7];
	//read_multibyte_i2c()

	read_multibyte_i2c(AK8963_ADDRESS, AK8963_XOUT_L, 7, rawData);


	// Turn the MSB and LSB into a signed 16-bit value
	destination[0] = ((int16_t)rawData[1] << 8) | rawData[0];
	// Data stored as little Endian
	destination[1] = ((int16_t)rawData[3] << 8) | rawData[2];
	destination[2] = ((int16_t)rawData[5] << 8) | rawData[4];

}
void setMagData(mpu9250 *foo){
	getMres(foo);
	readMagData(foo->magCount);
//	foo->mx = (float)foo->magCount[0]*foo->mRes;
//	foo->my = (float)foo->magCount[1]*foo->mRes;
//	foo->mz = (float)foo->magCount[2]*foo->mRes;
	foo->mx = -470.;
	foo->my = -120.;
	foo->mz = -125.;
}

void initMAG(mpu9250 *foo, float * destination){
	// First extract the factory calibration for each magnetometer axis
	uint8_t rawData[3];  // x/y/z gyro calibration data stored here
	write_i2c(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer
	__delay_cycles(1000);
	write_i2c(AK8963_ADDRESS, AK8963_CNTL, 0x0F); // Enter Fuse ROM access mode
	__delay_cycles(1000);
	read_multibyte_i2c(AK8963_ADDRESS, AK8963_ASAX, 3, &rawData[0]);  // Read the x-, y-, and z-axis calibration values
	destination[0] =  (float)(rawData[0] - 128)/256. + 1.;   // Return x-axis sensitivity adjustment values, etc.
	destination[1] =  (float)(rawData[1] - 128)/256. + 1.;
	destination[2] =  (float)(rawData[2] - 128)/256. + 1.;
	write_i2c(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer
	__delay_cycles(1000);
	// Configure the magnetometer for continuous read and highest resolution
	// set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL register,
	// and enable continuous mode data acquisition Mmode (bits [3:0]), 0010 for 8 Hz and 0110 for 100 Hz sample rates
	write_i2c(AK8963_ADDRESS, AK8963_CNTL, foo->Mscale << 4 | foo->Mmode); // Set magnetometer data resolution and sample ODR
	__delay_cycles(1000);
}

void calibrateMPU(float * gyroBias, float * accelBias){
	uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
	  uint16_t ii, packet_count, fifo_count;
	  int32_t gyro_bias[3]  = {0, 0, 0}, accel_bias[3] = {0, 0, 0};

	  // reset device
	  // Write a one to bit 7 reset bit; toggle reset device
	  write_i2c(MPU9250_ADDRESS, PWR_MGMT_1, 0x80);
	  __delay_cycles(1000);

	 // get stable time source; Auto select clock source to be PLL gyroscope
	 // reference if ready else use the internal oscillator, bits 2:0 = 001
	  write_i2c(MPU9250_ADDRESS, PWR_MGMT_1, 0x01);
	  write_i2c(MPU9250_ADDRESS, PWR_MGMT_2, 0x00);
	  __delay_cycles(2000);

	  // Configure device for bias calculation
	  write_i2c(MPU9250_ADDRESS, INT_ENABLE, 0x00);   // Disable all interrupts
	  write_i2c(MPU9250_ADDRESS, FIFO_EN, 0x00);      // Disable FIFO
	  write_i2c(MPU9250_ADDRESS, PWR_MGMT_1, 0x00);   // Turn on internal clock source
	  write_i2c(MPU9250_ADDRESS, I2C_MST_CTRL, 0x00); // Disable I2C master
	  write_i2c(MPU9250_ADDRESS, USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
	  write_i2c(MPU9250_ADDRESS, USER_CTRL, 0x0C);    // Reset FIFO and DMP
	  __delay_cycles(1500);

	// Configure MPU6050 gyro and accelerometer for bias calculation
	  write_i2c(MPU9250_ADDRESS, CONFIG, 0x01);      // Set low-pass filter to 188 Hz
	  write_i2c(MPU9250_ADDRESS, SMPLRT_DIV, 0x00);  // Set sample rate to 1 kHz
	  write_i2c(MPU9250_ADDRESS, GYRO_CONFIG, 0x00);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
	  write_i2c(MPU9250_ADDRESS, ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity

	  uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
	  uint16_t  accelsensitivity = 16384;  // = 16384 LSB/g

	    // Configure FIFO to capture accelerometer and gyro data for bias calculation
	  write_i2c(MPU9250_ADDRESS, USER_CTRL, 0x40);   // Enable FIFO
	  write_i2c(MPU9250_ADDRESS, FIFO_EN, 0x78);     // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in MPU-9150)
	  __delay_cycles(4000); // accumulate 40 samples in 40 milliseconds = 480 bytes

	// At end of sample accumulation, turn off FIFO sensor read
	  write_i2c(MPU9250_ADDRESS, FIFO_EN, 0x00);        // Disable gyro and accelerometer sensors for FIFO
	  read_multibyte_i2c(MPU9250_ADDRESS, FIFO_COUNTH, 2, &data[0]); // read FIFO sample count
	  fifo_count = ((uint16_t)data[0] << 8) | data[1];
	  packet_count = fifo_count/12;// How many sets of full gyro and accelerometer data for averaging

	  for (ii = 0; ii < packet_count; ii++)
	  {
	    int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
	    read_multibyte_i2c(MPU9250_ADDRESS, FIFO_R_W, 12, &data[0]); // read data for averaging
	    accel_temp[0] = (int16_t) (((int16_t)data[0] << 8) | data[1]  );  // Form signed 16-bit integer for each sample in FIFO
	    accel_temp[1] = (int16_t) (((int16_t)data[2] << 8) | data[3]  );
	    accel_temp[2] = (int16_t) (((int16_t)data[4] << 8) | data[5]  );
	    gyro_temp[0]  = (int16_t) (((int16_t)data[6] << 8) | data[7]  );
	    gyro_temp[1]  = (int16_t) (((int16_t)data[8] << 8) | data[9]  );
	    gyro_temp[2]  = (int16_t) (((int16_t)data[10] << 8) | data[11]);

	    accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
	    accel_bias[1] += (int32_t) accel_temp[1];
	    accel_bias[2] += (int32_t) accel_temp[2];
	    gyro_bias[0]  += (int32_t) gyro_temp[0];
	    gyro_bias[1]  += (int32_t) gyro_temp[1];
	    gyro_bias[2]  += (int32_t) gyro_temp[2];
	  }
	  accel_bias[0] /= (int32_t) packet_count; // Normalize sums to get average count biases
	  accel_bias[1] /= (int32_t) packet_count;
	  accel_bias[2] /= (int32_t) packet_count;
	  gyro_bias[0]  /= (int32_t) packet_count;
	  gyro_bias[1]  /= (int32_t) packet_count;
	  gyro_bias[2]  /= (int32_t) packet_count;

	  if(accel_bias[2] > 0L) {accel_bias[2] -= (int32_t) accelsensitivity;}  // Remove gravity from the z-axis accelerometer bias calculation
	  else {accel_bias[2] += (int32_t) accelsensitivity;}

	// Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
	  data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
	  data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
	  data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
	  data[3] = (-gyro_bias[1]/4)       & 0xFF;
	  data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
	  data[5] = (-gyro_bias[2]/4)       & 0xFF;

	// Push gyro biases to hardware registers
	  write_i2c(MPU9250_ADDRESS, XG_OFFSET_H, data[0]);
	  write_i2c(MPU9250_ADDRESS, XG_OFFSET_L, data[1]);
	  write_i2c(MPU9250_ADDRESS, YG_OFFSET_H, data[2]);
	  write_i2c(MPU9250_ADDRESS, YG_OFFSET_L, data[3]);
	  write_i2c(MPU9250_ADDRESS, ZG_OFFSET_H, data[4]);
	  write_i2c(MPU9250_ADDRESS, ZG_OFFSET_L, data[5]);

	// Output scaled gyro biases for display in the main program
	  gyroBias[0] = (float) gyro_bias[0]/(float) gyrosensitivity;
	  gyroBias[1] = (float) gyro_bias[1]/(float) gyrosensitivity;
	  gyroBias[2] = (float) gyro_bias[2]/(float) gyrosensitivity;

	// Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
	// factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
	// non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
	// compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
	// the accelerometer biases calculated above must be divided by 8.

	  int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
	  read_multibyte_i2c(MPU9250_ADDRESS, XA_OFFSET_H, 2, &data[0]); // Read factory accelerometer trim values
	  accel_bias_reg[0] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
	  read_multibyte_i2c(MPU9250_ADDRESS, YA_OFFSET_H, 2, &data[0]);
	  accel_bias_reg[1] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
	  read_multibyte_i2c(MPU9250_ADDRESS, ZA_OFFSET_H, 2, &data[0]);
	  accel_bias_reg[2] = (int32_t) (((int16_t)data[0] << 8) | data[1]);

	  uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
	  uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis

	  for(ii = 0; ii < 3; ii++) {
	    if((accel_bias_reg[ii] & mask)) mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
	  }

	  // Construct total accelerometer bias, including calculated average accelerometer bias from above
	  accel_bias_reg[0] -= (accel_bias[0]/8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
	  accel_bias_reg[1] -= (accel_bias[1]/8);
	  accel_bias_reg[2] -= (accel_bias[2]/8);

	  data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
	  data[1] = (accel_bias_reg[0])      & 0xFF;
	  data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
	  data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
	  data[3] = (accel_bias_reg[1])      & 0xFF;
	  data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
	  data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
	  data[5] = (accel_bias_reg[2])      & 0xFF;
	  data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers

	// Apparently this is not working for the acceleration biases in the MPU-9250
	// Are we handling the temperature correction bit properly?
	// Push accelerometer biases to hardware registers
	  write_i2c(MPU9250_ADDRESS, XA_OFFSET_H, data[0]);
	  write_i2c(MPU9250_ADDRESS, XA_OFFSET_L, data[1]);
	  write_i2c(MPU9250_ADDRESS, YA_OFFSET_H, data[2]);
	  write_i2c(MPU9250_ADDRESS, YA_OFFSET_L, data[3]);
	  write_i2c(MPU9250_ADDRESS, ZA_OFFSET_H, data[4]);
	  write_i2c(MPU9250_ADDRESS, ZA_OFFSET_L, data[5]);

	// Output scaled accelerometer biases for display in the main program
	   accelBias[0] = (float)accel_bias[0]/(float)accelsensitivity;
	   accelBias[1] = (float)accel_bias[1]/(float)accelsensitivity;
	   accelBias[2] = (float)accel_bias[2]/(float)accelsensitivity;
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
















