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
#include <mcp23008.h>
#include <lcd.h>
#include <time.h>

#define HT16K33_BLINK_CMD 0x80 
#define HT16K33_BLINK_DISPLAYON 0x01 
#define HT16K33_BLINK_OFF 0 
#define HT16K33_BLINK_2HZ  1 
#define HT16K33_BLINK_1HZ  2 
#define HT16K33_BLINK_HALFHZ  3 
#define HT16K33_CMD_BRIGHTNESS 0xE0 
#define SEVENSEG_DIGITS 5 

#define ADDR_MULTI 0x21
#define LCD_BASE 100
#define AF_BASE 100
#define AF_E (AF_BASE+2)
#define AF_RW (AF_BASE+14)
#define AF_RS (AF_BASE+1)
#define AF_DB4 (AF_BASE+3)
#define AF_DB5 (AF_BASE+4)
#define AF_DB6 (AF_BASE+5)
#define AF_DB7 (AF_BASE+6)
#define AF_BLUE (AF_BASE+7)

#define BTN_LEFT  6
#define BTN_RIGHT 24



int points_player1 = 0;
int points_player2 = 0;



///////////////////////////////
///////// TIMER ///////////////
///////////////////////////////
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
	if(n > max) n = max;
	return n;
}


///////////////////////////////
///////// LCD /////////////////
///////////////////////////////
int initLcd(){
	int lcdHandle;
	wiringPiSetupSys();
	mcp23008Setup(AF_BASE, ADDR_MULTI);
	
	int colour = 1;
	colour &! 7;
	pinMode(AF_BLUE, OUTPUT);
	digitalWrite(AF_BLUE, !(colour & 4));
	lcdHandle = lcdInit (2, 16, 4, AF_RS, AF_E, AF_DB4,AF_DB5,AF_DB6,AF_DB7, 0,0,0,0) ;
	
	if(lcdHandle < 0){
		printf("lcdInit failed\n");
		return -1;
	}
	return lcdHandle;
}


void ecrireLcd(int lcdHandle, int ligne, int colonne, char* msg){
	lcdPosition(lcdHandle, ligne, colonne);
	lcdPuts(lcdHandle, msg);
}


///////////////////////////////
///////// GAME ////////////////
///////////////////////////////
int main() {
	int random;
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
	int time_count = 0;
	int val_left, val_right;
	int time_pressed_green = 0;
	int time_pressed_yellow = 0;
	int flag_green = 0;
	int flag_yellow = 0;
	char msg[50], random_char[50];
	int min_generate_random_nb, max_generate_random_nb;
	int level = 0;
	int max_time_count;

	// Init left and right btn, and the lcd display
	wiringPiSetup();
	digitalWrite(BTN_LEFT, HIGH);
	digitalWrite(BTN_RIGHT, HIGH);
	pinMode(BTN_LEFT, INPUT);
	pinMode(BTN_RIGHT, INPUT);
	int lcdHandle = initLcd();

	/*************************
	****** WHILE LOOP ********
	*************************/
	while(true) {
		begin(fda);
		strcpy(random_char, "");
		level++;		

		// Set levels settings
		switch(level){
			// Level°1 : hide the 4 digits at 8sec,
			//	      then we guess the time between 10sec and 19sec
			case 1: 
				min_generate_random_nb = 10;
				max_generate_random_nb = 19;
				dizaine_seconde_at_hidding = 0;
				seconde_at_hidding = 8;
				max_time_count = 2500;
				srand(time(0));
				strcpy(msg, "Niveau 1 !      Guess Time=");
			break;

			// Level°2 : hide the 4 digits at 5sec,
			//            then we guess the time between 10sec and 19sec
			case 2: 
				min_generate_random_nb = 20;
				max_generate_random_nb = 29;
				dizaine_seconde_at_hidding = 0;
				seconde_at_hidding = 5;
				max_time_count = 3500;
				strcpy(msg, "Niveau 2 !      Guess Time=");
			break;

			// Level°3 : hide since the beginning the seconde digit but after 5sec,
			//            we hide all the digits and then we guess the time between 20sec and 39sec
			case 3:
				min_generate_random_nb = 20;
				max_generate_random_nb = 39;
				dizaine_seconde_at_hidding = 0;
				seconde_at_hidding = 5;
				max_time_count = 4500; 
				strcpy(msg, "Niveau 3 !      Guess Time=");
			break;
		}

		// Generate a time number to display on lcd display
		// and display it 
		random = generateRandomNb(min_generate_random_nb, max_generate_random_nb);	
		sprintf(random_char, "%d", random);	
		strcat(msg, random_char);	
		strcat(msg, "s");
		lcdPuts(lcdHandle, msg);
		sleep(5);

		// Game's While loop
		while(flag_green == 0 || flag_yellow == 0) { 
			
			// Exit while loop when level max time is reached
			if(time_count == max_time_count){ 
				ecrireLcd(lcdHandle, 2, 16, "No winner !");
				break;
			}

			// Launch 4 digits hidding
			if((dizaine_seconde == dizaine_seconde_at_hidding) && (seconde == seconde_at_hidding)){
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
			if(level == 3){
				if(time_count < 500){
					displaybuffer[1] = numbertable[16];
				}
			}
			// each 10 milliseconds
			usleep(5500);
			writeDisplay(fda, 0);
			time_count++;	

			val_left = digitalRead(BTN_LEFT);
			val_right = digitalRead(BTN_RIGHT);
			
			if(val_left == LOW && flag_green == 0) { time_pressed_green = time_count; flag_green = 1; }
			if(val_right == LOW && flag_yellow == 0) { time_pressed_yellow = time_count; flag_yellow = 1; } 
			if(flag_green == 1 && flag_yellow == 1){
				random *= 100;
				if(abs(time_pressed_green-random) == abs(time_pressed_yellow-random)){
					ecrireLcd(lcdHandle, 2, 16, "No winner !");
				} 
				else{
					lcdClear(lcdHandle);
					if(abs(time_pressed_green-random) < abs(time_pressed_yellow-random)){
						points_player1++;
						ecrireLcd(lcdHandle, 2, 16, "1pt goes to     GREEN!");
					}
					if(abs(time_pressed_green-random) > abs(time_pressed_yellow-random)){
						points_player2++;
						ecrireLcd(lcdHandle, 2, 16, "1pt goes to     YELLOW!");
					}
				}
				 
			} 
		}

		// Set to 0 all the time anf flags variables	
		flag_green = 0;
		flag_yellow = 0;
		flag_hide = 0;
		dizaine_seconde = 0;
		seconde = 0;
		dizieme = 0;
		centieme = 0;
		time_count = 0;
		
		// Reset timer and lcd display 
		displaybuffer[0] = numbertable[16];
		displaybuffer[1] = numbertable[16];
		displaybuffer[2] = numbertable[16];
		displaybuffer[3] = numbertable[16];
		sleep(5);
		lcdClear(lcdHandle);
		
		/**********************************
		****** DISPLAY THE WINNER *********
		**********************************/
		if(level == 3){
			if(points_player1 > points_player2) lcdPuts(lcdHandle, "WINNER IS       GREEN !"); 
			else if(points_player1 < points_player2) lcdPuts(lcdHandle, "WINNER IS       YELLOW !");
			exit(0);
		}
	}

	return 0;
}
