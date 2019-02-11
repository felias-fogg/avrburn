// -*- c++ -*-

// Since we are using the SPI bus for the display, the SD card, and for programming,
// we have to make sure that we operate them with the right speed and mode for each device.
// The Arduino API for that would be SPI.beingTransaction and SPI.endTransaction. However,
// the display class as well as the SD class do not support that. So, all of that has to be done "manually".
// In particular, there here are a few functions that read the SPI registers in order to determine
// the right parameters.


byte spidat[6];

byte get_spi_bitorder()
{
  if (SPCR & _BV(DORD)) return LSBFIRST;
  else return MSBFIRST;
}

byte get_spi_datamode()
{
  return (SPCR & SPI_MODE_MASK);
}

byte get_spi_clock()
{
  return ((SPCR & SPI_CLOCK_MASK) | ((SPSR & SPI_2XCLOCK_MASK) <<2));
}

void get_spi_para(byte dev)
{
  spidat[dev+0] = get_spi_bitorder();
  spidat[dev+1] = get_spi_datamode();
  spidat[dev+2] = get_spi_clock();
}

void set_spi_para(byte dev)
{
  SPI.setBitOrder(spidat[dev+0]);
  SPI.setDataMode(spidat[dev+1]);
  SPI.setClockDivider(spidat[dev+2]);
}
		  

uint32_t spi_transaction(uint8_t a,uint8_t b,uint8_t c,uint8_t d)
{
  uint32_t result;
  result = soft_spi(a);
  result = (result<<8) | soft_spi(b);
  result = (result<<8) | soft_spi(c);
  result = (result<<8) | soft_spi(d);
  return result;
}

uint8_t soft_spi(uint8_t data)
{
  uint8_t result = 0;

  cli();
  for (byte b=0; b < 8; b++) {
    if (data&0x80) 
      setHigh(SOFT_MOSI);
    else
      setLow(SOFT_MOSI);
    data <<= 1;
    result <<= 1;
    setHigh(SOFT_SCK);
    delayMicroseconds(1);
    if (getInput(SOFT_MISO)) result |= 1;
    delayMicroseconds(spidelay);
    setLow(SOFT_SCK);
    delayMicroseconds(spidelay);
  }
  sei();
  return result;
}
