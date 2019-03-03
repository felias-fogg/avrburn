typedef struct fuseItems
{
  const uint16_t mes;
  const uint8_t addr;
  const uint8_t mask;
  const uint8_t value;
} fuseItems;

typedef struct fuseMenu {
  const uint16_t sig;
  const fuseItems fuseList[100];
} fuseMenu;

#define N1V8 0
#define N2V7 1
#define BODEN 2
#define SPIEN 3
#define RSTDISBL 4
#define EXTXTALCRES 5
#define EXTLOFXTAL 6
#define EXTRCOSC 7
#define EXTCLK 8
#define INTRCOSC 9
#define SELFPRGEN 10
#define  DWEN 11
#define WDTON 12
#define EESAVE 13
#define DISABLED 14
#define N4V3 15
#define CKDIV8 16
#define CKOUT 17
#define EXTCLK_6CK_14CK_0MS 18
#define EXTCLK_6CK_14CK_4MS1 19
#define INTRCOSC_8MHZ_6CK_14CK_0MS 20
#define INTRCOSC_8MHZ_6CK_14CK_4MS 21
#define INTRCOSC_8MHZ_6CK_14CK_64MS 22
#define EXIT 23

const char * fuseProps[] = {
  "BOD 1.8 Vcc",
  "BOD 2.7 Vcc",
  "BOD enabled",
  "SPI prog. enabled",
  "RESET disabled",
  "Ext XTAL/Res.",
  "Ext low freq XTAL",
  "Ext RC osc",
  "Int RC osc",
  "Self prog enabled",
  "dWire enabled",
  "WDT always on",
  "Save EEP if erase",
  "BOD disabled",
  "BOD 4.3 Vcc",
  "Divide CLK by 8",
  "CLK output on port",
  "Ext CLK fast start",
  "Ext CLK med start",
  "Int RC fast start",
  "Int RC med start",
  "Int RC slow start",
  "EXIT" };

const fuseMenu fuseMenuList[] = {
  { 0, { } },
  { 0x9005, {
      { N1V8, 0, 0x80, 0x80 },
      { N2V7, 0, 0x80, 0x00 },
      { BODEN, 0, 0x40, 0x00 },
      { SPIEN, 0, 0x20, 0x00 },
      { RSTDISBL, 0, 0x10, 0x00 },
      { EXTXTALCRES, 0, 0x0f, 0x0f },
      { EXTXTALCRES, 0, 0x0f, 0x0e },
      { EXTXTALCRES, 0, 0x0f, 0x0d },
      { EXTXTALCRES, 0, 0x0f, 0x0c },
      { EXTXTALCRES, 0, 0x0f, 0x0b },
      { EXTXTALCRES, 0, 0x0f, 0x0a },
      { EXTLOFXTAL, 0, 0x0f, 0x09 },
      { EXTLOFXTAL, 0, 0x0f, 0x08 },
      { EXTRCOSC, 0, 0x0f, 0x07 },
      { EXTRCOSC, 0, 0x0f, 0x06 },
      { EXTRCOSC, 0, 0x0f, 0x05 },
      { INTRCOSC, 0, 0x0f, 0x04 },
      { INTRCOSC, 0, 0x0f, 0x03 },
      { INTRCOSC, 0, 0x0f, 0x02 },
      { EXTCLK, 0, 0x0f, 0x01 },
      { EXTCLK, 0, 0x0f, 0x01 },
      { EXIT, 0, 0, 0 } } },
  { 0x930B, {
      { SELFPRGEN, 2, 0x01, 0x00 },
      { RSTDISBL, 1, 0x80, 0x00 },
      { DWEN, 1, 0x40, 0x00 },
      { SPIEN, 1, 0x20, 0x00 },
      { WDTON, 1, 0x10, 0x00 },
      { EESAVE, 1, 0x08, 0x00 },
      { DISABLED, 1, 0x07, 0x07 },
      { N1V8, 1, 0x07, 0x06 },
      { N2V7, 1, 0x07, 0x05 },
      { N4V3, 1, 0x07, 0x04 },
      { CKDIV8, 0, 0x80, 0x00 },
      { CKOUT, 0, 0x40, 0x00 },
      { EXTCLK_6CK_14CK_0MS, 0, 0x3F, 0x00 },
      { EXTCLK_6CK_14CK_4MS1, 0, 0x3F, 0x10 },
      { INTRCOSC_8MHZ_6CK_14CK_0MS, 0, 0x3F, 0x02 },
      { INTRCOSC_8MHZ_6CK_14CK_4MS, 0, 0x3F, 0x12 },
      { INTRCOSC_8MHZ_6CK_14CK_64MS, 0, 0x3F, 0x22 },
      { EXIT, 0, 0, 0 } } }
};
			      
		    										  
  
