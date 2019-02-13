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
 * V0.3 (11.2.2019)
 *    - switch from Gamebuino Classic to Gamebuino META
 *    - File choose menu for different kinds 
 *    - SPI speed adjusted
 */

#define DEBUG

#define VERSION "0.3"

#include <Gamebuino-Meta.h>
#include <stdio.h>
#include <string.h>
#include "mcus.h"
#include "menus.h"

// number of els
#define NUMELS(x) (sizeof(x)/sizeof(x[0]))

// max string length
#define MAX_STRING_LENGTH 128

// max list length
#define MAX_LIST_LENGTH 128

// Pins
#define SOFT_RST 2
#define PORT_RST PORT_PA14
#define SOFT_MOSI 3
#define PORT_MOSI PORT_PA09
#define SOFT_MISO 5
#define PORT_MISO PORT_PA15
#define SOFT_SCK 4
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
#define NO_MEM_ERROR 14

// SPI devices
#define SPI_DISP 0
#define SPI_SD 1

// software spi speed
#define SPI_SPEED0 65 // 25 kHz
#define SPI_SPEED1 30 // 50 kHz
#define SPI_SPEED2 13 // 100 kHz
#define SPI_SPEED3 5 //  200 kHz
#define SPI_SPEED4 1 //  400 kHz
#define SPI_SPEED5 0 //  800 kHz

// KINDs
#define LOCKBIT_KIND 0
#define FUSE_KIND 1
#define FLASH_KIND 2
#define EEP_KIND 3

// timer
#define BUSY_MS 10000UL // longest time we consider us self busy 


#define NO_BTN 255


uint8_t state;
uint8_t error;
const char * kindExt[] = { ".LCK", ".FUS", ".HEX", ".EEP" };
const char * kindStr[] = { "Lock", "Fuses", "Flash", "EEPROM" };
int16_t settings[] = { 0, -1, -1, 0};
int16_t speedopt[] = { 2, -2, -2, -2,  -2, 0};
uint8_t spidelayvals[] = { SPI_SPEED0, SPI_SPEED1, SPI_SPEED2, SPI_SPEED3, SPI_SPEED4 };
uint8_t spidelay = SPI_SPEED0;
uint8_t progspidelay = SPI_SPEED0;
uint16_t mcusig;
int16_t mcuix = -1;
boolean progmode = false;
int16_t mmpos, setpos;
char buf[MAX_STRING_LENGTH+1];

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
#ifdef DEBUG
  SerialUSB.begin(115200);
  while (!SerialUSB);
  SerialUSB.println("AvrBurn V" VERSION " starts...");
#endif
  start_burner();
}

void start_burner()
{
  state = MENU_STATE;
  error = NO_ERROR;
  progmode = false;
  mmpos = 0;
  setpos = 0;
}

		    
void loop() {
  if (gb.update()) {
    if (gb.buttons.pressed(BUTTON_MENU)) state = RESET_STATE;
    if (error && state != RESET_STATE) state = ERROR_STATE;
    DEBPR(F("loop: state="));
    DEBLN(state);
    switch (state) {
    case RESET_STATE: start_burner(); break;
    case MENU_STATE: main_menu(); break;
    case INFO_STATE: display_info(); break;
    case SETTINGS_STATE: settings_menu(); break;
    case SPEED_STATE: speed_menu(); break;
    case DETECT_STATE: mcu_detect(); break;
    case ERASE_STATE: erase(); break;
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
  int item = menu("AVR Burn Menu",mainmenu, NULL, NUMELS(mainmenu), mmpos);
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
  default: error = NYI_ERROR + item; break;
  }
  DEBPR(F("main_menu exit: state="));
  DEBLN(state);
}


void settings_menu()
{
  setpos = 0;
  int16_t item = menu("Settings Menu",settingsmenu, settings, NUMELS(settingsmenu), setpos);
  if (state == RESET_STATE) return;
  switch (item) {
  case -1:
  case ST_EXIT: state = MENU_STATE; break;
  case ST_SPISPEED: state = SPEED_STATE; setpos = item; break;
  default: error = NYI_ERROR + item; break;
  }
 
}

void speed_menu()
{
  menu("Speed Menu",speedmenu, speedopt, NUMELS(speedmenu), 0);
  for (byte i = 0; i < NUMELS(speedopt); i++)
    if (speedopt[i] == 2) progspidelay = spidelayvals[i];
  if (state != RESET_STATE) state = SETTINGS_STATE;
}

void rwv_menu(byte kind)
{
  int16_t item = menu(rvwtitle[kind],rwvmenu, NULL, NUMELS(rwvmenu), 0);
  if (state == RESET_STATE) return;
  switch(item) {
  case RWV_PROG: program_file(kind); break;
  case RWV_SAVE: save_file(kind); break;
  case RWV_VERIFY: verify_file(kind); break;
  }
  if (state != RESET_STATE) state = MENU_STATE; 
}

void fuse_menu()
{
  int16_t item = menu("Fuse Menu",fusemenu, NULL, NUMELS(fusemenu), 0);
  if (state == RESET_STATE) return;
  switch(item) {
  case FUSE_PROG: program_file(FUSE_KIND); break;
  case FUSE_SAVE: save_file(FUSE_KIND); break;
  case FUSE_VERIFY: verify_file(FUSE_KIND); break;
  case FUSE_DEFAULT: set_default_fuse(); break;
  }
  if (state != RESET_STATE) state = MENU_STATE; 
}


void display_error(byte errnum)
{
  while (1) {
    while(!gb.update());
    if (check_OK()) return;
    gb.display.clear();
    gb.display.fontSize = 2;
    gb.display.println(F("  ERROR"));
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
      case NO_MEM_ERROR:
	gb.display.println(F("Not enough"));
	gb.display.println(F("RAM!"));
	break;
      default:
	gb.display.println(F("Unknown"));
	gb.display.print(F("Error: "));
	gb.display.println(errnum);
	break;
      }
    }
    display_OK();
  }
}

void display_OK()
{
  gb.display.fillRect(gb.display.width()/2-gb.display.fontWidth-2,gb.display.height()-gb.display.fontHeight-2,2*gb.display.fontWidth+4,gb.display.fontHeight+2);
  gb.display.setColor(RED);
  gb.display.setCursor(gb.display.width()/2-gb.display.fontWidth,gb.display.height()-gb.display.fontHeight);
  gb.display.print("OK");
  gb.display.setColor(WHITE);    
}

boolean check_OK()
{
  if (gb.buttons.released(BUTTON_A)) {
    error = NO_ERROR;
    state = MENU_STATE;
    gb.sound.playOK();
    return true;
  }
  if (gb.buttons.released(BUTTON_MENU)) {
    gb.sound.playCancel();
    state = RESET_STATE;
    return true;
  }
  return false;
}

void display_info()
{
  while (1) {
    while(!gb.update());
    if (check_OK()) return;
    gb.display.clear();
    gb.display.fontSize = 2;
    gb.display.println(F("  INFO"));
    gb.display.fontSize = 1;
    gb.display.println();
    gb.display.println("Version: " VERSION);
    gb.display.print("Free RAM: ");
    gb.display.println(gb.getFreeRam());
    display_OK();
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
  while (1) {
    while(!gb.update());
    if (check_OK()) return;
    gb.display.clear();
    gb.display.fontSize = 2;
    gb.display.println(F("    MCU"));
    gb.display.fontSize = 1;
    gb.display.println();
    gb.display.println(mcu_name(mcusig));
    display_OK();
  }
}

const char * mcu_name(uint16_t sig)
{
  DEBPR(F("mcu_name:"));
  DEBLNF(sig,HEX);
  for (int i=0; i < NUMELS(mcuList); i++) {
    DEBPR(F("Loop: "));
    DEBPR(i);
    DEBPR(F(" "));
    DEBPRF(sig,HEX);
    DEBPR(F(" "));
    DEBLNF(mcuList[i].signature,HEX);
    if (mcuList[i].signature  == sig) {
      DEBLN(F("Found"));
      mcuix = i;
      return mcuList[i].name;
    }
  }
  error = UNKNOWN_SIG_ERROR;
  return mcuList[0].name;
}

void erase()
{
  gb.display.clear();
  gb.display.fontSize = 2;
  gb.display.println("Chip Erase");
  gb.display.println();
  gb.display.fontSize = 1;
  gb.display.println("Proceeding ...");
  while (!gb.update());
  erase_chip();
  while (1) {
    while(!gb.update());
    if (check_OK()) return;
    gb.display.clear();
    gb.display.fontSize = 2;
    gb.display.println(F("Chip Erase"));
    gb.display.fontSize = 1;
    gb.display.println();
    gb.display.println("Done.");
    display_OK();
  }
}
    
void program_file(byte kind)
{
  char path[MAX_STRING_LENGTH+6] = "/BURN/";
  uint8_t percentage = 0;
  char * filename = choose_file(false,kind);
  boolean done = false;
  if (!filename) return;
  strcat(path,filename);
  File file = SD.open(path);
  while (true) {
    if (gb.update()) {
      gb.display.clear();
      gb.display.fontSize = 2;
      gb.display.println(" Program");
      gb.display.fontSize = 1;
      gb.display.println();
      gb.display.println(filename);
      display_progress(percentage);
      if (gb.buttons.released(BUTTON_MENU)) {
	file.close();
	state = RESET_STATE;
	file.close();
	return;
      }
      if (done) {
	gb.display.println("Done!");
	display_OK();
	if (check_OK()) {
	  file.close();
	  return;
	}
      }
      done = prog_page(percentage);
      if (!done) percentage++;
    }
  }
}

void display_progress(uint8_t p) {
  gb.display.setColor(RED);
  gb.display.drawRect(3,40,74,8);
  gb.display.fillRect(3,40,(int)(74*p/100),8);
  gb.display.setColor(WHITE);
}

boolean prog_page(uint8_t p) {
  delay(50);
  DEBLN(p);
  return (p >= 100);
}

void save_file(byte kind)
{
}

void verify_file(byte kind)
{
  choose_file(true,kind);
}

void set_default_fuse()
{
}

char * choose_file(boolean verify, byte kind)
{
  File dir;
  int16_t item;
  int16_t entrycnt = 0;
  char ** dlist;
  char title[20];
  const char * strVerify = "Verify ";
  const char * strRead = "Program ";
  char * strNoFile = "-- No File --";
  DEBLN(F("choose_file"));
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
  if (state == RESET_STATE) 
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

char ** read_dir(File dir, uint8_t kind)
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
  return (!stricmp(&buf[strlen(buf)-4],kindExt[kind]));
}

int16_t count_entries(File dir, uint8_t kind)
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
