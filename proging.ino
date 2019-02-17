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
  if (spidelay == 0) {
    result = fast_soft_spi(a);
    result = (result<<8) | fast_soft_spi(b);
    result = (result<<8) | fast_soft_spi(c);
    result = (result<<8) | fast_soft_spi(d);
  } else {
    result = soft_spi(a);
    result = (result<<8) | soft_spi(b);
    result = (result<<8) | soft_spi(c);
    result = (result<<8) | soft_spi(d);
  }
  return result;
}

void soft_delay(uint8_t wait)
{
  for (uint8_t i = 0; i < wait; i++) asm("nop");
}

uint8_t fast_soft_spi(uint8_t data)
{
  uint8_t result = 0;

  noInterrupts();
  for (byte b=0; b < 8; b++) {
    if (data&0x80) 
      setHigh(PORT_MOSI);
    else
      setLow(PORT_MOSI);
    data <<= 1;
    result <<= 1;
    setHigh(PORT_SCK);
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    if (getInput(PORT_MISO)) result |= 1;
    setLow(PORT_SCK);
  }
  interrupts();
  return result;
}

uint8_t soft_spi(uint8_t data)
{
  uint8_t result = 0;

  noInterrupts();
  for (byte b=0; b < 8; b++) {
    if (data&0x80) 
      setHigh(PORT_MOSI);
    else
      setLow(PORT_MOSI);
    data <<= 1;
    result <<= 1;
    setHigh(PORT_SCK);
    soft_delay(spidelay);
    if (getInput(PORT_MISO)) result |= 1;
    soft_delay(spidelay-1);
    setLow(PORT_SCK);
    soft_delay(spidelay*2-1);
  }
  interrupts();
  return result;
}

// Programming
void set_prog_mode(boolean on)
{
  spidelay = 65;
  
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
  byte vendorid = spi_transaction(0x30, 0x00, 0x00, 0x00) & 0xFF;
  if (vendorid != 0xff && vendorid != 0x00 && vendorid != 0x1E) {
    error = SIG_ERROR;
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
  set_prog_mode(true);
  sig = sig_trans();
  set_prog_mode(false);
  return sig;
}

void erase_chip()
{
  set_prog_mode(true);
  spidelay = FUSE_SPI_SPEED;
  spi_transaction(0xAC, 0x80, 0, 0);
  waitForRelease();
  set_prog_mode(false);
}

void waitForRelease()
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
  set_prog_mode(true);
  lock =  spi_transaction(0x58, 0x00, 0x00, 0x00) & 0xFF;
  set_prog_mode(false);
  return lock;
}

uint32_t read_fuses()
{
  uint32_t fusebytes = 0;
  uint8_t fusenum;
  spidelay = FUSE_SPI_SPEED;
  set_prog_mode(true);
  mcusig = sig_trans();
  fusenum = mcuList[mcu_ix(mcusig)].fuses;
  if (fusenum >= 3) fusebytes = spi_transaction(0x50, 0x08, 0x00, 0x00) & 0xFF; // ext
  fusebytes = (fusebytes << 8);
  if (fusenum >= 2) fusebytes |= (spi_transaction(0x58, 0x08, 0x00, 0x00) & 0xFF); // high
  fusebytes = (fusebytes << 8);
  if (fusenum >= 1) fusebytes  |= (spi_transaction(0x50, 0x00, 0x00, 0x00) & 0xFF); // low
  set_prog_mode(false);
  return fusebytes;
}

void program_lock(uint8_t lock)
{
  spidelay = FUSE_SPI_SPEED;
  set_prog_mode(true);
  spi_transaction(0xAC, 0xE0, 0x00, lock);
  set_prog_mode(false);
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
  spidelay = FUSE_SPI_SPEED;
  set_prog_mode(true);
  if (fusenum >= 1) spi_transaction(0xAC, 0xA0, 0x00, lo);
  if (fusenum >= 2) spi_transaction(0xAC, 0xA8, 0x00, hi);
  if (fusenum >= 3) spi_transaction(0xAC, 0xA4, 0x00, ex);    
  set_prog_mode(false);
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

