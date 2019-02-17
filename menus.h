// Menu result codes
// main menu
#define MM_DETECT 0
#define MM_ERASE 1
#define MM_LOCK 2
#define MM_FUSE 3
#define MM_FLASH 4
#define MM_EEP 5
#define MM_SETTINGS 6
#define MM_INFO 7
#define MM_EXIT 8

// settings menu
#define ST_SPISPEED 0
#define ST_AUTOVERIFY 1
#define ST_AUTOERASE 2
#define ST_EXIT 3

// read/write/verify menu
#define RWV_PROG 0
#define RWV_READ 1
#define RWV_VERIFY 2
#define RWV_EXIT 3

// fuse menu
#define FUSE_PROG 0
#define FUSE_READ 1
#define FUSE_VERIFY 2
#define FUSE_DEFAULT 3
#define FUSE_EDIT 4
#define FUSE_EXIT 5

const char * mainmenu[]  = {
  "Detect MCU", "Chip erase",  "Lock bits", "Fuses", "Flash", "EEPROM", "Settings", "Info", "Exit" };

const char * settingsmenu[]  = {
  "SPI Speed", "Auto Verify", "Auto Erase", "Exit" };


const char * speedmenu[]  = {
  "25 kHz", "50 kHz", "100 kHz", "200 kHz", "400 kHz", "800 kHz", "Exit" };

const char *  rwvmenu[]  = {
  "Program", "Read",  "Verify",  "Exit" };

const char *  rvwtitle[] = {
  "Lock Bit Menu", "Fuse Menu", "Flash Menu", "EEPROM Menu", "Exit" };

const char *  fusemenu[]   = {
  "Program", "Read",  "Verify", "Factory default", "Edit", "Exit" };
