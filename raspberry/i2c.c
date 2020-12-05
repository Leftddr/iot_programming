#include <stdio.h>
#include <pigpio.h>

int main(){
	gpioInitialise();
	int handle = i2cOpen(1, 0x68, 0);
	i2cWriteByteData(handle, 0x19, 0x07);
	i2cWriteByteData(handle, 0x6B, 0x01);

	while(1){

	int high_byte = i2cReadByteData(handle, 0x3B);
	int low_byte = i2cReadByteData(handle, 0x3C);
	int accel_x_raw = (high_byte << 8) | low_byte;
	float accel_x = accel_x_raw / 16384.0f;
	printf("accel_x = %f\n", accel_x);

	high_byte = i2cReadByteData(handle, 0x43);
	low_byte = i2cReadByteData(handle, 0x44);
	int gyro_x_raw = (high_byte << 8) | low_byte;
	float gyro_x = gyro_x_raw / 131.0f;
	printf("gyro_x = %f\n", gyro_x);

	high_byte = i2cReadByteData(handle, 0x41);
	low_byte = i2cReadByteData(handle, 0x42);
	int temp_raw = (high_byte << 8) | low_byte;
	float temp = temp_raw / 340.0f + 36.53f;
	printf("temp = %f\n", temp);
	gpioDelay(100000);
	}
}
