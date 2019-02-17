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
 * V0.4
 *    - Fuses and lock bits can be programmed, and verified
 *    - Activity screens look similar to menu screens
 *    - Only one NYI error
 */

// #define DEBUG

#define VERSION "0.4"

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

// max size of page buffer
#define MAX_PAGE_SIZE 256

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
#define STATE_ERROR 1
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

// special verifiaction addresses
#define VADDR_LOCK -4
#define VADDR_LO -1
#define VADDR_HI -2
#define VADDR_EX -3

// software spi speed
#define SPI_SPEED0 65 // 25 kHz
#define SPI_SPEED1 30 // 50 kHz
#define SPI_SPEED2 13 // 100 kHz
#define SPI_SPEED3 5 //  200 kHz
#define SPI_SPEED4 1 //  400 kHz
#define SPI_SPEED5 0 //  800 kHz
#define FUSE_SPI_SPEED SPI_SPEED0

// KINDs
#define LOCKBIT_KIND 0
#define FUSE_KIND 1
#define FLASH_KIND 2
#define EEP_KIND 3

// timer
#define BUSY_MS 3000UL // longest time we consider us self busy 


#define NO_BTN 255


uint8_t state;
uint8_t error;
const char * kindExt[] = { ".LCK", ".FUS", ".HEX", ".EEP" };
const char * kindStr[] = { "Lock", "Fuses", "Flash", "EEPROM" };
int16_t settings[] = { 0, 1, 1, 0};
boolean autoverify = true;
boolean autoerase = true;
int16_t speedopt[] = { -2, -2, -2, 2,  -2, -2, 0};
uint8_t spidelayvals[] = { SPI_SPEED0, SPI_SPEED1, SPI_SPEED2, SPI_SPEED3, SPI_SPEED4, SPI_SPEED5 };
uint8_t spidelay = SPI_SPEED0;
uint8_t progspidelay = SPI_SPEED3;
uint16_t mcusig;
uint16_t mcuix;
boolean progmode = false;
int16_t mmpos, setpos;
char buf[MAX_STRING_LENGTH+1];
uint8_t pagebuf[MAX_PAGE_SIZE];
boolean verified;
int32_t verifyaddr;
uint8_t verifyexpected, verifyseen;
const char * verifyaddrStr[] = { "", "Low fuse", "High fuse", "Ext. Fuse", "Lock bits" };

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
  set_prog_mode(false);
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
    case LOCK_STATE: fuse_menu(LOCKBIT_KIND); break;
    case FLASH_STATE:rwv_menu(FLASH_KIND); break;
    case EEP_STATE:rwv_menu(EEP_KIND); break;
    case FUSE_STATE: fuse_menu(FUSE_KIND); break;
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
  default: error = NYI_ERROR; break;
  }
  DEBPR(F("main_menu exit: state="));
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
  autoverify = (settings[ST_AUTOVERIFY] == 1);
  autoerase = (settings[ST_AUTOERASE] == 1);
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
  case RWV_PROG: program(kind); break;
  case RWV_READ: save_file(kind); break;
  case RWV_VERIFY: verify(kind); break;
  default: error = NYI_ERROR; break;
  }
  if (state != RESET_STATE) state = MENU_STATE; 
}

void fuse_menu(byte kind)
{
  int16_t item = menu(rvwtitle[kind],fusemenu, NULL, NUMELS(fusemenu), 0);
  if (state == RESET_STATE) return;
  switch(item) {
  case FUSE_PROG: program(kind); break;
  case FUSE_READ: save_file(kind); break;
  case FUSE_VERIFY: verify(kind); break;
  case FUSE_DEFAULT: set_default_values(kind); break;
  default: error = NYI_ERROR; break;
  }
  if (state != RESET_STATE) state = MENU_STATE; 
}

void display_header(Color color, const char * title)
{
  gb.display.clear();
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

void display_error(byte errnum)
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
      gb.display.println(F("This function"));
      gb.display.println(F("is not yet"));
      gb.display.println(F("implemented!"));
      break;
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
    case NO_MCU_ERROR:
      gb.display.println(F("No MCU detected!"));
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
    case FILE_DIR_ERROR:
      gb.display.println(F("File is a"));
      gb.display.println(F("directory!"));
      break;
    case FILE_OPEN_ERROR:
      gb.display.println(F("Cannot open"));
      gb.display.println(F("file!"));
      break;
    case HEX_FILE_ERROR:
      gb.display.println(F("Format error"));
      gb.display.println(F("in HEX file!"));
      break;
    default:
      gb.display.println(F("Unknown"));
      gb.display.print(F("Error: "));
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
    state = MENU_STATE;
    gb.sound.playOK();
    return true;
  }
  if (check_RST()) return true;
  return false;
}


void display_info()
{
  while (1) {
    while(!gb.update());
    if (check_OK()) return;
    display_header(YELLOW, "Info");
    gb.display.println("Version: " VERSION);
    gb.display.print("Free RAM: ");
    gb.display.println(gb.getFreeRam());
    display_OK();
  }
}




void mcu_detect()
{
  uint16_t mix;
  DEBLN(F("mcu_detect"));
  mcusig = read_sig();
  DEBPR(F("sig="));
  DEBLNF(mcusig,HEX);
  mix = mcu_ix(mcusig);
  if (error) return;
  while (1) {
    while(!gb.update());
    if (check_OK()) return;
    display_header(YELLOW, "Detected MCU");
    gb.display.print("MCU: ");
    gb.display.println(mcuList[mix].name);
    display_OK();
  }
}

uint16_t mcu_ix(uint16_t sig)
{
  DEBPR(F("mcu_ix:"));
  DEBLNF(sig,HEX);
  for (int i=0; i < NUMELS(mcuList); i++) {
    if (mcuList[i].signature  == sig) {
      return i;
    }
  }
  error = UNKNOWN_SIG_ERROR;
  return 0;
}

void erase()
{
  display_header(YELLOW, "Chip Erase");
  gb.display.println("Erasing ...");
  while (!gb.update());
  erase_chip();
  while (1) {
    while(!gb.update());
    if (check_OK()) return;
    display_header(YELLOW, "Chip Erase");
    gb.display.println("Erasing ...");
    gb.display.println("Done.");
    display_OK();
  }
}
    
void program(byte kind)
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

void verify(byte kind)
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


boolean read_file(boolean doverify, char * filename, File & file, uint8_t kind)
{
  char title[MAX_STRING_LENGTH];
  boolean done = false;
  uint8_t percentage = 0;
  uint16_t mix = mcu_ix(mcusig = read_sig());
  boolean verified;
  if (!error & mix == 0) error = NO_MCU_ERROR;
  if (error) {
    if (file) file.close();
    return false;
  }
  strcpy(title, (doverify ? "Verifying " : " Programming "));
  strcat(title, kindStr[kind]);
  DEBLN("read_file");
  file.seek(0);
  while (true) {
    while (!gb.update());
    display_header(YELLOW, title);
    gb.display.print("File: ");
    gb.display.println(filename);
    display_progress(percentage);
    if (check_RST()) {
      file.close();
      return false;
    }
    if (done) {
      gb.display.setCursor(0,20);
      gb.display.print((doverify ? (verified ? "Verified!" : "Deviation:") : "Programmed!"));
      if (autoverify && !doverify) return true;
      if (doverify && !verified) {
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
      display_OK();
      if (check_OK()) {
	file.close();
	return false;
      }
    }
    while (!done && percent_read(file) - percentage < 2) {
      done = (doverify ?  verify_page(file, kind, mix, verified) : prog_page(file, kind, mix));
      DEBPR("after read_file: ");
      DEBPR("doverify=");
      DEBPR(doverify);
      DEBPR(", done=");
      DEBPR(done);
      DEBPR(", error=");
      DEBPR(error);
      percentage = percent_read(file);
      DEBPR(", %=");
      DEBLN(percentage);
    }
    if (done) percentage = 100;
    if (error) {
      file.close();
      return false;
    }
  }
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



boolean prog_page(File & file, uint8_t kind, uint16_t mcuix) {
  DEBLN("prog_page");
  switch (kind) {
  case LOCKBIT_KIND:
    fill_page_buf(file, 1);
    if (!error) program_lock(pagebuf[0]);
    return true; 
  case FUSE_KIND:
    fill_page_buf(file, mcuList[mcuix].fuses);
    if (!error) program_fuses(mcuList[mcuix].fuses, pagebuf[0], pagebuf[1], pagebuf[2]);
    return true;
  default: error = NYI_ERROR; break;
  }
  return true;
}

boolean verify_page(File & file, uint8_t kind, uint16_t mcuix, boolean & verified) {
  DEBLN("verify_page");
  switch (kind) {
  case LOCKBIT_KIND:
    fill_page_buf(file, 1);
    if (!error) verified = verify_lock(pagebuf[0]);
    return true; 
  case FUSE_KIND:
    fill_page_buf(file, mcuList[mcuix].fuses);
    if (!error) verified = verify_fuses(mcuList[mcuix].fuses, pagebuf[0], pagebuf[1], pagebuf[2]);
    return true;
  default: error = NYI_ERROR; break;
  }
  return true;
}


void fill_page_buf(File & file, uint8_t count)
{
  DEBLN("fill_page_buf");
  for (uint8_t i=0; i < count; i++) {
    pagebuf[i] = read_hex_byte(file);
    if (i != count-1) skip_eol(file);
  }
}

uint8_t read_hex_byte(File & file)
{
  DEBLN("read_hex_byte");
  byte b = 0;
  b = hex_to_num(file, file.read());
  b <<= 4;
  b |= hex_to_num(file, file.read());
  DEBPR("return: ");
  DEBLNF(b,HEX);
  return b;
}

uint8_t hex_to_num(File & file, char c)
{
  DEBPR("hex_to_num: pos=");
  DEBPR(file.position());
  DEBPR(", c=");
  DEBLNF(c,HEX);
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
  DEBPR("skip_eol: pos=");
  DEBPR(file.position());
  DEBPR(", peek=");
  DEBLNF(file.peek(),HEX);
  if (file.peek() != '\n' && file.peek() != '\r') {
    error = HEX_FILE_ERROR;
    DEBLN("error return");
    return false;
  }
  while (file.peek() == 0x0A || file.peek() == 0x0D)
    file.read();
  DEBPR("return: pos=");
  DEBPR(file.position());
  DEBPR(", peek=");
  DEBLNF(file.peek(),HEX);
  return true;
}


void save_file(byte kind)
{
  error = NYI_ERROR;
}

void set_default_values(uint8_t kind)
{
    error = NYI_ERROR;
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
  return (!stricmp(&buf[strlen(buf)-4],kindExt[kind]));
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
