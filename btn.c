#include <stdio.h> 
#include <unistd.h> 
#include <stdint.h> 
#include <wiringPi.h> 
#include <wiringPiI2C.h> 
#include <unistd.h>
#include <sys/time.h> 
#include <stdbool.h> 
#include <string.h>
#include <stdlib.h>
#define BTN_LEFT 6
#define BTN_RIGHT 24

int main()
{
	int val_left, val_right;
	int btns_pushed_nb = 0;

	wiringPiSetup();

	pinMode(BTN_LEFT, INPUT);
	pinMode(BTN_RIGHT, INPUT);

	/*digitalWrite(BTN_LEFT, HIGH);
	digitalWrite(BTN_RIGHT, HIGH);*/

	while(true){
		val_left = digitalRead(BTN_LEFT);
		val_right = digitalRead(BTN_RIGHT);
		if(val_left == 0 ) btns_pushed_nb++;
		if(val_right == 0 ) btns_pushed_nb++; 
		printf("val_left=%d val_right=%d\n", val_left, val_right);
	}
	return 1;
}
