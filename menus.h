// main menu
#define MAINMENU_LENGTH 8
#define MM_DETECT 0
#define MM_ERASE 1
#define MM_LOCK 2
#define MM_FUSE 3
#define MM_FLASH 4
#define MM_EEP 5
#define MM_SETTINGS 6
#define MM_EXIT 7

const char strDetect[] PROGMEM = "Detect MCU";
const char strErase[] PROGMEM = "Chip erase";
const char strLocks[] PROGMEM = "Lock bits";
const char strFuses[] PROGMEM = "Fuses";
const char strFlash[] PROGMEM = "Flash";
const char strEEPROM[] PROGMEM = "EEPROM";
const char strSettings[] PROGMEM = "Settings";
const char strExit[] PROGMEM = "Exit";

const char * const mainmenu[MAINMENU_LENGTH] PROGMEM = {
  strDetect, strErase, strLocks, strFuses, strFlash, strEEPROM,
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
#define SPEED_LENGTH 6

const char str500khz[] PROGMEM = "500 kHz";
const char str250khz[] PROGMEM = "250 kHz";
const char str166khz[] PROGMEM = "166 kHz";
const char str125khz[] PROGMEM = "125 kHz";
const char str100khz[] PROGMEM = "100 kHz";

const char * const speedmenu[SPEED_LENGTH] PROGMEM = {
  str100khz, str125khz, str166khz, str250khz, str500khz, strExit };

// read/write/verify menu
#define RWV_LENGTH 4
#define RWV_READ 0
#define RWV_WRITE 1
#define RWV_VERIFY 2
#define RWV_EXIT 3


const char strWrite[] PROGMEM = "Write";
const char strRead[] PROGMEM = "Read";
const char strVerify[] PROGMEM = "Verify";
const char strDefault[] PROGMEM = "Factory default";

const char * const rwvmenu[RWV_LENGTH] PROGMEM = {
  strRead, strWrite, strVerify, strExit };

// fuse menu
#define FUSE_LENGTH 5
#define FUSE_READ 0
#define FUSE_WRITE 1
#define FUSE_VERIFY 2
#define FUSE_DEFAULT 3
#define FUSE_EXIT 4

const char * const fusemenu[FUSE_LENGTH] PROGMEM = {
  strRead, strWrite, strVerify, strDefault, strExit };
