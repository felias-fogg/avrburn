// -*- c++ -*-

//Port-Macros
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
void prog_mode(boolean on)
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


uint16_t read_sig()
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

void erase_chip()
{
  prog_mode(true);
  spidelay = SPI_SPEED0;
  spi_transaction(0xAC, 0x80, 0, 0);
  waitForRelease();
  prog_mode(false);
}

void waitForRelease()
{
  uint32_t start = millis();
  uint8_t busy = 1;

  while (busy && (millis() - start < BUSY_MS)) 
    busy = (spi_transaction(0xF0, 0x0, 0x0, 0x0) & 0x01);
}
