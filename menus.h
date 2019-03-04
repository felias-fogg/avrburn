// Menu result codes
// main menu
#define MM_DETECT 0
#define MM_FLASH 1
#define MM_EEP 2
#define MM_FUSE 3
#define MM_LOCK 4
#define MM_ERASE 5
#define MM_SETTINGS 6
#define MM_INFO 7
#define MM_EXIT 8

// settings menu
#define ST_SPISPEED 0
#define ST_AUTOVERIFY 1
#define ST_AUTOERASE 2
#define ST_SAFEEDIT 3
#define ST_EXIT 4

// read/write/verify menu
#define RWV_PROG 0
#define RWV_VERIFY 1
#define RWV_READ 2
#define RWV_EXIT 3

// fuse menu
#define FUSE_PROG 0
#define FUSE_VERIFY 1
#define FUSE_SHOW 2
#define FUSE_READ 3
#define FUSE_DEFAULT 4
#define FUSE_EDIT 5
#define FUSE_EXIT 6

const char * mainmenu[]  = {
  "Detect MCU",  "Flash", "EEPROM",  "Fuses",  "Lock bits", "Chip erase", "Settings", "Info", "Restart" };

const char * settingsmenu[]  = {
  "SPI Speed", "Auto Verify", "Auto Erase", "Safe Fuse Editting", "Exit" };


const char * speedmenu[]  = {
  "10 kHz", "20 kHz", "50 kHz", "100 kHz", "200 kHz", "400 kHz", "600 kHz", "Exit" };

const char *  rwvmenu[]  = {
  "Program",  "Verify",  "Read & Save", "Exit" };

const char *  rvwtitle[] = {
  "Lock Bit Menu", "Fuse Menu", "Flash Menu", "EEPROM Menu", "Exit" };

const char *  fusemenu[]   = {
  "Program", "Verify", "Show", "Read & Save", "Factory default", "Edit", "Exit" };

const char *  lockmenu[]   = {
  "Program", "Verify", "Show", "Read & Save", "Chip&lockbits erase", "Edit", "Exit" };
