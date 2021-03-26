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

int initLcd();
void ecrireLcd(int lcdHandle, int ligne, int colonne, char* msg);

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
///////// BUTTONS /////////////
///////////////////////////////


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

int lcd(int level, int random){
	int lcdHandle, time_to_guess = 0;
	char msg[50], randomChar[50];	
	lcdHandle = initLcd();
	ecrireLcd(lcdHandle, 2, 16, "");
		

	switch(level){
		case 1:
			strcpy(msg, "Niveau 1 !      Guess Time=");
		break;
		case 2:
			strcpy(msg, "Niveau 2 !      Guess Time=");
		break;
		case 3:
			strcpy(msg, "Niveau 3 !      Guess Time=");
		break;
	}
	sprintf(randomChar, "%d", random);
	strcat(msg, randomChar);
	ecrireLcd(lcdHandle, 2, 16, msg);
	sleep(5);
	
	return lcdHandle;
}



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
	int time_pressed_p1 = 0;
	int time_pressed_p2 = 0;
	int flag_p1 = 0;
	int flag_p2 = 0;
	int lcdHandle, level;
	char msg[50], random_char[50];

	/*********************
	****** NIVEAU°1 ******
	*********************/	
	level = 1;
	wiringPiSetup();
	digitalWrite(BTN_LEFT, HIGH);
	digitalWrite(BTN_RIGHT, HIGH);
	pinMode(BTN_LEFT, INPUT);
	pinMode(BTN_RIGHT, INPUT);
	
	srand(time(0));
	random = generateRandomNb(10, 19);
	lcdHandle = lcd(1, random);
		
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
	
	while(flag_p1 == 0 || flag_p2 == 0) { 
		
		// Exit while loop when level max time is reached
		if(time_count == 2000){ 
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
			
		// each 10 milliseconds
		usleep(8000);
		writeDisplay(fda, 0);
		time_count++;	

		val_left = digitalRead(BTN_LEFT);
		val_right = digitalRead(BTN_RIGHT);
		printf("time_count=%d\n", time_count);
		if(val_left == LOW && flag_p1 == 0) { time_pressed_p1 = time_count; flag_p1 = 1; }
		if(val_right == LOW && flag_p2 == 0) { time_pressed_p2 = time_count; flag_p2 = 1; } 
		if(flag_p1 == 1 && flag_p2 == 1){
			random *= 100;
			if(abs(time_pressed_p1-random) == abs(time_pressed_p2-random)){
				ecrireLcd(lcdHandle, 2, 16, "No winner !");
			} 
			else{
				lcdClear(lcdHandle);
				if(abs(time_pressed_p1-random) < abs(time_pressed_p2-random)){
					points_player1++;
					ecrireLcd(lcdHandle, 2, 16, "1pt goes to     GREEN!");
				}
				if(abs(time_pressed_p1-random) > abs(time_pressed_p2-random)){
					points_player2++;
					ecrireLcd(lcdHandle, 2, 16, "1pt goes to     YELLOW!");
				}
			}
			 
		} 
	}
	dizaine_seconde = 0;
	seconde = 0;
	dizieme = 0;
	centieme = 0;
	dizaine_seconde_at_hidding = 0;
	seconde_at_hidding = 0;
	flag_hide = 0;
	time_count = 0;
	flag_p1 = 0;
	flag_p2 = 0;
	sleep(5);
	lcdClear(lcdHandle);


	/*********************
	****** NIVEAU°2 ******
	*********************/
	random = generateRandomNb(20, 29);
	displaybuffer[0] = numbertable[0];
	displaybuffer[1] = numbertable[0];
	displaybuffer[2] = numbertable[0];
	displaybuffer[3] = numbertable[0];
	writeDisplay(fda, 0);	
	strcpy(msg, "Niveau 2 !      Guess Time=");
	sprintf(random_char, "%d", random);	
	strcat(msg, random_char);	
	lcdPuts(lcdHandle, msg);
	// niveau 2 = on cache tous les digits à 5 secondes, et on devine entre 20 et 29
	dizaine_seconde_at_hidding = 0;
	seconde_at_hidding = 5;
	time_to_guess = generateRandomNb(20, 29);
	sleep(5);

	while(flag_p1 == 0 || flag_p2 == 0) { 
		
		// Exit while loop when level max time is reached
		if(time_count == 3500){ 
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
			
		// each 10 milliseconds
		usleep(8000);
		writeDisplay(fda, 0);
		time_count++;	

		val_left = digitalRead(BTN_LEFT);
		val_right = digitalRead(BTN_RIGHT);
		printf("time_count=%d\n", time_count);
		if(val_left == LOW && flag_p1 == 0) { time_pressed_p1 = time_count; flag_p1 = 1; }
		if(val_right == LOW && flag_p2 == 0) { time_pressed_p2 = time_count; flag_p2 = 1; } 
		if(flag_p1 == 1 && flag_p2 == 1){
			random *= 100;
			if(abs(time_pressed_p1-random) == abs(time_pressed_p2-random)){
				ecrireLcd(lcdHandle, 2, 16, "No winner !");
			} 
			else{
				lcdClear(lcdHandle);
				if(abs(time_pressed_p1-random) < abs(time_pressed_p2-random)){
					points_player1++;
					ecrireLcd(lcdHandle, 2, 16, "1pt goes to     GREEN!");
				}
				if(abs(time_pressed_p1-random) > abs(time_pressed_p2-random)){
					points_player2++;
					ecrireLcd(lcdHandle, 2, 16, "1pt goes to     YELLOW!");
				}
			} 
		} 
	}
	dizaine_seconde = 0;
	seconde = 0;
	dizieme = 0;
	centieme = 0;
	dizaine_seconde_at_hidding = 0;
	seconde_at_hidding = 0;
	flag_hide = 0;
	time_count = 0;
	flag_p1 = 0;
	flag_p2 = 0;	
	sleep(5);
	lcdClear(lcdHandle);

	printf("Score p1=%d\n", points_player1);
	printf("Score p2=%d\n", points_player2);


	/*********************
	****** NIVEAU°3 ******
	*********************/
	random = generateRandomNb(20, 29);
	displaybuffer[0] = numbertable[0];
	displaybuffer[1] = numbertable[0];
	displaybuffer[2] = numbertable[0];
	displaybuffer[3] = numbertable[0];
	writeDisplay(fda, 0);	
	strcpy(msg, "Niveau 3 !      Guess Time=");
	sprintf(random_char, "%d", random);	
	strcat(msg, random_char);	
	lcdPuts(lcdHandle, msg);
	// niveau 3 = on cache dès le début la seconde mais après 5 on cache tout et on devine entre 20 et 39
	dizaine_seconde_at_hidding = 0;
	seconde_at_hidding = 5;				
	time_to_guess = generateRandomNb(20, 39);	
	sleep(5);


	while(flag_p1 == 0 || flag_p2 == 0) { 
		
		// Exit while loop when level max time is reached
		if(time_count == 4500){ 
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
		if(time_count < 500) displaybuffer[1] = numbertable[16];	
		// each 10 milliseconds
		usleep(8000);
		writeDisplay(fda, 0);
		time_count++;	

		val_left = digitalRead(BTN_LEFT);
		val_right = digitalRead(BTN_RIGHT);
		printf("time_count=%d\n", time_count);
		if(val_left == LOW && flag_p1 == 0) { time_pressed_p1 = time_count; flag_p1 = 1; }
		if(val_right == LOW && flag_p2 == 0) { time_pressed_p2 = time_count; flag_p2 = 1; } 
		if(flag_p1 == 1 && flag_p2 == 1){
			random *= 100;
			if(abs(time_pressed_p1-random) == abs(time_pressed_p2-random)){
				ecrireLcd(lcdHandle, 2, 16, "No winner !");
			} 
			else{
				lcdClear(lcdHandle);
				if(abs(time_pressed_p1-random) < abs(time_pressed_p2-random)){
					points_player1++;
					ecrireLcd(lcdHandle, 2, 16, "1pt goes to     GREEN!");
				}
				if(abs(time_pressed_p1-random) > abs(time_pressed_p2-random)){
					points_player2++;
					ecrireLcd(lcdHandle, 2, 16, "1pt goes to     YELLOW!");
				}
			} 
		} 
	}
	dizaine_seconde = 0;
	seconde = 0;
	dizieme = 0;
	centieme = 0;
	dizaine_seconde_at_hidding = 0;
	seconde_at_hidding = 0;
	flag_hide = 0;
	time_count = 0;
	flag_p1 = 0;
	flag_p2 = 0;	
	sleep(5);
	lcdClear(lcdHandle);

	printf("Score p1=%d\n", points_player1);
	printf("Score p2=%d\n", points_player2);
	

	/**********************************
	****** AFFICHER LE VAINQUEUR ******
	**********************************/
	if(points_player1 > points_player2) lcdPuts(lcdHandle, "WINNER IS       GREEN !"); 
	else if(points_player1 < points_player2) lcdPuts(lcdHandle, "WINNER IS       YELLOW !");

	return 0;
}
