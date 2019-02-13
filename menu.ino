// -*- c++ -*-


// modified menu function
const uint8_t menuYOffset = 9;
void myMenuDrawBox(const char* text, uint8_t i, int32_t y, int16_t optval) {
  y += i*8 + menuYOffset;
  if (y < 0 || y > 64) {
    return;
  }
  gb.display.setColor(DARKGRAY);
  gb.display.fillRect(1, y+1, gb.display.width()-2, 7);
  gb.display.setColor(WHITE);
  gb.display.setCursor(2, y+2);
  gb.display.print(text);
  if (optval) {
    switch (optval) {
    case 1:
      gb.display.fillRect(gb.display.width() - 1 - gb.display.fontWidth, y + 1, gb.display.fontWidth, gb.display.fontHeight );
      break;
    case 2:
      gb.display.fillCircle(gb.display.width() - 3 - gb.display.fontWidth/2, y + 1 +  gb.display.fontHeight/2 -1 , gb.display.fontWidth/2);
      break;
    case -1:
      gb.display.drawRect(gb.display.width() - 1 - gb.display.fontWidth, y + 1, gb.display.fontWidth, gb.display.fontHeight );
      break;
    case -2:
      gb.display.drawCircle(gb.display.width() - 3 - gb.display.fontWidth/2, y + 1 +  gb.display.fontHeight/2 -1 , gb.display.fontWidth/2);
      break;
    }
  }


}

void myMenuDrawCursor(uint8_t i, int32_t y) {
  //	if ((gb.frameCount % 8) < 4) {
  //		return;
  //	}
  y += i*8 + menuYOffset;
  gb.display.setColor(BROWN);
  gb.display.drawRect(0, y, gb.display.width(), 9);
}

int16_t menu(const char* title, const char* const * items, int16_t * opts, uint8_t length, int16_t startpos) {
  bool reInitAsIndexed = false;
  if (gb.display.width() == 160) {
    reInitAsIndexed = true;
    gb.display.init(80, 64, ColorMode::rgb565);
  }
  uint8_t fontSizeBak = gb.display.fontSize;
  gb.display.setFontSize(1);
  
  int16_t cursor = 0;
  int32_t cameraY = 0;
  int32_t cameraY_actual = 0;

  for (uint8_t p=0; p < startpos; p++) {
    cursor++;
    if ((cursor*8 + cameraY + menuYOffset) > 54) {
      cameraY -= 8;
    }
    if (cursor >= length) {
      cursor = 0;
      cameraY = 0;
    }
  }
  
  while(1) {
    while(!gb.update());
    gb.display.clear();
    
    cameraY_actual = (cameraY_actual + cameraY) / 2;
    if (cameraY_actual - cameraY == 1) {
      cameraY_actual = cameraY;
    }
    
    for (uint8_t i = 0; i < length; i++) {
      int16_t optval = (opts ? opts[i] : 0);
      myMenuDrawBox(items[i], i, cameraY_actual, optval);
    }
    
    myMenuDrawCursor(cursor, cameraY_actual);
    
    // last draw the top entry thing
    gb.display.setColor(DARKGRAY);
    gb.display.fillRect(0, 0, gb.display.width(), 7);
    gb.display.setColor(BLUE);
    gb.display.setCursor(1, 1);
    gb.display.print(title);
    gb.display.setColor(BLACK);
    gb.display.drawFastHLine(0, 7, gb.display.width());
    
    if (gb.buttons.released(BUTTON_A)) {
      gb.sound.playOK();
      if (opts && opts[cursor]) {
	if (opts[cursor] != 2) 
	  opts[cursor] = -opts[cursor];
	if (opts[cursor] == 2)
	  for (byte i = 0; i < length; i++)
	    if (i != cursor && opts[i] == 2) opts[i] = -2;
	continue;
      }
      break;
    }

    if (gb.buttons.released(BUTTON_MENU)) {
      gb.sound.playCancel();
      state = RESET_STATE;
      return -1;
    }

    if (gb.buttons.repeat(BUTTON_UP, 4)) {
      if (cursor == 0) {
	cursor = length - 1;
	if (length > 6) {
	  cameraY = -(cursor-5)*8;
	}
      } else {
	cursor--;
	if (cursor > 0 && (cursor*8 + cameraY + menuYOffset) < 14) {
	  cameraY += 8;
	}
      }
      gb.sound.playTick();
    }
    
    if (gb.buttons.repeat(BUTTON_DOWN, 4)) {
      cursor++;
      if ((cursor*8 + cameraY + menuYOffset) > 54) {
	cameraY -= 8;
      }
      if (cursor >= length) {
	cursor = 0;
	cameraY = 0;
      }
      gb.sound.playTick();
    }
  }
  if (reInitAsIndexed) {
    gb.display.init(160, 128, ColorMode::index);
  }
  gb.display.setFontSize(fontSizeBak);
  return cursor;
}

