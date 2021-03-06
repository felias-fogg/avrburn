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
 * V0.3 (13.2.2019)
 *    - switch from Gamebuino Classic to Gamebuino META
 *    - File choose menu for different kinds 
 *    - SPI speed adjusted
 * V0.4 (17.2.2019)
 *    - Fuses and lock bits can be programmed, and verified
 *    - Activity screens look similar to menu screens
 *    - Only one NYI error type
 * V0.5 (18.2.2019)
 *    - Fuses and lock bits can be shown, saved, set to defaults. No editing yet.
 *    - EEPROM and Flash can be saved to a file.
 *    - The settings are saved on the SD card.
 * V0.6 (28.2.2019)
 *    - if EEPROM saving, then also 0xFF lines are saved (in contrast to flash memory)
 *    - programming mode is now enabled and disabled globally in avrburn.ino
 *    - the info page shows now also all settings
 *    - programming flash works
 * V0.7 (2.3.2019)
 *    - verifying flash works now
 *    - extended addressing for Mega256 implemented
 *    - saving files for extended adressing works now as well
 *    - erase and programming busy wait implemented 
 *    - eeprom programming works now
 *    - main menu reordered
 *    - restart with old entry also in fuse/lock and flash/eeprom menus
 *    - instead of setting lockbits to factory defaults, we offer chip erase
 *    - Check whether programming address is below max address
 *    - text wrap disabled in order to deal with long file names
 *    - excluding file names starting with a dot 
 *    - Yes/No question before chip erase 
 *    - Similar question for file overwrite
 *    - use B as exit button (menus, left-or-right, choose file)
 * V0.8 (2.3.2019)
 *    - SPI bit banging checked again with scope. Now Speed from 10 kHz to 600 kHz
 * V0.9 (3.3.2019)
 *    - SPI speed settings in spi_transaction corrected
 *    - Verification after setting default values for fuses fixed: needs a restart of the programming mode
 *    - disabling text wrap after each clear!
 *    - DEBUG deactivated!
 * V0.9.1 (5.3.2019)
 *    - switched sound globally off because it leads to annyoingly destorted sound in long menus!
 *    - fuse editing works now
 *    - fuse extractor program works (some fuses still need short explanations)
 *    - new setting: safe fuse editing
 *    - this setting is respected in fuse_edit_menu
 * V0.9.2 (5.3.2019)
 *    - fixed yes/no problem
 * V0.9.3 (5.3.2019)
 *   - now all fuse strings for the tested MCUs are defined
 * V0.9.4 (6.3.2019)
 *   - lockbits editing
 *   - fuse edit works now even if no MCU is connected
 *   - some fuse value problems fixed
 * V0.9.5 (21.3.2019)
 *   - if level shifter is used, then the interface line should be HIGH when inactive (instead of INPUT!)
 *    
 */

/* Tested with: ATtiny12, ATtiny13, ATtiny84, ATtiny85, ATtiny167, ATtiny2313, ATmega8, ATmega48,  ATmega328P, ATMega1284P, ATmega2560, ATmega8535 */

// #define DEBUG

#define LEVELSHIFTER 1 // when level shifter is used: set output lines HIGH (instead of INPUT), when inactive.
#define VERSION "0.9.5"

// number of els
#define NUMELS(x) (sizeof(x)/sizeof(x[0]))

// max string length
#define MAX_STRING_LENGTH 128

// max list length
#define MAX_LIST_LENGTH 128

// max size of page buffer
#define MAX_PAGE_SIZE 256

// maximum number of data bytes in a line of a HEX file
#define MAX_LINE_BYTES 16

#define SOUNDOFF

// includes
#include <Gamebuino-Meta.h>
#include <stdio.h>
#include <string.h>
#include "mcus.h"
#include "menus.h"

// Pins
#define ARDU_PIN_RST 2
#define PORT_RST PORT_PA14
#define ARDU_PIN_MOSI 3
#define PORT_MOSI PORT_PA09
#define ARDU_PIN_MISO 5
#define PORT_MISO PORT_PA15
#define ARDU_PIN_SCK 4
#define PORT_SCK PORT_PA08

// States
#define NO_STATE 0
#define RESET_STATE 1
#define MENU_STATE 2
#define SETTINGS_STATE 3
#define SPEED_STATE 4
#define DETECT_STATE 5
#define ERASE_STATE 7
#define LOCK_STATE 8
#define FLASH_STATE 9
#define EEP_STATE 10
#define FUSE_STATE 11
#define ERROR_STATE 12
#define INFO_STATE 13

// Error codes
#define NO_ERROR 0
#define CONFUSION_ERROR 1
#define NYI_ERROR 2
#define SD_ERROR 3
#define SIG_ERROR 4
#define UNKNOWN_SIG_ERROR 5
#define NO_MCU_ERROR 6
#define NO_BURN_ERROR 7
#define NO_MEM_ERROR 8
#define FILE_DIR_ERROR 9
#define FILE_OPEN_ERROR 10
#define HEX_FILE_ERROR 11
#define REMOVE_ERROR 12
#define FILE_WRITE_ERROR 13
#define HEX_ADDR_ERROR 14
#define HEX_CRC_ERROR 15
#define HEX_RECTYPE_ERROR 16
#define FLASH_PROG_ERROR 17
#define EEPROM_PROG_ERROR 18
#define MAX_ADDR_ERROR 19
#define NO_FUSE_INFO_ERROR 20

// special verifiaction addresses
#define VADDR_LOCK -4
#define VADDR_LO -1
#define VADDR_HI -2
#define VADDR_EX -3

// software spi speed
#define SPI_SPEED0 335 //  10 kHz
#define SPI_SPEED1 165 //  20 kHz
#define SPI_SPEED2 64  //  50 kHz
#define SPI_SPEED3 30  // 100 kHz
#define SPI_SPEED4 13  // 200 kHz
#define SPI_SPEED5 4   // 400 kHz
#define SPI_SPEED6 1   // 600 kHz
#define FUSE_SPI_SPEED SPI_SPEED0

// KINDs
#define LOCKBITS_KIND 0
#define FUSE_KIND 1
#define FLASH_KIND 2
#define EEPROM_KIND 3

// Save variables
#define SAVE_AUTOVERIFY 0
#define SAVE_AUTOERASE 1
#define SAVE_SPEED_INDEX 2
#define SAVE_SAFEEDIT 3

// timer
#define BUSY_MS 200U // longest time we consider us busy 


#define NO_BTN 255


uint8_t state;
uint8_t error;
const char * kindExt[] = { ".LCK", ".FUS", ".HEX", ".EEP" };
const char * kindStr[] = { "Lock", "Fuses", "Flash", "EEPROM" };
int16_t settings[] = { 0, -1, -1, -1, 0};
boolean autoverify, autoerase, safeedit;
int16_t speedopt[] = { -2, -2, -2, -2,  -2, -2, -2, 0};
uint16_t spidelayvals[] = { SPI_SPEED0, SPI_SPEED1, SPI_SPEED2, SPI_SPEED3, SPI_SPEED4, SPI_SPEED5, SPI_SPEED6 };
uint8_t speed_index;
uint16_t spidelay;
uint16_t progspidelay;
uint16_t mcusig;
boolean progmode = false;
int16_t mmpos, setpos, rwvpos, fusepos;
char buf[MAX_STRING_LENGTH+1];
uint8_t pagebuf[MAX_PAGE_SIZE];
boolean verified;
int32_t verifyaddr;
uint8_t verifyexpected, verifyseen;
const char * verifyaddrStr[] = { "", "Low fuse", "High fuse", "Ext. Fuse", "Lock bits" };
const SaveDefault savefileDefaults[] = {
  { SAVE_AUTOVERIFY, SAVETYPE_INT, 1, 0 },
  { SAVE_AUTOERASE, SAVETYPE_INT, 1, 0 },
  { SAVE_SPEED_INDEX, SAVETYPE_INT, 3, 0 },
  { SAVE_SAFEEDIT, SAVETYPE_INT, 1, 0 },
};

#ifdef DEBUG
#define DEBPR(str) SerialUSB.print(str)
#define DEBPRF(num,len) SerialUSB.print(num,len)
#define DEBWR(c)   SerialUSB.write(c)
#define DEBLN(str) SerialUSB.println(str)
#define DEBLNF(num,len) SerialUSB.println(num,len)
#else
#define DEBPR(str)
#define DEBPRF(num,len)
#define DEBWR(c) 
#define DEBLN(str)
#define DEBLNF(num,len) 
#endif

void setup() {
  gb.begin();
#ifdef SOUNDOFF
  gb.sound.mute();
#endif
  gb.save.config(savefileDefaults);
  autoverify = gb.save.get(SAVE_AUTOVERIFY);
  if (autoverify) settings[ST_AUTOVERIFY] = 1;
  autoerase = gb.save.get(SAVE_AUTOERASE);
  if (autoerase) settings[ST_AUTOERASE] = 1;
  safeedit = gb.save.get(SAVE_SAFEEDIT);
  if (safeedit) settings[ST_SAFEEDIT] = 1;
  speed_index = gb.save.get(SAVE_SPEED_INDEX);
  speedopt[speed_index] = 2;
  progspidelay = spidelayvals[speed_index];
#ifdef DEBUG
  SerialUSB.begin(115200);
  while (!SerialUSB);
  SerialUSB.println("AvrBurn V" VERSION " starts...");
#endif
  gb.display.setTextWrap(true);
  start_burner();
}

void start_burner()
{
  uint32_t start = millis();
  while (true) {
    while (!gb.update());
    display_header(BROWN, "Initializing ...");
    if (millis() - start > 800) break;
  }
  state = MENU_STATE;
  error = NO_ERROR;
  mmpos = 0;
  setpos = 0;
  set_prog_mode(false);
}

		    
void loop() {
  if (gb.update()) {
    if (gb.buttons.pressed(BUTTON_MENU)) state = RESET_STATE;
    if (error && state != RESET_STATE) state = ERROR_STATE;
    DEBPR("loop: state=");
    DEBLN(state);
    switch (state) {
    case RESET_STATE: start_burner(); break;
    case MENU_STATE: main_menu(); break;
    case INFO_STATE: display_info(); break;
    case SETTINGS_STATE: settings_menu(); break;
    case SPEED_STATE: speed_menu(); break;
    case DETECT_STATE: mcu_detect(); break;
    case ERASE_STATE: erase(true); break;
    case LOCK_STATE: fuse_menu(LOCKBITS_KIND); break;
    case FLASH_STATE:rwv_menu(FLASH_KIND); break;
    case EEP_STATE:rwv_menu(EEPROM_KIND); break;
    case FUSE_STATE: fuse_menu(FUSE_KIND); break;
    case ERROR_STATE: display_error(error);  break;
    default: error = CONFUSION_ERROR; break;
    }
  }
}

void main_menu()
{
  int item = menu("AVR Burn Menu",mainmenu, NULL, NUMELS(mainmenu), mmpos);
  rwvpos = 0;
  fusepos = 0;
  if (item >= 0) mmpos = item;
  if (state == RESET_STATE) return;
  switch (item) {
  case -1: return;
  case MM_EXIT: state = RESET_STATE; break;
  case MM_SETTINGS: state = SETTINGS_STATE; break;
  case MM_INFO: state = INFO_STATE; break;
  case MM_DETECT: state = DETECT_STATE; break;
  case MM_ERASE: state = ERASE_STATE;  break;
  case MM_LOCK: state = LOCK_STATE; break;
  case MM_FUSE: state = FUSE_STATE; break;
  case MM_EEP: state = EEP_STATE; break;
  case MM_FLASH: state = FLASH_STATE; break;
  default: error = NYI_ERROR; break;
  }
  DEBPR("main_menu exit: state=");
  DEBLN(state);
}


void settings_menu()
{
  setpos = 0;
  int16_t item = menu("Settings Menu",settingsmenu, settings, NUMELS(settingsmenu), setpos);
  if (state != RESET_STATE) {
    switch (item) {
    case -1: 
    case ST_EXIT: state = MENU_STATE; break;
    case ST_SPISPEED: state = SPEED_STATE; setpos = item; break;
    default: error = NYI_ERROR; break;
    }
  }
  if (autoverify != (settings[ST_AUTOVERIFY] == 1)) {
    autoverify = (settings[ST_AUTOVERIFY] == 1);
    gb.save.set(SAVE_AUTOVERIFY, autoverify);
  }
  if (autoerase != (settings[ST_AUTOERASE] == 1)) {
    autoerase = (settings[ST_AUTOERASE] == 1);
    gb.save.set(SAVE_AUTOERASE, autoerase);
  }
  if (safeedit != (settings[ST_SAFEEDIT] == 1)) {
    safeedit = (settings[ST_SAFEEDIT] == 1);
    gb.save.set(SAVE_SAFEEDIT, safeedit);
  }
  
}

void speed_menu()
{
  menu("Speed Menu",speedmenu, speedopt, NUMELS(speedmenu), 0);
  for (uint8_t i = 0; i < NUMELS(speedopt); i++)
    if (speedopt[i] == 2) {
      if (speed_index != i) {
	progspidelay = spidelayvals[i];
	speed_index = i;
	gb.save.set(SAVE_SPEED_INDEX, i);
      }
    }
  if (state != RESET_STATE) state = SETTINGS_STATE;
}

void rwv_menu(uint8_t kind)
{
  int16_t item = menu(rvwtitle[kind],rwvmenu, NULL, NUMELS(rwvmenu), rwvpos);
  if (item >= 0) rwvpos = item;
  if (state == RESET_STATE) return;
  switch(item) {
  case RWV_PROG: program(kind); break;
  case RWV_READ: save(kind); break;
  case RWV_VERIFY: verify(kind); break;
  case -1:
  case RWV_EXIT: state = MENU_STATE; break;
  default: error = NYI_ERROR; break;
  }
}

void fuse_menu(uint8_t kind)
{
  DEBPR("fuse_menu state=");
  DEBLN(state);
  int16_t item = menu(rvwtitle[kind],(kind == FUSE_KIND ? fusemenu : lockmenu), NULL, NUMELS(fusemenu), fusepos);
  if (item >= 0) fusepos = item;
  if (state == RESET_STATE) return;
  DEBPR("fuse_menu (before switch) state=");
  DEBLN(state);
  switch(item) {
  case FUSE_PROG: program(kind); break;
  case FUSE_VERIFY: verify(kind); break;
  case FUSE_SHOW: show_fuses(kind);
    DEBPR("fuse_menu (after show) state=");
    DEBLN(state);
    break;
  case FUSE_READ: save(kind); break;
  case FUSE_DEFAULT:
    if (kind == FUSE_KIND) set_default_fuses();
    else erase(false);
    break;
  case FUSE_EDIT: fuse_lock_edit(kind); break;
  case -1:
  case FUSE_EXIT: state = MENU_STATE; break;
  default: error = NYI_ERROR; break;
  }
  DEBPR("fuse_menu (after switch) state=");
  DEBLN(state);
}

void display_header(Color color, const char * title)
{
  gb.display.clear();
  gb.display.setTextWrap(false);
  gb.display.setColor(DARKGRAY);
  gb.display.fillRect(0, 0, gb.display.width(), 7);
  gb.display.setColor(color);
  gb.display.setCursor(1, 1);
  gb.display.println(title);
  gb.display.setColor(BLACK);
  gb.display.drawFastHLine(0, 7, gb.display.width());
  gb.display.println();
  gb.display.setColor(WHITE);
}

void display_error(uint8_t errnum)
{
  while (1) {
    while(!gb.update());
    if (check_OK()) {
      error = NO_ERROR;
      state = MENU_STATE;
      return;
    }
    display_header(RED, "ERROR");
    switch (errnum) {
    case NYI_ERROR:
      gb.display.println("This function");
      gb.display.println("is not yet");
      gb.display.println("implemented!");
      break;
    case SD_ERROR:
      gb.display.println("SD-Card");
      gb.display.println("not accessible");
      break;
    case CONFUSION_ERROR:
      gb.display.println("Internal");
      gb.display.println("Confusion");
      break;
    case SIG_ERROR:
      gb.display.println("MCU signature");
      gb.display.println("unreadable");
      break;
    case UNKNOWN_SIG_ERROR:
      gb.display.println("Unknown MCU");
      gb.display.print("signature:");
      gb.display.println(mcusig,HEX);
      break;
    case NO_MCU_ERROR:
      gb.display.println("No MCU detected!");
      break;      
    case NO_BURN_ERROR:
      gb.display.println("SD-Card does");
      gb.display.println("not contain");
      gb.display.println("BURN folder");
      break;
    case NO_MEM_ERROR:
      gb.display.println("Not enough");
      gb.display.println("RAM!");
      break;
    case FILE_DIR_ERROR:
      gb.display.println("File is a");
      gb.display.println("directory!");
      break;
    case FILE_OPEN_ERROR:
      gb.display.println("Cannot open");
      gb.display.println("file!");
      break;
    case HEX_FILE_ERROR:
      gb.display.println("Format error");
      gb.display.println("in HEX file!");
      break;
    case REMOVE_ERROR:
      gb.display.println("Unable to remove");
      gb.display.println("file!");
      break;
    case FILE_WRITE_ERROR:
      gb.display.println("Error when writing");
      gb.display.println("to file!");
      break;
    case HEX_ADDR_ERROR:
      gb.display.println("Address error in");
      gb.display.println("HEX file!");
      break;      
    case HEX_CRC_ERROR:
      gb.display.println("Checksum error in");
      gb.display.println("HEX file!");
      break;
    case HEX_RECTYPE_ERROR:
      gb.display.println("Unknown record type");
      gb.display.println("in HEX file!");
      break;
    case FLASH_PROG_ERROR:
      gb.display.println("Error while");
      gb.display.println("programming FLASH.");
      break;
    case EEPROM_PROG_ERROR:
      gb.display.println("Error while");
      gb.display.println("programming EEPROM.");
      break;
    case MAX_ADDR_ERROR:
      gb.display.println("Trying to program");
      gb.display.println("or verify a memory");
      gb.display.println("cell with addr out");
      gb.display.println("of bound.");
      break;            
    case NO_FUSE_INFO_ERROR:
      gb.display.println("For this MCU there");
      gb.display.println("is nothing known");
      gb.display.println("about the fuse");
      gb.display.println("definitions.");
      break;            
    default:
      gb.display.println("Unknown");
      gb.display.print("Error: ");
      gb.display.println(errnum);
      break;
    }
    display_OK();
  }
}

void display_OK()
{
  gb.display.setColor(DARKGRAY);
  gb.display.fillRect(gb.display.width()/2-gb.display.fontWidth-2,gb.display.height()-gb.display.fontHeight-2,2*gb.display.fontWidth+4,gb.display.fontHeight+2);
  gb.display.setColor(RED);
  gb.display.setCursor(gb.display.width()/2-gb.display.fontWidth,gb.display.height()-gb.display.fontHeight);
  gb.display.print("OK");
  gb.display.setColor(WHITE);    
}

boolean check_RST()
{
  if (gb.buttons.released(BUTTON_MENU)) {
    gb.sound.playCancel();
    state = RESET_STATE;
    return true;
  }
  return false;
}


boolean check_OK()
{
  if (gb.buttons.released(BUTTON_A)) {
    error = NO_ERROR;
    gb.sound.playOK();
    return true;
  }
  if (check_RST()) {
    state = RESET_STATE;
    return true;
  }
  return false;
}


void display_info()
{
  while (1) {
    while(!gb.update());
    if (check_OK()) break;
    display_header(YELLOW, "Info");
    gb.display.println("Version " VERSION);
    gb.display.print("Free RAM: ");
    gb.display.println(gb.getFreeRam());
    gb.display.print("SPI freq.  =");
    gb.display.println(speedmenu[speed_index]);
    gb.display.print("Auto verify=");
    gb.display.println((autoverify ? "on" : "off"));
    gb.display.print("Auto erase =");
    gb.display.println((autoerase ? "on" : "off"));
    gb.display.print("Safe edit  =");
    gb.display.println((safeedit ? "on" : "off"));
    display_OK();
  }
  if (state != RESET_STATE) state = MENU_STATE;  
}




void mcu_detect()
{
  uint16_t mix;
  DEBLN("mcu_detect");
  set_prog_mode(true);
  mcusig = read_sig();
  set_prog_mode(false);
  DEBPR("sig=");
  DEBLNF(mcusig,HEX);
  mix = mcu_ix(mcusig);
  if (error) return;
  while (1) {
    while(!gb.update());
    if (check_OK()) break;
    display_header(YELLOW, "Detected MCU");
    gb.display.print("MCU: ");
    gb.display.println(mcuList[mix].name);
    display_OK();
  }
  if (state != RESET_STATE) state = MENU_STATE;
}

uint16_t mcu_ix(uint16_t sig)
{
  DEBPR("mcu_ix:");
  DEBLNF(sig,HEX);
  for (int i=0; i < NUMELS(mcuList); i++) {
    if (mcuList[i].signature  == sig) {
      return i;
    }
  }
  error = UNKNOWN_SIG_ERROR;
  return 0;
}

void erase(boolean toMainMenu)
{
  uint16_t mix;
  boolean yes = left_or_right("Chip Erase", "Do you really want\nto erase everything?", "Yes", "No", false);
  if (yes) { 
    display_header(YELLOW, "Chip Erase");
    gb.display.println("Erasing ...");
    set_prog_mode(true);
    mix = mcu_ix(read_sig());
    if (mix == 0) error = NO_MCU_ERROR;
    if (error) {
      set_prog_mode(false);
      return;
    }
    while (!gb.update());
    erase_chip(mix);
    set_prog_mode(false);
    while (1) {
      while(!gb.update());
      if (check_OK()) break;
      display_header(YELLOW, "Chip Erase");
      gb.display.println("Erasing ...");
      gb.display.println("Done.");
      display_OK();
    }
  }
  if (state != RESET_STATE && toMainMenu) state = MENU_STATE;
}
    
void program(uint8_t kind)
{
  char path[MAX_STRING_LENGTH+6] = "/BURN/";
  char * filename = choose_file(false,kind);
  DEBLN("program");
  if (!filename) return;
  strcat(path,filename);
  File file = SD.open(path);
  if (!file) {
    error = FILE_OPEN_ERROR;
    return;
  }
  if (file.isDirectory()) {
    error = FILE_DIR_ERROR;
    return;
  }
  if (read_file(false, filename, file, kind) && autoverify)
    read_file(true, filename, file, kind);
  if (file) file.close();
}

void verify(uint8_t kind)
{
  char path[MAX_STRING_LENGTH+6] = "/BURN/";
  char * filename = choose_file(false,kind);
  DEBLN("program");
  if (!filename) return;
  strcat(path,filename);
  File file = SD.open(path);
  if (!file) {
    error = FILE_OPEN_ERROR;
    return;
  }
  if (file.isDirectory()) {
    error = FILE_DIR_ERROR;
    return;
  }
  read_file(true, filename, file, kind);
  if (file) file.close();
}

void show_fuses(uint8_t kind)
{
  uint32_t fuses;
  set_prog_mode(true);
  uint16_t mcuix = mcu_ix(mcusig = read_sig());
  set_prog_mode(false);
  if (mcuix == 0) error = NO_MCU_ERROR;
  if (error) return;
  display_header(YELLOW, (kind == FUSE_KIND ? "Show Fuses" : "Show Lock Bits"));
  while (!gb.update());
  set_prog_mode(true);
  switch (kind) {
  case FUSE_KIND:
    fuses = read_fuses();
    break;
  case LOCKBITS_KIND:
    fuses = read_lock();
    break;
  default: error = CONFUSION_ERROR;
    break;
  }
  set_prog_mode(false);
  if (error) return;
  while (true) {
    while (!gb.update());
    display_header(YELLOW, (kind == FUSE_KIND ? "Show Fuses" : "Show Lock Bits"));
    switch (kind) {
    case FUSE_KIND:
      if (mcuList[mcuix].fuses == 0) {
	gb.display.println("No fuses!");
	break;
      }
      if (mcuList[mcuix].fuses >= 1) {
	gb.display.print("Low  fuse=0x");
	gb.display.println(fuses & 0xFF, HEX);
      }
      if (mcuList[mcuix].fuses >= 2) {
	gb.display.print("High fuse=0x");
	gb.display.println((fuses >> 8) & 0xFF, HEX);
      }
      if (mcuList[mcuix].fuses >= 3) {
	gb.display.print("Ext. fuse=0x");
	gb.display.println((fuses >> 16) & 0xFF, HEX);
      }
      break;
    case LOCKBITS_KIND:
      gb.display.print("Lockbits=0x");
      gb.display.println(fuses & 0xFF,HEX);
      break;
    }
    display_OK();
    if (check_OK()) return;
  }
}


boolean read_file(boolean doverify, char * filename, File & file, uint8_t kind)
{
  char title[MAX_STRING_LENGTH];
  boolean done = false;
  uint8_t percentage = 0;
  boolean verified = true;
  uint32_t size, pageaddr = 0, lineaddr = 0, offset = 0;
  uint16_t pagesize = 0;
  uint8_t linebuf[256];
  uint16_t linefill = 0, lix = 0;
  boolean eofreached = false;
  uint8_t polling = 0;
  uint8_t wait_ms = 0;

  DEBLN("read_file");
  set_prog_mode(true);
  uint16_t mix = mcu_ix(mcusig = read_sig());
  if (kind == FLASH_KIND && autoerase && !doverify) erase_chip(mix);
  if (!error & mix == 0) error = NO_MCU_ERROR;
  if (error) {
    if (file) file.close();
    set_prog_mode(false);
    return false;
  }
  strcpy(title, (doverify ? "Verifying " : " Programming "));
  strcat(title, kindStr[kind]);
  file.seek(0);
  pagesize = determine_pagesize(kind, mix);
  polling = determine_polling(kind, mix);
  wait_ms = determine_wait(kind, mix);
  size = determine_size(kind, mix);
  while (true) {
    while (!gb.update());
    display_header(YELLOW, title);
    gb.display.print("File:");
    gb.display.println(filename);
    display_progress(percentage);
    if (check_RST()) {
      file.close();
      set_prog_mode(false);
      return false;
    }
    if (done) {
      if (autoverify && !doverify) {
	set_prog_mode(false);
	return true;
      }
      gb.display.setCursor(0,20);
      gb.display.setColor(WHITE,BLACK);
      if (verified) 
	gb.display.print((doverify ? "Verified!" : "Programmed!"));
      else 
	show_failed_verification();
      display_OK();
      if (check_OK()) break;
    }
    if (percentage == 0) while (!gb.update());
    while (!done && percent_read(file) - percentage < 1) {
      while (pageaddr < lineaddr) pageaddr += pagesize;
      done = fill_next_page(file, kind, eofreached, pagesize, pageaddr, offset, lineaddr, linefill, lix, linebuf);
      DEBLN("readfile next page loop");
      if (!done) {
	if (pageaddr + pagesize > size) error = MAX_ADDR_ERROR;
	else if (doverify) verify_page(file, kind, pagesize, pageaddr, verified);
	else prog_page(file, kind, pagesize, pageaddr, polling, wait_ms);
      if (!verified || error) done = true;
      }
      pageaddr += pagesize;
    }
    percentage = percent_read(file);
    // DEBLN("read_file update loop");
    if (done) percentage = 100;
    if (error) break;
  }
  set_prog_mode(false);
  if (file) file.close();
  return false;
}
  
uint32_t determine_size(uint8_t kind, uint16_t mcuix)
{
  switch (kind) {
  case LOCKBITS_KIND:
    return 1;
  case FUSE_KIND:
    return mcuList[mcuix].fuses;
  case FLASH_KIND:
    return mcuList[mcuix].flashSize;
  case EEPROM_KIND:
    return mcuList[mcuix].eepromSize;
  default:
    error = CONFUSION_ERROR;
    return 1;
  }
}

uint16_t determine_pagesize(uint8_t kind, uint16_t mcuix)
{
  switch (kind) {
  case LOCKBITS_KIND:
    return 1;
  case FUSE_KIND:
    return mcuList[mcuix].fuses;
  case FLASH_KIND:
    if (mcuList[mcuix].flashMode == 0x4C)
      return mcuList[mcuix].flashPS;
    else
      return 1; // only byte programmable, no pages!
  case EEPROM_KIND:
    if (mcuList[mcuix].flashMode == 0xC2)
      return mcuList[mcuix].eepromPS;
    else
      return 1; // only byte programmable
  default:
    error = CONFUSION_ERROR;
    return 1;
  }
}

uint8_t determine_polling(uint8_t kind, uint16_t mcuix)
{
  if (kind == FLASH_KIND) return mcuList[mcuix].flashPoll;
  else if  (kind == EEPROM_KIND) return mcuList[mcuix].eepromPoll;
  else return 0;
}

uint8_t determine_wait(uint8_t kind, uint16_t mcuix)
{
  if (kind == FLASH_KIND) return mcuList[mcuix].flashDelay;
  else if  (kind == EEPROM_KIND) return mcuList[mcuix].eepromDelay;
  else return 0;
}


void show_failed_verification()
{
  gb.display.print("Deviation:");
  if (verifyaddr < 0) gb.display.println(verifyaddrStr[-verifyaddr]);
  else {
    gb.display.print("0x");
    gb.display.println(verifyaddr,HEX);
  }
  gb.display.print("Expected: 0x");
  gb.display.println(verifyexpected,HEX);
  gb.display.print("Seen:     0x");
  gb.display.println(verifyseen,HEX);
}

uint32_t percent_read(File & file)
{
  return ((100UL*file.position())/file.size());
}

void display_progress(uint8_t p) {
  gb.display.setColor(RED);
  gb.display.drawRect(3,40,74,9);
  gb.display.fillRect(3,40,(int)(74*p/100),9);
  gb.display.setCursor(36,42);
  gb.display.setColor(WHITE);
  gb.display.print(p);
  gb.display.print("%");
  gb.display.setCursor(0,50);  
}



void prog_page(File & file, uint8_t kind, uint16_t pagesize, uint32_t pageaddr, uint8_t polling, uint8_t wait_ms) {
  DEBLN("prog_page");
  switch (kind) {
  case LOCKBITS_KIND:
    program_lock(pagebuf[0]);
    break; 
  case FUSE_KIND:
    program_fuses(pagesize, pagebuf[0], pagebuf[1], pagebuf[2]);
    break;
  case FLASH_KIND:
    program_flash(pageaddr, pagesize, polling, wait_ms);
    break;
  case EEPROM_KIND:
    program_eeprom(pageaddr, pagesize, polling, wait_ms);
    break;
  default: error = CONFUSION_ERROR;
    break;
  }
}

void verify_page(File & file, uint8_t kind,  uint16_t pagesize, uint32_t pageaddr, boolean & verified) {
  DEBLN("verify_page");
  DEBPR("  pageaddr=");
  DEBLNF(pageaddr,HEX);
  DEBPR("  pagesize=");
  DEBLNF(pagesize,HEX);
  switch (kind) {
  case LOCKBITS_KIND:
    verified = verify_lock(pagebuf[0]);
    break;
  case FUSE_KIND:
    verified = verify_fuses(pagesize, pagebuf[0], pagebuf[1], pagebuf[2]);
    break;
  case FLASH_KIND:
    verified = verify_flash(pageaddr, pagesize);
    break;
  case EEPROM_KIND:
    verified = verify_eeprom(pageaddr, pagesize);
    break;
  default: error = CONFUSION_ERROR; break;
  }
}


boolean fill_next_page(File & file,  uint8_t kind, boolean & eofreached, uint16_t pagesize, uint32_t & pageaddr, uint32_t & offset, uint32_t & lineaddr, uint16_t & linefill, uint16_t & lix, uint8_t linebuf[])
{
  uint16_t fill = 0;
  DEBLN("fill_next_page");
  if (eofreached) return true;
  if (kind == LOCKBITS_KIND || kind == FUSE_KIND) {
    for (uint8_t i=0; i < pagesize; i++) {
      pagebuf[i] = read_hex_byte(file);
      if (i != pagesize-1) skip_eol(file);
    }
    eofreached = true;
    return false;
  } else {
  // flash and eeprom
    for (uint16_t i=0; i < pagesize; i++) pagebuf[i] = 0xFF;
    do {
      if (lix == linefill) {
	fill_next_line(file, kind, eofreached, offset, lineaddr, linefill, linebuf);
	lix = 0;
	if (eofreached) return (fill == 0);
      }
      if (lineaddr+lix < pageaddr) {
	DEBLN("HEX ADDR ERROR");
	error = HEX_ADDR_ERROR;
	return true;
      }
      if (error || eofreached) return false;
      DEBLN("fill_next_page before fill loop");
      DEBPR("  lineaddr=");
      DEBLNF(lineaddr,HEX);
      DEBPR("       lix=");
      DEBLN(lix);
      DEBPR("  pageaddr=");
      DEBLNF(pageaddr,HEX);
      DEBPR("  pagesize=");
      DEBLN(pagesize);
      while (lineaddr + lix < pageaddr + pagesize && lix < linefill) {
	pagebuf[lineaddr - pageaddr + lix] = linebuf[lix++];
	fill++;
      }
    } while (lineaddr + lix < pageaddr + pagesize);
    return false;
  }
}

void fill_next_line(File & file, uint8_t kind, boolean & eofreached, uint32_t & offset, uint32_t & lineaddr, uint16_t & linefill, uint8_t linebuf[])
{
  uint8_t rectype;
  uint16_t recaddr;
  do {
    rectype = read_record(file, recaddr, linefill, linebuf);
    if (!error && rectype > 3) error = HEX_RECTYPE_ERROR;
    if (error) return;
    eofreached = (rectype == 1);
    if (rectype == 2) offset = (linebuf[0] << 12) + (linebuf[1] << 4);
  } while (rectype == 2 || rectype == 3);
  lineaddr = offset + recaddr;
}

uint8_t read_record(File & file, uint16_t & recaddr, uint16_t & linefill, uint8_t linebuf[])
{
  uint8_t b1, b2;
  if (file.read() != ':') error = HEX_FILE_ERROR;
  linefill = read_hex_byte(file);
  uint8_t crc = linefill;
  b1 = read_hex_byte(file);
  crc += b1;
  b2 += read_hex_byte(file);
  crc += b2;
  recaddr = (b1<<8) + b2;
  uint8_t rectype = read_hex_byte(file);
  crc += rectype;
  for (uint8_t i=0; i< linefill; i++) {
    linebuf[i] = read_hex_byte(file);
    crc += linebuf[i];
  }
  crc += read_hex_byte(file);
  if (!error && crc != 0) error = HEX_CRC_ERROR;
  skip_eol(file);
#ifdef DEBUG
  DEBLN("read_record:");
  DEBPR("  type=");
  DEBLN(rectype);
  DEBPR("  len =");
  DEBLN(linefill);
  DEBPR("  addr=");
  DEBLNF(recaddr,HEX);
  DEBPR("  line=");
  for (uint16_t j=0; j < linefill; j++) {
    DEBPRF(linebuf[j],HEX);
    DEBPR(" ");
  }
  DEBLN("");
#endif
  return rectype;
}

		 

uint8_t read_hex_byte(File & file)
{
  uint8_t b = 0;
  b = hex_to_num(file, file.read());
  b <<= 4;
  b |= hex_to_num(file, file.read());
  //DEBPR("read_hex_byte: ");
  //DEBLNF(b,HEX);
  return b;
}

uint8_t hex_to_num(File & file, char c)
{
  if (c >= '0' && c <= '9') 
    return (c - '0');
  else if (toupper(c) >= 'A' && toupper(c) <= 'F')
    return (toupper(c) -'A' + 10);
  else
    error = HEX_FILE_ERROR;
  return 0;
}

boolean skip_eol(File & file)
{
  if (file.peek() != '\n' && file.peek() != '\r') {
    if (!error) error = HEX_FILE_ERROR;
    return false;
  }
  while (file.peek() == 0x0A || file.peek() == 0x0D)
    file.read();
  return true;
}

void save(uint8_t kind)
{
  DEBLN("save");
  char path[MAX_STRING_LENGTH+6] = "/BURN/";
  char * filename = typein_filename(kind);
  if (!filename) return;
  strcat(path,filename);
  if (SD.exists(path)) {
    if (!overwrite_ask(filename)) return;
    if (!SD.remove(path)) {
      error = REMOVE_ERROR;
      return;
    }
  }
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    error = FILE_OPEN_ERROR;
    return;
  }
  save_file(filename, file, kind);
  if (file) file.close();
}

boolean overwrite_ask(char * filename)
{
  char question[50] = "File already exists:\n";
  question[strlen(question)+20] = '\0';
  strncat(question,filename,20);
  
  boolean reply = left_or_right("File exists", question , "Overwrite", "Abort", false);
  if (state == RESET_STATE) return false;
  return (reply);
}

char * typein_filename(uint8_t kind)
{
  char title[20] = "Save ";
  strcat(title, kindStr[kind]);
  strcat(title, " File");
  buf[0] = '\0';
  keyboard(title, buf, 20);
  if (!buf[0]) return NULL;
  char * dot = strchr(buf,'.');
  if (dot) *dot = '\0';
  strcat(buf,kindExt[kind]);
  return buf;
}

void save_file(char * filename, File & file, uint8_t kind)
{
  char title[MAX_STRING_LENGTH] = "Saving ";
  boolean done = false;
  uint8_t percentage = 0;
  uint32_t startaddr = 1, endaddr = 1, curraddr, lineaddr, offsetaddr = 0, offset = 0;
  uint8_t linebuf[16];
  uint16_t linefill = 0;
  uint32_t fusebytes;

  set_prog_mode(true);
  uint16_t mix = mcu_ix(mcusig = read_sig());
  if (!error & mix == 0) error = NO_MCU_ERROR;
  if (error) {
    if (file) file.close();
    set_prog_mode(false);
    return;
  }
  strcat(title, kindStr[kind]);
  strcat(title, " File");
  DEBLN("save_file");
  switch (kind) {
  case LOCKBITS_KIND:
    pagebuf[1] = read_lock();
    break;
  case FUSE_KIND:
    fusebytes = read_fuses();
    pagebuf[1] = fusebytes & 0xFF;
    pagebuf[2] = (fusebytes >> 8) & 0xFF;
    pagebuf[3] = (fusebytes >> 16) & 0xFF;
    endaddr = mcuList[mix].fuses;
    break;
  case EEPROM_KIND:
    startaddr = 0;
    DEBPR("mix=");
    DEBLNF(mix,HEX);
    endaddr = mcuList[mix].eepromSize-1;
    DEBPR("endaddr=");
    DEBLNF(endaddr,HEX);
    break;
  case FLASH_KIND:
    startaddr = 0;
    endaddr = mcuList[mix].flashSize-1;
    break;
  }
  if (error) {
    file.close();
    set_prog_mode(false);
    return;
  }
  curraddr = startaddr;
  lineaddr = startaddr;
  percentage = 0;
  DEBLN("save_file: start loop");
  spidelay = progspidelay;
  while (true) {
    while (!gb.update());
    display_header(YELLOW, title);
    gb.display.print("File:");
    gb.display.println(filename);
    display_progress(percentage);
    if (check_RST()) {
      if (file) file.close();
      return;
    }
    if (done) {
      gb.display.setColor(WHITE, BLACK);
      gb.display.setCursor(0,20);
      gb.display.println("File saved!");
      display_OK();
      if (check_OK()) {
	return;
      }
    }
    if (percentage == 0) while (!gb.update());
    while (curraddr <= endaddr && (100 - (100UL*(endaddr+1-curraddr)/(endaddr+1-startaddr)) - percentage < 1)) {
      save_byte(file, kind, offset, curraddr, lineaddr, linefill, linebuf);
    }
    percentage = (100 - (100UL*(endaddr+1-curraddr)/(endaddr+1-startaddr)));
    if (!done && curraddr > endaddr) {
      done = true;
      percentage = 100;
      save_finish(file, kind, offset, lineaddr, linefill, linebuf);
      if (kind == FLASH_KIND || kind == EEPROM_KIND) set_prog_mode(false);
    }
    if (error) {
      if (file) file.close();
      set_prog_mode(false);
      return;
    }
  }
}

void save_byte(File & file, uint8_t kind, uint32_t & offset, uint32_t & curraddr, uint32_t & lineaddr, uint16_t & linefill, uint8_t linebuf[])
{
  uint8_t data;
  switch (kind) {
  case FUSE_KIND:
  case LOCKBITS_KIND:
    write_hex_byte(file, pagebuf[curraddr++]);
    file.println();
    return;
  case EEPROM_KIND:
    data = read_eeprom_byte(curraddr);
    break;
  case FLASH_KIND:
    data = read_flash_byte(curraddr);
    break;
  }
  if (linefill == MAX_LINE_BYTES) {
    save_data_record(file, kind, offset, lineaddr, linefill, linebuf);
    linefill = 0;
    lineaddr = curraddr;
  }
  linebuf[linefill++] = data;
  curraddr++;
}

void save_data_record(File & file, uint8_t kind, uint32_t & offset, uint32_t lineaddr, uint16_t linefill, uint8_t  linebuf[])
{
  uint8_t crc;
  DEBPR("save_data_record: lineaddr=");
  DEBPRF(lineaddr,HEX);
  DEBPR(", offset=");
  DEBPRF(offset,HEX);
  DEBPR(", cond=");
  DEBLN(lineaddr - offset > 0x10000UL);
  if (lineaddr - offset > 0x10000UL) { // we need to write an extended segment address record
    offset += 0x10000UL; // new offset
    file.print(":02000002"); // 2 data bytes, base addr is 0, record type 2
    file.print((offset>>4),HEX); // the data part is offset/16
    crc = ~(0x02 + 0x02 + (offset >> 12)) + 1; // compute CRC
    write_hex_byte(file, crc & 0xFF); // CRC
    file.println();
  }
  boolean empty = true;
  for (uint8_t i = 0; i < linefill; i++) 
    if (linebuf[i] != 0xFF) empty = false;
  if (kind == EEPROM_KIND) empty = false;
  if (!empty) { // if different from all 0xFF
    file.print(":");
    DEBPR(":");
    write_hex_byte(file, linefill); // record length
    crc = linefill;
    write_hex_word(file, lineaddr-offset); // address
    crc += ((lineaddr-offset) & 0xFF);
    crc += (((lineaddr-offset)>>8) & 0xFF);
    write_hex_byte(file, 0); // type = data record
    for (uint8_t i = 0; i < linefill; i++) {
      crc += linebuf[i];
      write_hex_byte(file, linebuf[i]);
    }
    crc = (~crc + 1) & 0xFF;
    write_hex_byte(file, crc);
    file.println();
    DEBLN("");
  }
}

void save_finish(File & file, uint8_t kind, uint32_t & offset, uint32_t lineaddr, uint16_t linefill, uint8_t linebuf[])
{
  if (kind == FLASH_KIND || kind == EEPROM_KIND) {
    if (linefill) save_data_record(file, kind, offset, lineaddr, linefill, linebuf);
    file.println(":00000001FF"); // EOF record
    DEBLN(":00000001FF");
  }
  file.close();
  return;
}

void write_hex_byte(File & file, uint8_t data)
{
  DEBPRF((data >> 4) & 0xF, HEX);
  DEBPRF((data) & 0xF, HEX);
  if (!file.print((data >> 4) & 0xF, HEX) || !file.print((data) & 0xF, HEX)) 
    error = FILE_WRITE_ERROR;
  return;
}

void write_hex_word(File & file, uint16_t word)
{
  write_hex_byte(file, (word >> 8) & 0xFF);
  write_hex_byte(file, word  & 0xFF);
}

void set_default_fuses() {
  set_prog_mode(true);
  uint16_t mcuix = mcu_ix(mcusig = read_sig());
  set_prog_mode(false);
  if (mcuix == 0) error = NO_MCU_ERROR;
  if (error) return;
  set_fuses("Default fuses", FUSE_KIND, mcuList[mcuix].fuses,mcuList[mcuix].lowFuse,mcuList[mcuix].highFuse,mcuList[mcuix].extendedFuse, 0);
}

void set_fuses(const char fusetitle[], uint8_t kind, uint8_t fusenum, uint8_t low, uint8_t high, uint8_t ext, uint8_t lock)
{
  boolean verified = true;
  display_header(YELLOW, fusetitle);
  while (!gb.update());
  set_prog_mode(true);
  if (kind == FUSE_KIND) 
    program_fuses(fusenum, low, high, ext);
  else
    program_lock(lock);
  set_prog_mode(false);
  set_prog_mode(true);
  if (!error) { 
    if (autoverify) {
      if (kind == FUSE_KIND)
	verified = verify_fuses(fusenum, low, high, ext);
      else
	verified = verify_lock(lock);
    }
    while (true) {
      if (gb.update()) {
	display_header(YELLOW, fusetitle);
	gb.display.println("Programmed!");
	if (autoverify) {
	  if (verified) {
	    gb.display.println("Verified!");
	  } else {
	    gb.display.println("Verification failed!");
	    show_failed_verification();
	  }
	}
	display_OK();
	if (check_OK()) {
	  break;
	}
      }
    }
  }
  set_prog_mode(false);
}

char * choose_file(boolean verify, uint8_t kind)
{
  File dir;
  int16_t item;
  int16_t entrycnt = 0;
  char ** dlist;
  char title[20];
  char strNoFile[] = "-- No File --";
  const char * strVerify = "Verify ";
  const char * strRead = "Program ";
  DEBLN("choose_file");
  if (error == NO_ERROR) {
    dir = SD.open("/BURN");
    if (!dir || !(dir.isDirectory())) {
      error = NO_BURN_ERROR;
      return NULL;
    }
  }
  dlist = read_dir(dir,kind);
  dir.close();
  if (!dlist) return NULL;
  entrycnt = 0;
  while (dlist[entrycnt++]);
  entrycnt--;
  if (verify)
    strcpy(title, strVerify);
  else
    strcpy(title, strRead);
  strcat(title, kindStr[kind]);
  DEBPR("Entries=");
  DEBLN(entrycnt);
  if (entrycnt == 0) {
    dlist[0] = strNoFile;
    menu(title, dlist, NULL, 1, 0);
    dlist[0] = NULL;
    deallocate(dlist);
    return NULL;
  }
  item = menu(title, dlist, NULL, entrycnt , 0);
  if (item >= 0) strcpy(buf, dlist[item]);
  deallocate(dlist);
  if (state == RESET_STATE || item < 0) 
    return NULL;
  return buf;
}

void deallocate(char ** list)
{
  int16_t ix = 0;
  if (!list) return;
  while (list[ix]) free(list[ix++]);
  free(list);
  return;
}

char ** read_dir(File & dir, uint8_t kind)
{
  int16_t listlen;
  int16_t ix;
  char ** dlist;
  DEBLN("read_dir");
  listlen = count_entries(dir, kind);
  DEBPR("entries=");
  DEBLN(listlen);
  dlist = (char **)malloc((listlen+1)*sizeof(char *));
  for (int16_t i=0; i< listlen+1; i++) dlist[i] = NULL;
  if (!dlist) {
    dir.rewindDirectory();
    error = NO_MEM_ERROR;
    return NULL;
  }
  ix = 0;
  do {
    File entry = dir.openNextFile();
    if (!entry) break;
    entry.getName(buf, MAX_STRING_LENGTH);
    buf[MAX_STRING_LENGTH] = '\0';
    entry.close();
    if (!check_kind(buf,kind)) continue;
    DEBLN(buf);
    dlist[ix] = (char *)malloc(strlen(buf)+1);
    if (!dlist[ix]) {
      dir.rewindDirectory();
      error = NO_MEM_ERROR;
      deallocate(dlist);
      return NULL;
    }
    strcpy(dlist[ix], buf);
    DEBLN(dlist[ix]);
    ix++;
  } while (true);
  dir.rewindDirectory();
  return dlist;
}

boolean check_kind(char * buf, uint8_t kind)
{
  return ((!stricmp(&buf[strlen(buf)-4],kindExt[kind])) && (buf[0] != '.'));
}

int16_t count_entries(File & dir, uint8_t kind)
{
  int16_t len = 0;
  do {
    File entry = dir.openNextFile();
    if (!entry) break;
    entry.getName(buf, MAX_STRING_LENGTH);
    buf[MAX_STRING_LENGTH] = '\0';
    entry.close();
    if (!check_kind(buf,kind)) continue;
    DEBLN(len);
    DEBLN(buf);
    len++;
  } while (true);
  dir.rewindDirectory();
  return len;
}  


void fuse_lock_edit(uint8_t kind)
{
  const char * mcuprefix = (kind == FUSE_KIND ? "Do you really want\nto write the fuses:\n"
			 : "Do you really want\nto write the lockbits:");
  const char * nomcuprefix = (kind == FUSE_KIND ? "Resulting fuses:\n"
			 : "Resulting lockbits:\n");
  char question[80];
  uint16_t fix = 0;
  boolean nomcu = false;
  int8_t reply;
  boolean verified;
  uint32_t fuses;
  uint8_t fusenum;
  set_prog_mode(true);
  uint16_t mcusig = read_sig();
  if (!error && mcusig) {
    if (kind == FUSE_KIND)
      fuses = read_fuses();
    else
      fuses = (read_lock() << 24);
    set_prog_mode(false);
  } else if (mcusig == 0) {
    set_prog_mode(false);
    nomcu = true;
    mcusig = choose_mcu();
    if (mcusig) {
      fuses = (mcuList[mcu_ix(mcusig)].lowFuse + (uint32_t)(mcuList[mcu_ix(mcusig)].highFuse<<8) +
	       (uint32_t)(mcuList[mcu_ix(mcusig)].extendedFuse<<16) + 0xFF000000UL);
    }
  } else {
    set_prog_mode(false);
    return;
  }
  if (mcusig == 0) return;
  fusenum = mcuList[mcu_ix(mcusig)].fuses;
  if (error) return;
  for (uint16_t i=0; i < NUMELS(fuseMenuList); i++) 
    if (fuseMenuList[i].sig == mcusig) fix = i;
  if (fix == 0) {
    error = NO_FUSE_INFO_ERROR;
    return;
  }
  do {
    DEBPR("fuse_edit before: ");
    DEBLNF(fuses,HEX);
    fuses = fuses_edit_menu((kind == FUSE_KIND ? "Edit fuses" : "Edit lockbits"), kind, fuseMenuList[fix].fuseList,  fuses);
    DEBPR("fuse_edit after: ");
    DEBLNF(fuses,HEX);
    if (nomcu)
      strcpy(question, nomcuprefix);
    else
      strcpy(question, mcuprefix);
    if (kind == FUSE_KIND) {
      if (fusenum > 0) {
	strcat(question, "L:0x");
	strcat(question, hex_byte_str(fuses & 0xFF));
      }
      if (fusenum > 1) {
	strcat(question, ",H:0x");
	strcat(question, hex_byte_str((fuses >> 8)& 0xFF));
      }
      if (fusenum > 2) {
	strcat(question, ",X:0x");
	strcat(question, hex_byte_str((fuses >> 16)& 0xFF));
      }
    } else {
      strcat(question, "LB: 0x");
      strcat(question, hex_byte_str((fuses >> 24)& 0xFF));
    }
    if (nomcu) {
      reply = (left_or_right((kind == FUSE_KIND ? "Inspect fuse values" : "Inspect lockbits"),
			     question, "Abort", "Back", true) ? 1 : -1);
    } else {
      reply = left_mid_or_right((kind == FUSE_KIND ? "Writing fuse values" : "Writing lockbits"), question, "Write", "Abort", "Back", 0);
    }
    if (reply == 0 || error) return;
  } while (reply != 1);
  if (nomcu) return;
  set_fuses(((kind == FUSE_KIND) ? "Write fuse values" : "Write lockbits"), kind, fusenum,
    (fuses & 0xFF),
    ((fuses>>8) & 0xFF),
    ((fuses>>16) & 0xFF),
    ((fuses>>24) & 0xFF));
}

char * hex_byte_str(uint8_t num)
{
  static char str[3];
  str[0] = conv_to_hexchar(num >> 4);
  str[1] = conv_to_hexchar(num&0xF);
  str[2] = '\0';
  return str;
}

char conv_to_hexchar(uint8_t dig)
{
  if ((dig&0xf) >= 0 && (dig&0xf) <= 9) {
    return (char)((dig&0xf)+'0');
  } else {
    return (char)((dig&0xf)+(uint8_t)'A'-10);
  }
}

uint16_t choose_mcu()
{
  uint16_t mcusig = 0;
  char * mcunames[NUMELS(mcuList)];

  if (!left_or_right("No MCU present", "Do you want to\ninspect fuses/lock\nbits of a MCUtype?", "Yes", "No", 1)) return 0;
  for (uint16_t i=0; i < NUMELS(mcuList); i++) 
    mcunames[i] = (char *)mcuList[i].name;
  int16_t result = menu("Choose MCU type", mcunames, NULL, NUMELS(mcuList), 0);
  if (result < 0) return 0;
  else return mcuList[result].signature;
}
