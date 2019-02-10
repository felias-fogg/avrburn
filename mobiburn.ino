// -*- c++ -*-

/* Changelog:
 *
 * V0.1 (10.2.2019):
 *    - First version: MCU type detection works
 */

// #define DEBUG

#define VERSION "0.1"

#define SPLASH_MS 1000UL*15

#include <stdio.h>
#include <ctype.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Gamebuino.h>
#include <SD.h>
#include "menus.h"
#include "pics.h"
#include "mcus.h"

// Pins
#define RST_PIN 19

// States
#define NO_STATE 0
#define RESET_STATE 1
#define MENU_STATE 2
#define SETTINGS_STATE 3
#define SPEED_STATE 4
#define DETECT_STATE 5
#define SHOW_MCU_STATE 6
#define ERROR_STATE 7

// Error codes
#define NO_ERROR 0
#define STATE_ERROR 1
#define NYI_ERROR 2
#define NYI_DETECT 3
#define NYI_LOCKS 4
#define NYI_FUSES 5
#define NYI_FLASH 6
#define NYI_EEPROM 7
#define NYI_SETTINGS 8
#define SD_ERROR 9
#define SIG_ERROR 10
#define UNKNOWN_SIG_ERROR 11 

// SPI devices
#define SPI_DISP 0
#define SPI_SD 1
#define SPI_ISP 2


#define NO_BTN 255

//extern const byte font3x5[]; //a small but efficient font (default)
extern const byte font5x7[]; //a large, comfy font

Gamebuino gb;
byte state;
byte error;
char settings[SETTINGS_LENGTH] = {0, -1, -1, 0};
char speedopt[SPEED_LENGTH] = {-2, -2, -2, -2, -2,  2, 0};
byte spiclockdiv[SPEED_LENGTH-1] = {SPI_CLOCK_DIV4, SPI_CLOCK_DIV8,
			       SPI_CLOCK_DIV16, SPI_CLOCK_DIV32,
			       SPI_CLOCK_DIV64, SPI_CLOCK_DIV128};
int spispeed = SPI_CLOCK_DIV128;
unsigned int mcusig;
int mcuix = -1;

#ifdef DEBUG
#define DEBPR(str) Serial.print(str)
#define DEBPRF(num,len) Serial.print(num,len)
#define DEBWR(c)   Serial.write(c)
#define DEBLN(str) Serial.println(str)
#define DEBLNF(num,len) Serial.println(num,len)
#else
#define DEBPR(str)
#define DEBPRF(num,len)
#define DEBWR(c) 
#define DEBLN(str)
#define DEBLNF(num,len) 
#endif


void setup() {
#ifdef DEBUG
  Serial.begin(57600);
  Serial.println(F("MobiBurn V" VERSION " starts..."));
#endif
  pinMode(10, OUTPUT); // chip select should be output!
  gb.begin();
  get_spi_para(SPI_DISP);
  //gb.display.setFont(font3x5);
  gb.display.setFont(font5x7);
  gb.titleScreen(F("MobiBurn" " V" VERSION),fire);
  gb.display.persistence = true;
  if (!SD.begin()) error = SD_ERROR;
  get_spi_para(SPI_SD);
  state = MENU_STATE;
  error = NO_ERROR;
}


void loop() {
  set_spi_para(SPI_DISP);
  if (gb.update()) {
    check_reset();
    if (error && state != RESET_STATE) state = ERROR_STATE;
    switch (state) {
    case RESET_STATE: setup(); break;
    case MENU_STATE: main_menu(); break;
    case SETTINGS_STATE: settings_menu(); break;
    case SPEED_STATE: speed_menu(); break;
    case DETECT_STATE: mcu_detect(); break;
    case SHOW_MCU_STATE: display_mcu(); break;
    case ERROR_STATE: display_error(error);  break;
    default: error = STATE_ERROR; break;
    }
  }
}

void main_menu()
{
  int item = menu(mainmenu, NULL, MAINMENU_LENGTH);
  switch (item) {
  case -1: break;
  case MM_EXIT: state = RESET_STATE; break;
  case MM_SETTINGS: state = SETTINGS_STATE; break;
  case MM_DETECT: state = DETECT_STATE; break;
  default: error = NYI_ERROR + item; break;
  }
}

void settings_menu()
{
  int item = menu(settingsmenu, settings, SETTINGS_LENGTH);
  switch (item) {
  case -1:
  case ST_EXIT: state = MENU_STATE; break;
  case ST_SPISPEED: state = SPEED_STATE; break;
  default: error = NYI_ERROR + item; break;
  }
}

void speed_menu()
{
  menu(speedmenu, speedopt, SPEED_LENGTH);
  for (byte i = 0; i < SPEED_LENGTH; i++)
    if (speedopt[i] == 2) spispeed = spiclockdiv[i];
  state = SETTINGS_STATE;
}


void check_reset(void) {
  if (gb.buttons.pressed(BTN_C)) state = RESET_STATE;
}

void display_error(byte errnum)
{
  gb.display.clear();
  gb.display.fontSize = 2;
  gb.display.println(F(" ERROR"));
  gb.display.fontSize = 1;
  gb.display.println();
  if (errnum >= NYI_ERROR && errnum <= NYI_SETTINGS) {
    gb.display.print(F("     NYI: "));
    gb.display.println(errnum-NYI_ERROR);
  } else  {
    switch (errnum) {
    case SD_ERROR:
      gb.display.println(F("SD-Card"));
      gb.display.println(F("not accessible"));
      break;
    case STATE_ERROR:
      gb.display.println(F("Internal"));
      gb.display.println(F("Confusion"));
      break;
    case SIG_ERROR:
      gb.display.println(F("MCU signature"));
      gb.display.println(F("unreadable"));
      break;
    case UNKNOWN_SIG_ERROR:
      gb.display.println(F("Unknown MCU"));
      gb.display.print(F("signature:"));
      gb.display.println(mcusig,HEX);
      break;
    default:
      gb.display.println(F("Unknown"));
      gb.display.print(F("Error: "));
      gb.display.println(errnum);
      break;
    }
  }
}

void mcu_detect()
{
  uint16_t sig;
  DEBLN(F("mcu_detect"));
  prog_mode(true);
  sig = read_sig();
  DEBLNF(sig,HEX);
  if (error) return;
  DEBPR(F("sig="));
  mcusig = sig;
  prog_mode(false);
  state = SHOW_MCU_STATE;
}

void prog_mode(boolean on)
{
  if (on) {
    init_isp_spi();
    pinMode(SCK_PIN, OUTPUT);
    digitalWrite(SCK_PIN, LOW);
    delay(50);
    pinMode(RST_PIN, OUTPUT);
    digitalWrite(RST_PIN, LOW);
    delay(50);
    spi_transaction(0xAC, 0x53, 0x00, 0x00);
  } else {
    pinMode(RST_PIN, INPUT);
  }
}

uint16_t read_sig()
{
  uint16_t sig;
  init_isp_spi();
  byte vendorid = spi_transaction(0x30, 0x00, 0x00, 0x00) & 0xFF;
  if (vendorid != 0xff && vendorid != 0x00 && vendorid != 0x1E) {
    error = SIG_ERROR;
    return 0;
  }
  sig = spi_transaction(0x30, 0x00, 0x01, 0x00) & 0xFF;
  sig <<= 8;
  sig |= spi_transaction(0x30, 0x00, 0x02, 0x00) & 0xFF;
  return sig;
}

void display_mcu()
{
  if (gb.buttons.pressed(BTN_A) || gb.buttons.pressed(BTN_B) ||
      gb.buttons.pressed(BTN_C)) {
    state = MENU_STATE;
    return;
  }
  gb.display.clear();
  gb.display.fontSize = 2;
  gb.display.println(F("  MCU"));
  gb.display.fontSize = 1;
  gb.display.println();
  gb.display.println((const __FlashStringHelper*)mcu_name(mcusig));
}

uint16_t mcu_name(uint16_t sig)
{
  DEBPR(F("mcu_name:"));
  DEBLNF(sig,HEX);
  for (int i=0; i < MCU_NUM; i++) {
    DEBPR(F("Loop: "));
    DEBPR(i);
    DEBPR(F(" "));
    DEBPRF(sig,HEX);
    DEBPR(F(" "));
    DEBLNF(pgm_read_word(&(mcutypes[i][0])),HEX);
    if (pgm_read_word(&(mcutypes[i][0]))  == sig) {
      DEBLN(F("Found"));
      mcuix = i;
      return pgm_read_word(&(mcutypes[i][3]));
    }
  }
  error = UNKNOWN_SIG_ERROR;
  return pgm_read_word(&(mcutypes[0][3]));
}
    

int8_t menu(const char* const* items, char * opts, uint8_t length) {
  gb.display.persistence = false;
  int8_t activeItem = 0;
  int8_t currentY = LCDHEIGHT;
  int8_t targetY = 0;
  boolean exit = false;
  int8_t answer = -1;
  while (1) {
    if (gb.update()) {
      if (gb.buttons.pressed(BTN_A) || gb.buttons.pressed(BTN_B) || gb.buttons.pressed(BTN_C)) {
	if (opts == NULL || opts[activeItem] == 0 || gb.buttons.pressed(BTN_C) || gb.buttons.pressed(BTN_B)) {
	  exit = true; //time to exit menu !
	  targetY = - gb.display.fontHeight * length - 2; //send the menu out of the screen
	  if (gb.buttons.pressed(BTN_A)) {
	    answer = activeItem;
	    gb.sound.playOK();
	  } else {
	    gb.sound.playCancel();
	  }
	} else if (gb.buttons.pressed(BTN_A)) {
	  if (opts[activeItem] != 2) 
	    opts[activeItem] = -opts[activeItem];
	  if (opts[activeItem] == 2)
	    for (byte i = 0; i < length; i++)
	      if (i != activeItem && opts[i] == 2) opts[i] = -2;
	}
      }
      if (exit == false) {
	if (gb.buttons.repeat(BTN_DOWN,4)) {
	  activeItem++;
	  gb.sound.playTick();
	}
	if (gb.buttons.repeat(BTN_UP,4)) {
	  activeItem--;
	  gb.sound.playTick();
	}
	//don't go out of the menu
	if (activeItem == length) activeItem = 0;
	if (activeItem < 0) activeItem = length - 1;

	targetY = -gb.display.fontHeight * activeItem + (gb.display.fontHeight+4); //center the menu on the active item
      } else { //exit :
	if ((currentY - targetY) <= 1)
	  return (answer);
      }
      //draw a fancy menu
      currentY = (currentY + targetY) / 2;
      gb.display.cursorX = 0;
      gb.display.cursorY = currentY;
      gb.display.fontSize = 1;
      gb.display.textWrap = false;
      for (byte i = 0; i < length; i++) {
	if (i == activeItem){
	  gb.display.cursorX = 3;
	  gb.display.cursorY = currentY + gb.display.fontHeight * activeItem;
	}
	gb.display.println((const __FlashStringHelper*)pgm_read_word(items+i));
	if (opts) {
	  if (opts[i] > 0) gb.display.fillRect(LCDWIDTH - 3 - gb.display.fontWidth, currentY + gb.display.fontHeight * i, gb.display.fontWidth, gb.display.fontHeight - 1);
	  else if (opts[i] < 0) gb.display.drawRect(LCDWIDTH - 3 - gb.display.fontWidth, currentY + gb.display.fontHeight * i, gb.display.fontWidth, gb.display.fontHeight - 1);
	}
      }
      
      //gb.display.fillRect(0, currentY + 3 + 8 * activeItem, 2, 2);
      gb.display.setColor(WHITE);
      gb.display.drawFastHLine(0, currentY + gb.display.fontHeight * activeItem - 1, LCDWIDTH);
      gb.display.setColor(BLACK);
      gb.display.drawRoundRect(0, currentY + gb.display.fontHeight * activeItem - 2, LCDWIDTH, (gb.display.fontHeight+3), 3);
    }
  }
}


