// main menu
#define MAINMENU_LENGTH 7
#define MM_DETECT 0
#define MM_LOCKS 1
#define MM_FUSES 2
#define MM_FLASH 3
#define MM_EEPROM 4
#define MM_SETTINGS 5
#define MM_EXIT 6

const char strDetect[] PROGMEM = "Detect MCU";
const char strErase[] PROGMEM = "Chip erase";
const char strLocks[] PROGMEM = "Locks";
const char strFuses[] PROGMEM = "Fuses";
const char strFlash[] PROGMEM = "Flash";
const char strEEPROM[] PROGMEM = "EEPROM";
const char strSettings[] PROGMEM = "Settings";
const char strExit[] PROGMEM = "Exit";

const char * const mainmenu[MAINMENU_LENGTH] PROGMEM = {
  strDetect, strLocks, strFuses, strFlash, strEEPROM,
  strSettings, strExit };

// settings menu
#define SETTINGS_LENGTH 4
#define ST_SPISPEED 0
#define ST_AUTOVERIFY 1
#define ST_AUTOERASE 2
#define ST_EXIT 3

const char strSpiSpeed[] PROGMEM = "SPI Speed";
const char strAutoVerify[] PROGMEM = "Auto Verify";
const char strAutoErase[] PROGMEM = "Auto Erase";

const char * const settingsmenu[SETTINGS_LENGTH] PROGMEM = {
  strSpiSpeed,  strAutoVerify, strAutoErase, strExit };

// speed menu
#define SPEED_LENGTH 7
#define SP_DIV4 0 // 4 MHz
#define SP_DIV8 1 // 2 MHz
#define SP_DIV16 2 // 1 MHz
#define SP_DIV32 3 // 500 kHz
#define SP_DIV64 4 // 250 kHz
#define SP_DIV128 5 // 125 khz

const char str4mhz[] PROGMEM = "4 MHz";
const char str2mhz[] PROGMEM = "2 MHz";
const char str1mhz[] PROGMEM = "1 MHz";
const char str500khz[] PROGMEM = "500 kHz";
const char str250khz[] PROGMEM = "250 kHz";
const char str125khz[] PROGMEM = "125 kHz";

const char * const speedmenu[SPEED_LENGTH] PROGMEM = {
  str4mhz, str2mhz, str1mhz, str500khz, str250khz, str125khz, strExit };

// read/write/verify menu
#define LOCK_LENGTH 3
#define FUSE_LENGTH 4
#define MEM_LENGTH 3

const char strWrite[] PROGMEM = "Write";
const char strRead[] PROGMEM = "Read";
const char strVerify[] PROGMEM = "Verify";
const char strDefault[] PROGMEM = "Factory default";
