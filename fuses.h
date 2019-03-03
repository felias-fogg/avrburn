typedef struct fuseItems
{
  const char fieldname[20];
  const uint8_t addr;
  const uint8_t mask;
  const uint8_t value;
} fuseItems;

typedef struct fuseMenu {
  const char mcu[20];
  const fuseItems fuseList[100];
} fuseMenu;

const fuseMenu fuseMenuList[] = {
  { "No MCU", { } },
  { "ATtiny12", {
      { "BOD at 1.8Vcc", 0, 0x80, 0x80 },
      { "BOD at 2.7Vcc", 0, 0x80, 0x00 },
      { "BOD enabled", 0, 0x40, 0x00 },
      { "SPI Prog. enabled", 0, 0x20, 0x00 },
      { "Reset disabled", 0, 0x10, 0x00 },
      { "Ext Cr 1KCk", 0, 0x0f, 0x0f },
      { "Ext Cr 4.2ms+1KCk", 0, 0x0f, 0x0e },
      { "Ext Cr 67ms+1KCk", 0, 0x0f, 0x0d },
      { "Ext Cr 16KCk", 0, 0x0f, 0x0c },
      { "Ext Cr 4.2ms+16KCk", 0, 0x0f, 0x0b },
      { "Ext Cr 67ms+16KCk", 0, 0x0f, 0x0a },
      { "Ext LwCr 67ms+1KCk", 0, 0x0f, 0x09 },
      { "Ext LwCr 67ms+32KCk", 0, 0x0f, 0x08 },
      { "Ext RC 6Ck", 0, 0x0f, 0x07 },
      { "Ext RC 4.2ms+6Ck", 0, 0x0f, 0x06 },
      { "Ext RC 67ms+6Ck", 0, 0x0f, 0x05 },
      { "Int RC 6Ck", 0, 0x0f, 0x04 },
      { "Int RC 4.2ms+6Ck", 0, 0x0f, 0x03 },
      { "Int RC 67ms+6Ck", 0, 0x0f, 0x02 },
      { "Ext Clk 6Ck", 0, 0x0f, 0x01 },
      { "Ext Clk 4.2ms+6Ck", 0, 0x0f, 0x01 },
      { "Exit", 0, 0, 0 } } }
};
			      
		    
											  

  
