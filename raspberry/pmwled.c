#include <stdio.h>
#include <pigpio.h>

int main(){
	int pin = 18;
	int step = 10000;
	int delay = 10000;
	int freq = 10000;
	int duty = 0;

	gpioInitialise();

	while(1){
		duty += step;
		if(duty >= 1000000 || duty <= 0){
			step = -step;
		}
		gpioHardwarePWM(pin, freq, duty);
		gpioDelay(delay);
	}

	gpioTerminate();
	return 0;
}
