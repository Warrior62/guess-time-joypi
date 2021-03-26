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


int main(){

	int i = 0;
	int dizaine_seconde = 0;
	int seconde = 0;
	int dizieme = 0;
	int centieme = 0;
	int seconde_at_hidding = 0;
	int dizaine_seconde_at_hidding = 0;
	int flag_hide = 0;
	int time_to_guess = 0;
	int time = 0;
	int val_left, val_right;
		int time_pressed_p1 = 0;
		int time_pressed_p2 = 0;
		int flag_p1 = 0;
		int flag_p2 = 0;
	while(true) { 
		// Init Left and Right buttons
		/*int val_left, val_right, btns_pressed_nb = 0;		
		wiringPiSetup();
		pinMode(BTN_LEFT, INPUT);
		pinMode(BTN_RIGHT, INPUT);
		digitalWrite(BTN_LEFT, LOW);
		digitalWrite(BTN_RIGHT, LOW);*/
		
		wiringPiSetup();

		pinMode(BTN_LEFT, INPUT);
		pinMode(BTN_RIGHT, INPUT);

		digitalWrite(BTN_LEFT, HIGH);
		digitalWrite(BTN_RIGHT, HIGH);

		
			val_left = digitalRead(BTN_LEFT);
			val_right = digitalRead(BTN_RIGHT);
			if(val_left == LOW && flag_p1 == 0) { time_pressed_p1 = time; flag_p1 = 1; }
			if(val_right == LOW && flag_p2 == 0) { time_pressed_p2 = time; flag_p2 = 1; } 
			//printf("time_pressed_p1=%d, time_pressed_p2=%d\n", time_pressed_p1, time_pressed_p2);
			if(flag_p1 == 1 && flag_p2 == 1) exit(0);
			printf("flag_p1=%d, flag_p2=%d\n", flag_p1, flag_p2);
		// Detect pressed button(s)
		/*while(btns_pressed_nb < 2){
			val_left = digitalRead(BTN_LEFT);
			val_right = digitalRead(BTN_RIGHT);
			if(val_left == LOW ) {btns_pressed_nb++;}
			if(val_right == LOW ) {btns_pressed_nb++;} 
		}*/
		//printf("val_left=%d, val_right=%d, btns_pressed_nb=%d\n", val_left, val_right, btns_pressed_nb);

		// Launch 4 digits hidding
		if((dizaine_seconde == dizaine_seconde_at_hidding) && (seconde == seconde_at_hidding)){
			printf("TIME TO HIDE!\n");
			flag_hide = 1;
		} 
			
		// Incr digits depending on the time
		if(centieme == 9){ 
			centieme=0;
			if(dizieme == 9){ 
				dizieme = 0;
				if(seconde == 9){
					seconde = 0;
					if(dizaine_seconde == 5){
						exit(0);
					}
					else dizaine_seconde++;
				}
				else seconde++;
			}
			else dizieme++; 
		}
		else centieme++;
		
		// Display digits depending on the time
		if(flag_hide == 0){	
			// display figures
			/*displaybuffer[0] = numbertable[dizaine_seconde];
			displaybuffer[1] = numbertable[seconde];
			displaybuffer[2] = numbertable[dizieme];
			displaybuffer[3] = numbertable[centieme];*/
		}
		else{
			// display hyphens 
			/*displaybuffer[0] = numbertable[16];
			displaybuffer[1] = numbertable[16];
			displaybuffer[2] = numbertable[16];
			displaybuffer[3] = numbertable[16];*/
		}
			
		// each 10 milliseconds
		usleep(6000);
		//writeDisplay(fda, 0);
		time++;	 
	}

	return 0;
}
