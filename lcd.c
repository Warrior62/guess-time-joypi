#include <wiringPi.h>           //WiringPi headers
#include <lcd.h>                //LCD headers from WiringPi
#include <stdio.h> 
#include <mcp23008.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "lcd.h"

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

void lcd(int level){
	int lcdHandle, time_to_guess = 0;
	char msg[50];	
	lcdHandle = initLcd();
	
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
	ecrireLcd(lcdHandle, 2, 16, msg);
}
