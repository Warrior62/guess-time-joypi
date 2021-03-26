#include <wiringPi.h>           //WiringPi headers
#include <lcd.h>                //LCD headers from WiringPi
#include <stdio.h> 
#include <mcp23008.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


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



int initLcd();
void ecrireLcd(int lcdHandle, int ligne, int colonne, char* msg);
void lcd(int level);
