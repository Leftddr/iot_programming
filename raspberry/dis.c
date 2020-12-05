#include <stdio.h>
#include <pigpio.h>

int main(){
	int trig = 17;
	int echo = 27;
	unsigned int start, end;
	float distance;

	gpioInitialise();

	gpioSetMode(trig, PI_OUTPUT);
	gpioSetMode(echo, PI_INPUT);
	gpioWrite(trig, 0);
	gpioDelay(10000);

	while(1){
		printf("start");
		gpioWrite(trig, 1);
		gpioDelay(10);
		gpioWrite(trig, 0);
		while(gpioRead(echo) == 0){
			start = gpioTick();
		}

		while(gpioRead(echo) == 1){
			end = gpioTick();
		}

		distance = (end - start) / 58.0f;
		printf("diff = %u, distance (cm) : %f\n", \
				end - start, distance);
		gpioWrite(trig, 0);
		gpioDelay(10000);

	}
	gpioTerminate();
	return 0;
}
