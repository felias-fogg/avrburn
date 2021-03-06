// -*- c++ -*-


// Parts of the code below has been taken from the Gambuino Meta library, which has been publishe dunder LGPL

const uint8_t menuYOffset = 9;

void myMenuDrawBox(const char* text, uint16_t i, int32_t y, int8_t optval, boolean danger) {
  char itemline[21];
  y += i*8 + menuYOffset;
  if (y < 0 || y > 64) {
    return;
  }
  strncpy(itemline, text, 18);
  itemline[18] = '\0';
  gb.display.setColor(DARKGRAY);
  gb.display.fillRect(1, y+1, gb.display.width()-2, 7);
  if (danger) gb.display.setColor(GRAY);
  else gb.display.setColor(WHITE);
  gb.display.setCursor(2, y+2);
  gb.display.print(itemline);
  if (optval) {
    switch (optval) {
    case 1:
      gb.display.fillRect(gb.display.width() - 1 - gb.display.fontWidth, y + 1, gb.display.fontWidth, gb.display.fontHeight );
      break;
    case 2:
      gb.display.fillCircle(gb.display.width() - 2 - gb.display.fontWidth/2, y + 1 +  gb.display.fontHeight/2 -1 , gb.display.fontWidth/2);
      break;
    case -1:
      gb.display.drawRect(gb.display.width() - 1 - gb.display.fontWidth, y + 1, gb.display.fontWidth, gb.display.fontHeight );
      break;
    case -2:
      gb.display.drawCircle(gb.display.width() - 2 - gb.display.fontWidth/2, y + 1 +  gb.display.fontHeight/2 -1 , gb.display.fontWidth/2);
      break;
    }
  }
}

void myMenuDrawCursor(uint16_t i, int32_t y) {
  //	if ((gb.frameCount % 8) < 4) {
  //		return;
  //	}
  y += i*8 + menuYOffset;
  gb.display.setColor(BROWN);
  gb.display.drawRect(0, y, gb.display.width(), 9);
}

int16_t menu(const char* title, const char* const * items, int16_t * opts, uint16_t length, int16_t startpos) {
  
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
    gb.display.setTextWrap(false);

    cameraY_actual = (cameraY_actual + cameraY) / 2;
    if (cameraY_actual - cameraY == 1) {
      cameraY_actual = cameraY;
    }
    
    for (uint8_t i = 0; i < length; i++) {
      int8_t optval = (opts ? opts[i] : 0);
      myMenuDrawBox(items[i], i, cameraY_actual, optval, false);
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
	  for (uint16_t i = 0; i < length; i++)
	    if (i != cursor && opts[i] == 2) opts[i] = -2;
	continue;
      }
      break;
    }

    if (gb.buttons.released(BUTTON_B)) {
      gb.sound.playTick();
      return -1;
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
  return cursor;
}


uint32_t fuses_edit_menu(const char title[], uint8_t kind, fuseItems const list[], uint32_t initial)
{
  
  int16_t cursor = 0;
  int32_t cameraY = 0;
  int32_t cameraY_actual = 0;
  uint8_t buf[4];
  uint16_t length = 0;
  uint16_t index[MAX_FUSE_PROPS];

  // select the right entries
  for (uint16_t i=0; i < MAX_FUSE_PROPS; i++) {
    if (kind == FUSE_KIND) {
      if (list[i].addr <= 2) 
	index[length++] = i;
    } else { // LOCKBITS_KIND
      if (list[i].addr == 3 || list[i].mask == 0)
	index[length++] = i;
    } 
    if (list[i].mask == 0) break;
  }

  DEBPR("fuses_edit_menu: length=");
  DEBLN(length);
  
  for (uint8_t i=0; i<4; i++) {
    buf[i] = initial & 0xFF;
    initial >>= 8;
  }
  while(1) {
    while(!gb.update());
    gb.display.clear();
    gb.display.setTextWrap(false);

    cameraY_actual = (cameraY_actual + cameraY) / 2;
    if (cameraY_actual - cameraY == 1) {
      cameraY_actual = cameraY;
    }
    
    for (uint16_t i = 0; i < length; i++) 
      myMenuDrawBox(fuseProps[list[index[i]].mes], i, cameraY_actual, optval(list, length, i, index, buf),list[index[i]].danger && safeedit);
    
    myMenuDrawCursor(cursor, cameraY_actual);
    
    // last draw the top entry thing
    gb.display.setColor(DARKGRAY);
    gb.display.fillRect(0, 0, gb.display.width(), 7);
    gb.display.setColor(GREEN);
    gb.display.setCursor(1, 1);
    gb.display.print(title);
    gb.display.setColor(BLACK);
    gb.display.drawFastHLine(0, 7, gb.display.width());

    if (gb.buttons.released(BUTTON_A)) {
      if (list[index[cursor]].mask == 0) {
	gb.sound.playOK();
      	return new_value(buf);
      }
      DEBPR("selected entry: ");
      DEBLN(cursor);
      DEBLN(length);
      if (change_value(list, length, cursor, index, buf) && !error) gb.sound.playOK();
    }

    if (gb.buttons.released(BUTTON_B)) {
      gb.sound.playTick();
      return new_value(buf);
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
  return -1;
}

int8_t optval(fuseItems const list[], uint16_t length, uint16_t ix, uint16_t index[], uint8_t buf[4])
{
  int8_t marker = 1;
  if (list[index[ix]].mask == 0) return 0;
  for (uint16_t i=0; i < length; i++)
    if (i != ix && list[index[i]].mask == list[index[ix]].mask && list[index[i]].addr == list[index[ix]].addr) marker = 2;
  if ((list[index[ix]].mask & buf[list[index[ix]].addr]) != list[index[ix]].value) marker = -marker;
  return marker;
}

boolean change_value(fuseItems const list[], uint16_t length, uint16_t ix, uint16_t index[], uint8_t buf[4])
{
  if (list[index[ix]].danger && safeedit) {
    return false;
  }
  int8_t marker = optval(list, length, ix, index, buf);
  DEBPR("change_value: marker=");
  DEBPR(marker);
  DEBPR(" mask=");
  DEBPRF(list[index[ix]].mask,HEX);
  DEBPR(" value=");
  DEBLNF(list[index[ix]].value,HEX);
  DEBPR(" before: ");
  DEBLNF(buf[list[index[ix]].addr],HEX);
  switch (marker) {
  case 2: break;
  case -2: buf[list[index[ix]].addr] = (buf[list[index[ix]].addr] & ~list[index[ix]].mask) | list[index[ix]].value; break;
  case 1: buf[list[index[ix]].addr] |= list[index[ix]].mask; break;
  case -1: buf[list[index[ix]].addr] &= ~list[index[ix]].mask; break;
  default: error = CONFUSION_ERROR; break;
  }
  DEBPR(" after: ");
  DEBLNF(buf[list[index[ix]].addr],HEX);
  return true;
}


uint32_t new_value(uint8_t buf[4])
{
  return (uint32_t)((buf[3]<<24)+(buf[2]<<16)+(buf[1]<<8)+buf[0]);
}


// returns 1, 0, -1 based on whether left, mid, or right alternative has been chosen
// id mid == NULL, then only twoalternatives are shown.
int8_t left_mid_or_right(const char * title, const char * question, const char * left, const char * mid, const char * right, int8_t pos)
{
  int leftx, leftwidth, midx, midwidth, rightx, rightwidth;
  if (mid == NULL) {
    leftwidth = (strlen(left)*gb.display.getFontWidth());
    leftx = (gb.display.width()/2-leftwidth)/2;
    rightwidth = (strlen(right)*gb.display.getFontWidth());
    rightx = gb.display.width()/2+(gb.display.width()/2-rightwidth)/2;
  } else {
    leftwidth = (strlen(left)*gb.display.getFontWidth());
    leftx = (gb.display.width()/3-leftwidth)/2;
    midwidth = (strlen(left)*gb.display.getFontWidth());
    midx = gb.display.width()/3+(gb.display.width()/3-midwidth)/2;
    rightwidth = (strlen(right)*gb.display.getFontWidth());
    rightx = 2*gb.display.width()/3+(gb.display.width()/3-rightwidth)/2;
  }

  while (1) {
    while(!gb.update());
    gb.display.clear();
    gb.display.setTextWrap(false);

    gb.display.setColor(DARKGRAY);
    gb.display.fillRect(0, 0, gb.display.width(), 7);
    gb.display.setColor(RED);
    gb.display.setCursor(1, 1);
    gb.display.print(title);
    gb.display.setColor(BLACK);
    gb.display.drawFastHLine(0, 7, gb.display.width());
    gb.display.setColor(WHITE);

    gb.display.setCursor(0,20);
    gb.display.print(question);

    gb.display.setColor(DARKGRAY);
    gb.display.fillRect(leftx, 50, leftwidth, gb.display.getFontHeight());
    gb.display.fillRect(rightx, 50, rightwidth, gb.display.getFontHeight());
    if (mid != NULL)
      gb.display.fillRect(midx, 50, midwidth, gb.display.getFontHeight());

    gb.display.setColor(RED);
    gb.display.setCursor(leftx,50);
    gb.display.print(left);
    gb.display.setCursor(rightx,50);
    gb.display.print(right);
    if (mid != NULL) {
      gb.display.setCursor(midx,50);
      gb.display.print(mid);
    }

    gb.display.setColor(BROWN);
    switch (pos) {
    case 1:  gb.display.drawRect(leftx-1, 49, leftwidth+2, gb.display.getFontHeight()+2); break;
    case 0:  gb.display.drawRect(midx-1, 49, midwidth+2, gb.display.getFontHeight()+2); break;
    case -1: gb.display.drawRect(rightx-1, 49, rightwidth+2, gb.display.getFontHeight()+2); break;
    }
    
    if (check_OK()) return pos;
    
    if (gb.buttons.released(BUTTON_LEFT) && pos < 1) {
      pos++;
      if (pos == 0 && mid == NULL) pos++;
    }
    if (gb.buttons.released(BUTTON_RIGHT) && pos > -1) {
      pos--;
      if (pos == 0 && mid == NULL) pos--;
    }
  }
}

boolean left_or_right(const char * title, const char * question, const char * left,  const char * right, boolean leftpos)
{
  return (left_mid_or_right(title, question, left, NULL, right, (leftpos ? 1 : -1)) == 1);
}

// Keyboard copied from Gambuino Meta

const char keyboardLayoutPage0[] = "1234567890qwertyuiop[]*asdfghjkl;'\\*zxcvbnm,./-=*AB";
const char keyboardLayoutPage1[] = "!@*$%^&*()QWERTYUIOP{}*ASDFGHJKL:\"|*ZXCVBNM<>?_+*ab";
const char* keyboardLayout[] = {
	keyboardLayoutPage0,
	keyboardLayoutPage1,
};
const uint8_t keyboardLayoutPages = 2;


const uint8_t keyboardYOffset = 23;

void keyboardDrawKey(uint8_t x, uint8_t y, char c) {
	gb.display.setColor(DARKGRAY);
	gb.display.fillRect(x+1, y+1, 5, 7);
	gb.display.setColor(WHITE);
	gb.display.setCursor(x+2, y+2);
	gb.display.print(c);
}

void keyboardDrawLayout(const char* layout) {
	for (uint8_t y = 0; y < 4; y++) {
		for (uint8_t x = 0; x < 13; x++) {
			if (y == 0 && x >= 10) {
				// we skip the backspace key
				break;
			}
			keyboardDrawKey(x*6, y*8 + keyboardYOffset, *(layout++));
		}
	}
	// last we draw the new layout switch
	gb.display.setColor(DARKGRAY);
	gb.display.fillRect(7, keyboardYOffset + 1 + 4*8, 11, 7);
	gb.display.setColor(GRAY);
	gb.display.setCursor(9, keyboardYOffset + 2 + 4*8);
	gb.display.print(layout);
}

void keyboardDrawBackspace() {
	const uint8_t arrow[] = {
		8, 5,
		0x20, 0x40, 0xFF, 0x40, 0x20,
	};
	gb.display.setColor(DARKGRAY);
	gb.display.fillRect(6*10+1, keyboardYOffset + 1, 17, 7);
	gb.display.setColor(GRAY);
	gb.display.drawBitmap(6*10+9, keyboardYOffset+2, arrow);
}

void keyboardDrawSwitch() {
	const uint8_t menu[] = {
		8, 5,
		0xB8, 0x00, 0xB8, 0x00, 0xB8,
	};
	gb.display.setColor(DARKGRAY);
	gb.display.fillRect(1, keyboardYOffset + 1 + 4*8, 17, 7);
	gb.display.setColor(GRAY);
	gb.display.drawBitmap(2, keyboardYOffset + 2 + 4*8, menu);
}

void keyboardDrawSelect() {
	const uint8_t checkmark[] = {
		8, 4,
		0x08, 0x10, 0xA0, 0x40,
	};
	gb.display.setColor(DARKGRAY);
	gb.display.fillRect(6*10+1, keyboardYOffset + 1 + 4*8, 17, 7);
	gb.display.setColor(LIGHTGREEN);
	gb.display.drawBitmap(6*10+7, keyboardYOffset + 3 + 4*8, checkmark);
}

void keyboardDrawSpace() {
	gb.display.setColor(DARKGRAY);
	gb.display.fillRect(19, keyboardYOffset + 1 + 4*8, 41, 7);
}

void keyboardDrawCursorReal(uint8_t x, int8_t y) {
	if (y == 0 && x >= 10) {
		// blinking backspace
		gb.display.drawRect(10*6, keyboardYOffset, 19, 9);
		return;
	}
	if (y == 4) {
		// lower row
		if (x < 3) {
			// switch
			gb.display.drawRect(0, keyboardYOffset + 4*8, 19, 9);
			return;
		}
		if (x < 10) {
			// space
			gb.display.drawRect(3*6, keyboardYOffset + 4*8, 43, 9);
			return;
		}
		// select
		gb.display.drawRect(10*6, keyboardYOffset + 4*8, 19, 9);
		return;
	}
	gb.display.drawRect(x*6, y*8 + keyboardYOffset, 7, 9);
}

void keyboardDrawCursor(int8_t x, int8_t y) {
	gb.display.setColor((gb.frameCount % 8) >= 4 ? BROWN : BLACK);
	keyboardDrawCursorReal(x, y);
}

void keyboardEraseCursor(int8_t x, int8_t y) {
	gb.display.setColor(BLACK);
	keyboardDrawCursorReal(x, y);
}

char keyboardGetChar(int8_t x, int8_t y, const char* layout) {
	if (y == 4) {
		if (x < 10 && x >= 3) {
			return ' ';
		}
		return 0;
	}
	if (y == 0 && x >= 10) {
		return 0;
	}
	if (y > 0) { // skip the backspace button
		x -= 3;
	}
	return layout[13*y + x];
}

void keyboard(const char* title, char* text, uint8_t length) {
	
	gb.display.fill(BLACK);
	gb.display.setColor(DARKGRAY);
	gb.display.fillRect(0, 0, gb.display.width(), 7);
	gb.display.setCursor(1, 1);
	gb.display.setColor(WHITE);
	gb.display.print(title);
	
	
	keyboardDrawBackspace();
	keyboardDrawSwitch();
	keyboardDrawSpace();
	keyboardDrawSelect();
	
	uint8_t curLayout = 0;
	
	keyboardDrawLayout(keyboardLayout[curLayout]);
	
	int8_t cursorX = 6;
	int8_t cursorY = 2;
	int8_t activeChar = 0;
	while(1) {
		while(!gb.update());
		// update cursor movement
		int8_t cursorXPrev = cursorX;
		int8_t cursorYPrev = cursorY;
		keyboardEraseCursor(cursorX, cursorY);
		// cursorX movement
		if (gb.buttons.repeat(BUTTON_LEFT, 4)) {
			if ((cursorY == 0 || cursorY == 4) && cursorX >= 10) {
				cursorX = 9;
			} else if (cursorY == 4 && cursorX >= 3) {
				cursorX = 2;
			} else if (cursorX == 0 || (cursorX <= 2 && cursorY == 4)) {
				cursorX = 12;  // Wrap around
			} else {
				cursorX--;
			}
		}
		if (gb.buttons.repeat(BUTTON_RIGHT, 4)) {
			if (cursorY == 4 && cursorX < 3) {
				cursorX = 3;
			} else if (cursorY == 4 && cursorX < 10) {
				cursorX = 10;
			} else if (cursorX == 12 || (cursorX >=10 && (cursorY == 0 || cursorY == 4))) {
				cursorX = 0;  // Wrap around
			} else {
				cursorX++;
			}
		}
		// cursorY movement
		cursorY += gb.buttons.repeat(BUTTON_DOWN, 4) - gb.buttons.repeat(BUTTON_UP, 4);
		if (cursorY > 4) cursorY = 0;
		else if (cursorY < 0) cursorY = 4;

		keyboardDrawCursor(cursorX, cursorY);
		if (cursorX != cursorXPrev || cursorY != cursorYPrev) {
			gb.sound.playTick();
		}
		bool backspace = false;

		// check for other button presses
		if (gb.buttons.released(BUTTON_B)) {
		  text[0] = '\0';
		  return;
		}
		bool switchLayout = gb.buttons.released(BUTTON_MENU);
		if (gb.buttons.released(BUTTON_A)) {
			char c = keyboardGetChar(cursorX, cursorY, keyboardLayout[curLayout]);
			if (!c) {
				// other handling
				if (cursorY == 0) {
					backspace = true;
				} else if (cursorY == 4 && cursorX < 3) {
					switchLayout = true;
				} else {
					// we are done!
					gb.sound.playOK();
					break;
				}
			} else if (activeChar < length) {
				text[activeChar++] = c;
				text[activeChar] = '\0';
				gb.sound.playOK();
			}
		}
		if (backspace) {
			if (activeChar > 0) {
				text[--activeChar] = '\0';
				gb.sound.playCancel();
			}
		}
		if (switchLayout) {
			if (++curLayout >= keyboardLayoutPages) {
				curLayout = 0;
			}
			keyboardDrawLayout(keyboardLayout[curLayout]);
			gb.sound.playOK();
		}
		
		// render drawing text
		gb.display.setColor(BLACK);
		gb.display.fillRect(0, 7, gb.display.width(), 15);
		gb.display.setColor(activeChar?WHITE:DARKGRAY);
		gb.display.setCursor(1, 13);
		gb.display.print(text);
		
		gb.display.setColor(WHITE);
		gb.display.drawFastHLine(activeChar*4 + 1, 19, 3);
	}
}
