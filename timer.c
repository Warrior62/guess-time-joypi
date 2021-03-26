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

//#include "timer.h"

#define HT16K33_BLINK_CMD 0x80 
#define HT16K33_BLINK_DISPLAYON 0x01 
#define HT16K33_BLINK_OFF 0 
#define HT16K33_BLINK_2HZ  1 
#define HT16K33_BLINK_1HZ  2 
#define HT16K33_BLINK_HALFHZ  3 
#define HT16K33_CMD_BRIGHTNESS 0xE0 
#define SEVENSEG_DIGITS 5 

#define BTN_LEFT 6
#define BTN_RIGHT 24

// tableau de valaurs à afficher sur l'afficheur 7 segments
uint16_t displaybuffer[12]; 

static const uint8_t numbertable[] = { 
	0x3F, /* 0 */ 
	0x06, /* 1 */ 
	0x5B, /* 2 */ 
	0x4F, /* 3 */ 
	0x66, /* 4 */ 
	0x6D, /* 5 */ 
	0x7D, /* 6 */ 
	0x07, /* 7 */ 
	0x7F, /* 8 */ 
	0x6F, /* 9 */ 
	0x77, /* a */ 
	0x7C, /* b */ 
	0x39, /* C */ 
	0x5E, /* d */ 
	0x79, /* E */ 
	0x71, /* F */ 
	0x40  /* - */
}; 

void setBrightness(int fd, uint8_t b) { 
  if (b > 15) b = 15; 
  wiringPiI2CWrite(fd, HT16K33_CMD_BRIGHTNESS | b); 
} 

void blinkRate(int fd, uint8_t b) { 
  if (b > 3) b = 0; // turn off if not sure 
  wiringPiI2CWrite(fd, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1)); 
} 

void begin(int fd) { 
  wiringPiI2CWrite(fd, 0x21); 
  blinkRate(fd, HT16K33_BLINK_OFF); 
  setBrightness(fd, 15); // max brightness 
} 

void writeDisplay(int fd, int offset) { 
  uint8_t addr = (uint8_t) 0x00;	 
  uint8_t i=0; 
  for(i=0 ; i<4 ; i++) { 
	if(i==2) // 2 points 
	    addr+=2;   
        wiringPiI2CWriteReg8(fd, addr++, displaybuffer[i+offset] & 0xFF); 
        wiringPiI2CWriteReg8(fd, addr++, displaybuffer[i+offset] >> 8);  
  }  
} 

void clear(void) { 
  uint8_t i=0; 
  for (i=0; i<12; i++) { 
    displaybuffer[i] = 0; 
  } 
} 

int generateRandomNb(int min, int max) {
	int n = rand() % max + min;
	return n;
}


void timer(int level) 
{
	int fda = wiringPiI2CSetup(0x70); 
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
	//int val_left, val_right, btns_pressed_nb = 0;
	int val_left, val_right;
	int btns_pressed_nb = 0;
	int time_pressed_p1 = 0;
		int time_pressed_p2 = 0;
		int flag_p1 = 0;
		int flag_p2 = 0;
int points_player1 = 0;
int points_player2 = 0;


	begin(fda); 
	 
	clear(); 

	switch(level){
		case 1: // niveau 1 = on cache tous les digits à 8 secondes, et on devine entre 10 et 19
			dizaine_seconde_at_hidding = 0;
			seconde_at_hidding = 8;
			time_to_guess = generateRandomNb(10, 19);
		break;

		case 2: // niveau 2 = on cache tous les digits à 5 secondes, et on devine entre 20 et 29
			dizaine_seconde_at_hidding = 0;
			seconde_at_hidding = 5;
			time_to_guess = generateRandomNb(20, 29);
		break;

		case 3: // niveau 3 = on cache dès le début la seconde mais après 5 on cache tout et on devine entre 20 et 39
			dizaine_seconde_at_hidding = 0;
			seconde_at_hidding = 5;				
			time_to_guess = generateRandomNb(20, 39);
		break;
	}

	wiringPiSetup();

		pinMode(BTN_LEFT, INPUT);
		pinMode(BTN_RIGHT, INPUT);
	/*digitalWrite(BTN_LEFT, HIGH);
		digitalWrite(BTN_RIGHT, HIGH);*/
		int temps = 11;
	while(true) { 
		// Init Left and Right buttons
		/*int val_left, val_right, btns_pressed_nb = 0;		
		wiringPiSetup();
		pinMode(BTN_LEFT, INPUT);
		pinMode(BTN_RIGHT, INPUT);
		digitalWrite(BTN_LEFT, LOW);
		digitalWrite(BTN_RIGHT, LOW);*/
		

		

		
		
	
		val_left = digitalRead(BTN_LEFT);
		val_right = digitalRead(BTN_RIGHT);
		if(val_left == LOW && flag_p1 == 0) { time_pressed_p1 = time; flag_p1 = 1; }
		if(val_right == LOW && flag_p2 == 0) { time_pressed_p2 = time; flag_p2 = 1; } 
		if(flag_p1 == 1 && flag_p2 == 1){
			if(abs(time_pressed_p1-temps) == abs(time_pressed_p2-temps)) printf("No winner ! - %d\n", time_pressed_p1); 
			else{
				if(abs(time_pressed_p1-temps) < abs(time_pressed_p2-temps)){
					points_player1++;
					printf("Winner p1 ! - %d\n", time_pressed_p1);
				}
				if(abs(time_pressed_p1-temps) > abs(time_pressed_p2-temps)){
					points_player2++;
					printf("Winner p2 ! - %d\n", time_pressed_p2);
				}
			} 
			exit(0);
		}
			
		
		// Detect pressed button(s)
		/*while(btns_pressed_nb < 2){
			val_left = digitalRead(BTN_LEFT);
			val_right = digitalRead(BTN_RIGHT);
			if(val_left == LOW ) {btns_pressed_nb++;}
			if(val_right == LOW ) {btns_pressed_nb++;} 
		}*/
		printf("val_left=%d, val_right=%d, btns_pressed_nb=%d\n", val_left, val_right, btns_pressed_nb);

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
			displaybuffer[0] = numbertable[dizaine_seconde];
			displaybuffer[1] = numbertable[seconde];
			displaybuffer[2] = numbertable[dizieme];
			displaybuffer[3] = numbertable[centieme];
		}
		else{
			// display hyphens 
			displaybuffer[0] = numbertable[16];
			displaybuffer[1] = numbertable[16];
			displaybuffer[2] = numbertable[16];
			displaybuffer[3] = numbertable[16];
		}
			
		// each 10 milliseconds
		usleep(6000);
		writeDisplay(fda, 0);
		time++;	 
	}
}

int main(){

	timer(1);
}
