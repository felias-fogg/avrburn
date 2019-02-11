// -*- c++ -*-

/* Changelog:
 *
 * V0.1 (10.2.2019):
 *    - First version: MCU type detection works
 * V0.2 (10.2.2019):
 *    - added extract.py, which now goes through all atdf files of the Atmel chips.
 *      This adds, however, 4k to the code. So, maybe, I will remove a few defs.
 *    - changed to soft spi for isp programming. This was necessary, because
 *      the programmed chips may use the programming lines afterwards! 
 */

#define DEBUG

#define VERSION "0.2"

#define SPLASH_MS 1000UL*15

#include <avr/wdt.h>
#include <stdio.h>
#include <ctype.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Gamebuino.h>
#include <SD.h>
#include "menus.h"
#include "pics.h"
#include "mcus.h"
#include "ports.h"

// number of els
#define NUMELS(x) (sizeof(x)/sizeof(x[0])) 

// Pins
#define SOFT_RST C,5
#define SOFT_MOSI C,4
#define SOFT_MISO B,3
#define SOFT_SCK B,5
#define SPI_CONFLICT 1 // states that there is a conflict with the SPI lines (should be =0 for production)

// States
#define NO_STATE 0
#define RESET_STATE 1
#define MENU_STATE 2
#define SETTINGS_STATE 3
#define SPEED_STATE 4
#define DETECT_STATE 5
#define SHOW_MCU_STATE 6
#define ERASE_STATE 7
#define LOCK_STATE 8
#define FLASH_STATE 9
#define EEP_STATE 10
#define FUSE_STATE 11
#define ERROR_STATE 12

// Error codes
#define NO_ERROR 0
#define STATE_ERROR 1
#define NYI_ERROR 2
#define NYI_DETECT 3
#define NYI_ERASE 4
#define NYI_LOCKS 5
#define NYI_FUSES 6
#define NYI_FLASH 7
#define NYI_EEPROM 8
#define NYI_SETTINGS 9
#define SD_ERROR 10
#define SIG_ERROR 11
#define UNKNOWN_SIG_ERROR 12
#define NO_BURN_ERROR 13

// SPI devices
#define SPI_DISP 0
#define SPI_SD 1

// software spi speed
#define SPI_SPEED0 5 // 100 kHz
#define SPI_SPEED1 4 // 125 kHz
#define SPI_SPEED2 3 // 166 kHz
#define SPI_SPEED3 2 // 250 kHz
#define SPI_SPEED4 1 // 500 kHz

// KINDs
#define LOCKBIT_KIND 0
#define FUSE_KIND 1
#define FLASH_KIND 2
#define EEP_KIND 3

// timer
#define BUSY_MS 10000UL // longest time we consider us self busy 


#define NO_BTN 255

//extern const byte font3x5[]; //a small but efficient font (default)
extern const byte font5x7[]; //a large, comfy font

Gamebuino gb;
byte state;
byte error;
char settings[SETTINGS_LENGTH] = { 0, -1, -1, 0};
char speedopt[SPEED_LENGTH] = { 2, -2, -2, -2,  -2, 0};
char spidelayvals[SPEED_LENGTH-1] = { SPI_SPEED0, SPI_SPEED1, SPI_SPEED2, SPI_SPEED3, SPI_SPEED4 };
int spidelay = SPI_SPEED0;
int progspidelay = SPI_SPEED0;
unsigned int mcusig;
int mcuix = -1;
boolean progmode = false;
boolean sdinit = false;

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

// This guards against reset loops caused by resets with active WDT
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3"))) __attribute__((used));
void wdt_init(void)
{
  MCUSR = 0;
  wdt_disable();
  return;
}


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
  state = MENU_STATE;
  error = NO_ERROR;
}


void loop() {
  set_spi_para(SPI_DISP);
  if (gb.update()) {
    check_reset();
    if (error && state != RESET_STATE) state = ERROR_STATE;
    DEBPR(F("loop: state="));
    DEBLN(state);
    switch (state) {
    case RESET_STATE: wdt_enable(WDTO_15MS); break;
    case MENU_STATE: main_menu(); break;
    case SETTINGS_STATE: settings_menu(); break;
    case SPEED_STATE: speed_menu(); break;
    case DETECT_STATE: mcu_detect(); break;
    case SHOW_MCU_STATE: display_mcu(); break;
    case ERASE_STATE: erase_chip(); if (state != RESET_STATE) state = MENU_STATE; break;
    case LOCK_STATE: rwv_menu(LOCKBIT_KIND); break;
    case FLASH_STATE:rwv_menu(FLASH_KIND); break;
    case EEP_STATE:rwv_menu(EEP_KIND); break;
    case FUSE_STATE: fuse_menu(); break;
    case ERROR_STATE: display_error(error);  break;
    default: error = STATE_ERROR; break;
    }
  }
}

void main_menu()
{
  int item = menu(mainmenu, NULL, MAINMENU_LENGTH);
  if (state == RESET_STATE) return;
  switch (item) {
  case -1: return;
  case MM_EXIT: state = RESET_STATE; break;
  case MM_SETTINGS: state = SETTINGS_STATE; break;
  case MM_DETECT: state = DETECT_STATE; break;
  case MM_ERASE: state = ERASE_STATE; gb.display.clear(); gb.display.println(F("\nErasing chip ...")); break;
  case MM_LOCK: state = LOCK_STATE; break;
  case MM_FUSE: state = FUSE_STATE; break;
  case MM_EEP: state = EEP_STATE; break;
  case MM_FLASH: state = FLASH_STATE; break;
  default: error = NYI_ERROR + item; break;
  }
  DEBPR(F("main_menu exit: state="));
  DEBLN(state);
}

void settings_menu()
{
  int item = menu(settingsmenu, settings, SETTINGS_LENGTH);
  if (state == RESET_STATE) return;
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
    if (speedopt[i] == 2) progspidelay = spidelayvals[i];
  if (state != RESET_STATE) state = SETTINGS_STATE;
}

void rwv_menu(byte kind)
{
  byte item = menu(rwvmenu, NULL, RWV_LENGTH);
  if (state == RESET_STATE) return;
  switch(item) {
  case RWV_READ: read_file(kind); break;
  case RWV_WRITE: write_file(kind); break;
  case RWV_VERIFY: verify_file(kind); break;
  }
  state = MENU_STATE; 
}

void fuse_menu()
{
  byte item = menu(fusemenu, NULL, FUSE_LENGTH);
  if (state == RESET_STATE) return;
  switch(item) {
  case FUSE_READ: read_file(FUSE_KIND); break;
  case FUSE_WRITE: write_file(FUSE_KIND); break;
  case FUSE_VERIFY: verify_file(FUSE_KIND); break;
  case FUSE_DEFAULT: set_default_fuse(); break;
  }
  state = MENU_STATE; 
}

void read_file(byte kind)
{
  choose_file(kind);
}

void write_file(byte kind)
{
}

void verify_file(byte kind)
{
}

void set_default_fuse()
{
}

void choose_file(byte kind)
{
  File dir;
  int entrycnt = 0;
  char * name;
  DEBLN(F("choose_file"));
  gb.display.clear();
  gb.display.fontSize = 2;
  gb.display.println(F("OPEN SD"));
  gb.display.update();
  openSD();
  if (error == NO_ERROR) {
    dir = SD.open("/BURN");
    if (!dir || !(dir.isDirectory())) {
      error = NO_BURN_ERROR;
      return;
    }
  }
  DEBLN(F("SD opened"));
  File entry = dir.openNextFile();
  set_spi_para(SPI_DISP);
  DEBLN(F("1st file in BURN opened"));
  DEBLN(entry.name());
  DEBLN(entry.size());
  while (true) {
    if (gb.update()) {
      gb.display.clear();
      gb.display.fontSize = 2;
      gb.display.println(F("Choose"));
      gb.display.fontSize = 1;
      gb.display.println(entry.name());
      if (gb.buttons.pressed(BTN_C)) {
	state = RESET_STATE;
	return;
      }
      if (gb.buttons.pressed(BTN_A)) {
	state = MENU_STATE;
	return;
      }
    }
  }
  set_spi_para(SPI_SD);
  dir.close();
  DEBLN(F("choose_file exit"));
}

void openSD()
{
  if (!sdinit) {
    if (!SD.begin()) error = SD_ERROR;
    get_spi_para(SPI_SD);
    sdinit = true;
  } else set_spi_para(SPI_SD);
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
    case NO_BURN_ERROR:
      gb.display.println(F("SD-Card does"));
      gb.display.println(F("not contain"));
      gb.display.println(F("BURN folder"));
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
  DEBPR(F("sig="));
  mcusig = sig;
  prog_mode(false);
  if (state != ERROR_STATE) state = SHOW_MCU_STATE;
}

void display_mcu()
{
  DEBLN(F("display_mcu"));
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
  for (int i=0; i < NUMELS(mcuList); i++) {
    DEBPR(F("Loop: "));
    DEBPR(i);
    DEBPR(F(" "));
    DEBPRF(sig,HEX);
    DEBPR(F(" "));
    DEBLNF(pgm_read_word(&(mcuList[i].signature)),HEX);
    if (pgm_read_word(&(mcuList[i].signature))  == sig) {
      DEBLN(F("Found"));
      mcuix = i;
      return pgm_read_word(&(mcuList[i].name));
    }
  }
  error = UNKNOWN_SIG_ERROR;
  return pgm_read_word(&(mcuList[0].name));
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
	if (gb.buttons.pressed(BTN_C)) state = RESET_STATE;
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
	  switch (opts[i]) {
	  case 1:
	    gb.display.fillRect(LCDWIDTH - 3 - gb.display.fontWidth, currentY + gb.display.fontHeight * i, gb.display.fontWidth, gb.display.fontHeight - 1);
	    break;
	  case 2:
	    gb.display.fillCircle(LCDWIDTH - 3 - gb.display.fontWidth/2, currentY + gb.display.fontHeight * i +  gb.display.fontHeight/2 -1 , gb.display.fontWidth/2);
	    break;
	  case -1:
	    gb.display.drawRect(LCDWIDTH - 3 - gb.display.fontWidth, currentY + gb.display.fontHeight * i, gb.display.fontWidth, gb.display.fontHeight - 1);
	    break;
	  case -2:
	    gb.display.drawCircle(LCDWIDTH - 3 - gb.display.fontWidth/2, currentY + gb.display.fontHeight * i +  gb.display.fontHeight/2 -1 , gb.display.fontWidth/2);
	    break;
	  }
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


