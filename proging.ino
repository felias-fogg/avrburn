// -*- c++ -*-

void prog_mode(boolean on)
{
  if (on) {
#if SPI_CONFLICT
    SPCR &= ~(1 << SPE); // disable SPI subsystem
#endif
    setInput(SOFT_SCK);
    setOutput(SOFT_RST);
    setHigh(SOFT_RST);
    setOutput(SOFT_SCK);
    setLow(SOFT_SCK);
    delay(50);
    setLow(SOFT_RST);
    delay(50);
    setInput(SOFT_MISO);
    setOutput(SOFT_MOSI);
    spidelay = SPI_SPEED0;
    spi_transaction(0xAC, 0x53, 0x00, 0x00);
    delay(11);
    progmode = true;
  } else {
    setLow(SOFT_SCK);
    setInput(SOFT_SCK);
    setLow(SOFT_MISO);
    setInput(SOFT_MISO);
    setLow(SOFT_MOSI);
    setInput(SOFT_MOSI);
    setLow(SOFT_RST);
    setInput(SOFT_RST);
#if SPI_CONFLICT
    setOutput(SOFT_SCK);
    setOutput(SOFT_MISO);
    SPCR |= (1 << SPE); // enable SPI subsystem again
#endif
    progmode = false;
  }
}


uint16_t read_sig()
{
  uint16_t sig;
  spidelay = SPI_SPEED0;
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
