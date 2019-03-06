// -*- c++ -*-

//Port macros
#define setHigh(port)             REG_PORT_OUTSET0 = port
#define setLow(port)              REG_PORT_OUTCLR0 = port
#define getInput(port)            (REG_PORT_IN0 & port)
#define setInput(port)            REG_PORT_DIRCLR0 = port
#define setOutput(port)           REG_PORT_DIRSET0 = port


// Software SPI for ISP

uint32_t spi_transaction(uint8_t a,uint8_t b,uint8_t c,uint8_t d)
{
  uint32_t result;
  result = soft_spi(a);
  result = (result<<8) | soft_spi(b);
  result = (result<<8) | soft_spi(c);
  result = (result<<8) | soft_spi(d);
  return result;
}

inline void soft_delay(uint16_t wait)
{
  for (uint16_t i = 0; i < wait; i++) asm("nop");
}


uint8_t soft_spi(uint8_t data)
{
  uint8_t result = 0;

  noInterrupts();
  for (uint8_t b=0; b < 8; b++) {
    if (data&0x80) 
      setHigh(PORT_MOSI);
    else
      setLow(PORT_MOSI);
    data <<= 1;
    result <<= 1;
    setHigh(PORT_SCK);
    soft_delay(spidelay);
    if (getInput(PORT_MISO)) result |= 1;
    setLow(PORT_SCK);
    soft_delay(spidelay-1);
  }
  interrupts();
  return result;
}

// Programming
void set_prog_mode(boolean on)
{
  DEBPR("set_prog_mode: ");
  DEBLN(on);
  spidelay = FUSE_SPI_SPEED;
  if (on) {
    setInput(PORT_SCK);
    setOutput(PORT_RST);
    setHigh(PORT_RST);
    setOutput(PORT_SCK);
    setLow(PORT_SCK);
    delay(50);
    setLow(PORT_RST);
    delay(50);
    setInput(PORT_MISO);
    setOutput(PORT_MOSI);
    spi_transaction(0xAC, 0x53, 0x00, 0x00);
    delay(11);
    progmode = true;
    check_extended_addr(0);
  } else {
    setLow(PORT_SCK);
    setInput(PORT_SCK);
    setLow(PORT_MISO);
    setInput(PORT_MISO);
    setLow(PORT_MOSI);
    setInput(PORT_MOSI);
    setLow(PORT_RST);
    setInput(PORT_RST);
    progmode = false;
  }
}


uint16_t sig_trans()
{
  uint16_t sig;
  uint8_t vendorid = spi_transaction(0x30, 0x00, 0x00, 0x00) & 0xFF;
  if (vendorid != 0xff && vendorid != 0x00 && vendorid != 0x1E) {
    error = SIG_ERROR;
    DEBLN("SIGNATURE ERROR");
  }
  sig = spi_transaction(0x30, 0x00, 0x01, 0x00) & 0xFF;
  sig <<= 8;
  sig |= spi_transaction(0x30, 0x00, 0x02, 0x00) & 0xFF;
  if (sig == 0xFFFF) sig = 0;
  return sig;
}

uint16_t read_sig()
{
  uint16_t sig;
  spidelay = FUSE_SPI_SPEED;
  sig = sig_trans();
  DEBPR("Signature read:");
  DEBLNF(sig,HEX);
  return sig;
}

void erase_chip(uint16_t mix)
{
  spidelay = FUSE_SPI_SPEED;
  spi_transaction(0xAC, 0x80, 0, 0);
  if (mcuList[mix].erasePoll) wait_for_completion();
  else delay(mcuList[mix].eraseDelay);
}

void wait_for_completion()
{
  uint32_t start = millis();
  uint8_t busy = 1;

  while (busy && (millis() - start < BUSY_MS)) 
    busy = (spi_transaction(0xF0, 0x0, 0x0, 0x0) & 0x01);
}

uint8_t read_lock()
{
  uint8_t lock;
  spidelay = FUSE_SPI_SPEED;
  lock =  spi_transaction(0x58, 0x00, 0x00, 0x00) & 0xFF;
  return lock;
}

uint32_t read_fuses()
{
  spidelay = FUSE_SPI_SPEED;
  uint32_t fusebytes = 0;
  uint8_t fusenum;
  DEBLN("read_fuses");
  uint16_t mcusig = read_sig();
  DEBLN(mcusig);
  fusenum = mcuList[mcu_ix(mcusig)].fuses;
  DEBLN(fusenum);
  if (fusenum >= 3) fusebytes = spi_transaction(0x50, 0x08, 0x00, 0x00) & 0xFF; // ext
  DEBPR("read ext: ");
  DEBLNF(fusebytes,HEX);
  fusebytes = (fusebytes << 8);
  if (fusenum >= 2) fusebytes |= (spi_transaction(0x58, 0x08, 0x00, 0x00) & 0xFF); // high
  DEBPR("read high: ");
  DEBLNF(fusebytes,HEX);
  fusebytes = (fusebytes << 8);
  if (fusenum >= 1) fusebytes  |= (spi_transaction(0x50, 0x00, 0x00, 0x00) & 0xFF); // low
  DEBPR("read lo: ");
  DEBLNF(fusebytes,HEX);
  return fusebytes;
}

void program_lock(uint8_t lock)
{
  spidelay = FUSE_SPI_SPEED;
  spi_transaction(0xAC, 0xE0, 0x00, lock);
  return;
}

boolean verify_lock(uint8_t lock)
{
  uint8_t rlock;
  rlock = read_lock();
  if (rlock == lock) return true;
  verifyaddr = VADDR_LOCK;
  verifyexpected = lock;
  verifyseen = rlock;
  return false;
}

void program_fuses(uint8_t fusenum, uint8_t lo, uint8_t hi, uint8_t ex)
{
  DEBPR("program_fuses: ");
  DEBLNF(lo+(hi<<8)+(ex<<16),HEX);
  if (fusenum >= 1) spi_transaction(0xAC, 0xA0, 0x00, lo);
  delay(100);
  if (fusenum >= 2) spi_transaction(0xAC, 0xA8, 0x00, hi);
  delay(100);
  if (fusenum >= 3) spi_transaction(0xAC, 0xA4, 0x00, ex);    
  return;
}

boolean verify_fuses(uint8_t fusenum, uint8_t lo, uint8_t hi, uint8_t ex) {
  uint8_t rlo = 0, rhi = 0, rex = 0;
  uint32_t fuses = read_fuses();
  DEBPR("verify_fuses: expected=");
  DEBPRF((lo | (hi << 8) | (ex << 16)),HEX);
  DEBPR(", seen=");
  DEBLNF(fuses,HEX);
  if (fusenum >= 1) {
    rlo = fuses & 0xFF;
    if (rlo != lo) {
      verifyaddr = VADDR_LO;
      verifyexpected = lo;
      verifyseen = rlo;
      DEBLN("Lo deviation");
      return false;
    }
  }
  fuses >>= 8;
  if (fusenum >= 2) {
    rhi = fuses & 0xFF;
    if (rhi != hi) {
      verifyaddr = VADDR_HI;
      verifyexpected = hi;
      verifyseen = rhi;
      DEBLN("Hi deviation");
      return false;
    }
  }
  fuses >>= 8;
  if (fusenum >= 3) {
    rex = fuses & 0xFF;
    if (rex != ex) {
      verifyaddr = VADDR_EX;
      verifyexpected = ex;
      verifyseen = rex;
      DEBLN("Ext deviation");
      return false;
    }
  }
  return true;
}

boolean verify_flash(uint32_t pageaddr, uint16_t pagesize)
{
  return verify_mem(pageaddr, pagesize, FLASH_KIND);
}

boolean verify_eeprom(uint32_t pageaddr, uint16_t pagesize)
{
  return verify_mem(pageaddr, pagesize, EEPROM_KIND);
}

boolean verify_mem(uint32_t pageaddr, uint16_t pagesize, uint8_t kind)
{
  DEBLN("verify_mem");
  DEBPR("  pageaddr=");
  DEBLNF(pageaddr,HEX);
  DEBPR("  pagesize=");
  DEBLNF(pagesize,HEX);
  spidelay = progspidelay;
  for (verifyaddr=pageaddr;  verifyaddr < pageaddr+pagesize; verifyaddr++) {
    verifyseen = (kind == FLASH_KIND ? read_flash_byte(verifyaddr) : read_eeprom_byte(verifyaddr));
    DEBPR("VERIFY: addr=");
    DEBPRF(verifyaddr,HEX);
    DEBPR(" seen=");
    DEBPRF(verifyseen,HEX);
    DEBPR(" expct=");
    DEBLNF(pagebuf[verifyaddr-pageaddr],HEX);
    if (verifyseen != pagebuf[verifyaddr-pageaddr]) {
      verifyexpected = pagebuf[verifyaddr-pageaddr];
      return false;
    }
  }
  return true;
}


void program_flash(uint32_t pageaddr, uint16_t pagesize, uint8_t polling, uint8_t wait_ms)
{
  boolean empty = true;
  uint16_t reply;
  for (uint16_t i=0; i < pagesize; i++)
    empty &= (pagebuf[i] == 0xFF);
  if (empty) return;
  spidelay = progspidelay;
  if (pagesize > 1) { // real page programming
    for (uint16_t i=0; i < pagesize/2; i++) {
      spi_transaction(0x40, (i >> 8) & 0xFF, i & 0xFF,  pagebuf[2 * i]);
      spi_transaction(0x48, (i >> 8) & 0xFF, i & 0xFF,  pagebuf[2 * i + 1]);
    }
    //convert to word address
    check_extended_addr(pageaddr);
    pageaddr >>= 1;
    reply = spi_transaction(0x4C, (pageaddr >> 8) & 0xFF, pageaddr & 0xFF, 0);
    if (reply != (pageaddr & 0xFFFF)) {
      error = FLASH_PROG_ERROR;
      return;
    }
  } else { // byte programming
    spi_transaction(0x40 + (pageaddr%2)*8, (pageaddr >> 9) & 0xFF, (pageaddr >> 1) & 0xFF,  pagebuf[0]);
  }
  if (polling == 0) wait_for_completion(); // usual busy polling
  else {
    uint16_t pollix = 0; 
    while (pagebuf[pollix] == 0xFF && pollix < pagesize) pollix++; // find entry different from 0xFF
    if (pollix >= pagesize) delay(wait_ms); // if there is no one, just delay
    else {
      uint32_t start = millis(); // otherwise wait for programmed value to show up
      while (pagebuf[pollix] != read_flash_byte(pageaddr + pollix) && (millis() - start <= wait_ms)) delayMicroseconds(50);
    }
  }
}

void program_eeprom(uint32_t pageaddr, uint16_t pagesize, uint8_t polling, uint8_t wait_ms)
{
  uint16_t reply;
  spidelay = progspidelay;
  if (pagesize > 1) { // real page programming
    for (uint16_t i=0; i < pagesize; i++) {
      spi_transaction(0xC1, 0, i & 0xFF,  pagebuf[i]);
    }
    reply = spi_transaction(0xC2, (pageaddr >> 8) & 0xFF, pageaddr & 0xFF, 0);
    if (reply != pageaddr) {
      error = EEPROM_PROG_ERROR;
      return;
    }
  } else { // byte programming
    DEBPR("EEPROM prog addr=");
    DEBPRF(pageaddr,HEX);
    DEBPR(", byte=");
    DEBLNF(pagebuf[0],HEX);
    spi_transaction(0xC0, (pageaddr >> 8) & 0xFF, pageaddr  & 0xFF,  pagebuf[0]);
  }
  if (polling == 0) wait_for_completion(); // usual busy polling
  else {
    uint16_t pollix = 0; 
    while (pagebuf[pollix] == 0xFF && pollix < pagesize) pollix++; // find entry different from 0xFF
    if (pollix >= pagesize) delay(wait_ms); // if there is no one, just delay
    else {
      uint32_t start = millis(); // otherwise wait for programmed value to show up
      while (pagebuf[pollix] != read_eeprom_byte(pageaddr + pollix) && (millis() - start <= wait_ms)) delayMicroseconds(50);
    }
  }
}


uint8_t read_eeprom_byte(uint32_t addr)
{
  return (spi_transaction(0xA0, addr >> 8, (addr & 0xFF), 0x00) & 0xFF);
}


uint8_t read_flash_byte(uint32_t addr)
{
  check_extended_addr(addr);
  return (spi_transaction(0x20 + 8*(addr % 2), (addr >> 9) & 0xFF, (addr / 2) & 0xFF, 0x00) & 0xFF);
}

// checks whether extended (word) adressing is necessary
// if so, issues the correct SPI command
// (only necessary for ATmega 256)
void check_extended_addr(uint32_t addr)
{
  static uint8_t extendedAddr = 0;
  if (extendedAddr != ((addr >> 17) & 0xFF)) {
    DEBPR("extended address: ");
    DEBLNF((addr >> 17) & 0xFF,HEX);
    extendedAddr = (addr >> 17) & 0xFF;
    spi_transaction(0x4D, 0, extendedAddr, 0);
  }
}
