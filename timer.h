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


void setBrightness(int fd, uint8_t b);
void blinkRate(int fd, uint8_t b);
void writeDisplay(int fd, int offset);
void clear(void);
int generateRandomNb(int min, int max);
void timer();
int initBtns();
