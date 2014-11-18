#include "LSlave.h"




/***********************************************************
 *                       PROPERTIES                        *
 **********************************************************/




LCDCHAR(lcdchar_voidBar)    = LCDCHAR_VOIDBAR_CONTENT;
LCDCHAR(lcdchar_halfBar)    = LCDCHAR_HALFBAR_CONTENT;
LCDCHAR(lcdchar_fullBar)    = LCDCHAR_FULLBAR_CONTENT;
LCDCHAR(lcdchar_leftBar)    = LCDCHAR_LEFTBAR_CONTENT;
LCDCHAR(lcdchar_rightBar)   = LCDCHAR_RIGHTBAR_CONTENT;
LCDCHAR(lcdchar_noLeftBar)  = LCDCHAR_NOLEFTBAR_CONTENT;
LCDCHAR(lcdchar_noRightBar) = LCDCHAR_NORIGHTBAR_CONTENT;

const uint8_t LSlave::index_info     = 0;
const uint8_t LSlave::index_message  = LSlave::index_info      + (true && Core::messages_len);
const uint8_t LSlave::index_pulse    = LSlave::index_message   + (true && Core::pulses_len);
const uint8_t LSlave::index_digital  = LSlave::index_pulse     + (true && Core::digitals_len);
const uint8_t LSlave::menu_len       = LSlave::index_digital   + 1;
LiquidCrystal LSlave::_lcd(LCD_PINS/*, LCD_BLPIN, LCD_BLPOLARITY*/);

LSlave::Key    LSlave::_key   = KEYPAD_NONE;
LSlave::State  LSlave::_state = SLEEPING;
uint8_t LSlave::_menuType      = LSlave::index_info;
uint8_t LSlave::_menuIndex     = 0;
uint8_t LSlave::_menuMax       = LSlave::menu_len;




/***********************************************************
 *                         PUBLIC                          *
 **********************************************************/




void LSlave::begin()
{
  //LSlave::_lcd.on();
  LSlave::_lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  LSlave::_lcd.createChar(LCDCHAR_VOIDBAR    , lcdchar_voidBar     );
  LSlave::_lcd.createChar(LCDCHAR_HALFBAR    , lcdchar_halfBar     );
  LSlave::_lcd.createChar(LCDCHAR_FULLBAR    , lcdchar_fullBar     );
  LSlave::_lcd.createChar(LCDCHAR_LEFTBAR    , lcdchar_leftBar     );
  LSlave::_lcd.createChar(LCDCHAR_RIGHTBAR   , lcdchar_rightBar    );
  LSlave::_lcd.createChar(LCDCHAR_NOLEFTBAR  , lcdchar_noLeftBar   );
  LSlave::_lcd.createChar(LCDCHAR_NORIGHTBAR , lcdchar_noRightBar  );
  LSlave::_lcd.home();
  LOGLN("display LCD");
}


void LSlave::check()
{
  if (LSlave::hasNewPulsedKey() && LSlave::_key != KEYPAD_NONE) {
    LOGLN(">>> LCD");

    // wakeup
    if (LSlave::_state != AWAKE) {
      LSlave::_state = AWAKE;
      //_lcd.on();
    } else {

      uint8_t deltaValue = 0;

      // UP/DOWN: page select
      // LEFT/RIGHT: change value
      // SELECT: switch between INFO, MESSAGES, PULSES, DIGITALS
      switch (LSlave::_key) {
        case KEYPAD_UP:
        LSlave::jump(-1);
        break;
        case KEYPAD_DOWN:
        LSlave::jump(+1);
        break;
        case KEYPAD_LEFT:
        deltaValue = +ANALOGSTEP;
        break;
        case KEYPAD_RIGHT:
        deltaValue = -ANALOGSTEP;
        break;
        case KEYPAD_SELECT:
        // jump section
        LSlave::switchMenu();
        break;
      }

      //LSlave::_lcd.home();
      LSlave::_lcd.clear();
      if (LSlave::_menuType==LSlave::index_info) {
        LSlave::printInfo();
        goto _switchEnd;
      }
      if (LSlave::_menuType==LSlave::index_message) {
        LSlave::printMessage();
        goto _switchEnd;
      }
      if (LSlave::_menuType==LSlave::index_pulse) {
        if (deltaValue) {
          Core::pulses[_menuIndex].addValue(deltaValue);
        }
        LSlave::printPulse();
        goto _switchEnd;
      }
      if (LSlave::_menuType==LSlave::index_digital) {
        if (deltaValue) {
          Core::digitals[_menuIndex].setValue(deltaValue>0);
        }
        LSlave::printDigital();
        goto _switchEnd;
      }
      _switchEnd:
      ;
    }

    LOGLN("<<< LCD");
  }
}


void LSlave::idle()
{
  if (LSlave::_state != SHUTDOWN) {
    switch (LSlave::_state) {
      case AWAKE:
      //LSlave::_lcd.setBacklight(LCD_BACKLIGHT_SLEEP);
      LSlave::_state = SLEEPING;
      break;
      default:
      //case SLEEPING:
      //LSlave::_lcd.off();
      //LSlave::_menuType = 0;
      LSlave::_menuType  = LSlave::index_info;
      LSlave::_menuIndex = 0;
      LSlave::_menuMax   = NUMBEROFMENU_HOME;
      //break;
    }
  }
}


void LSlave::printInfo()
{
  LOG("item I"); LOGLN(_menuType);
  LSlave::printTitle_P(PSTR(DEVICE_NAME));
  LSlave::_lcd.setCursor(0, 1);
  //#if USE_ETH
  LSlave::_lcd.print(Ethernet.localIP());
  /*
  #else
  LSlave::_lcd.print("--:--:--");
  #endif USE_ETH
  */
}




/***********************************************************
 *                        PROTECTED                        *
 **********************************************************/




void LSlave::printTitle_P(const prog_char* const label)
{
  uint8_t i = 0;
  LSlave::_lcd.setCursor(LCDPOSITION_TITLE_XY);
  while (i<LCDPOSITION_TITLE_LENGTH && pgm_read_byte_near(&label[i])) {
    LSlave::_lcd.print((char)pgm_read_byte_near(&label[i]));
    i++;
  }
  while (i++<LCDPOSITION_TITLE_LENGTH + LCDPOSITION_PAGE_OFFSET) {
    LSlave::_lcd.moveCursorRight();
  }
  if (LSlave::_menuType<9) {
    LSlave::_lcd.moveCursorRight();
  }
  LOG("page #"); LOG(_menuType+1); LOG('/'); LOGLN(_menuMax);
  LSlave::_lcd.print(LSlave::_menuType+1);
  LSlave::_lcd.print(LCDCHAR_PAGESEPARATOR);
  LSlave::_lcd.print(LSlave::_menuMax);
}


void LSlave::printMessage()
{
  LOG("item M"); LOGLN(_menuIndex);
  //LSlave::printTitle_P(Core::messages[LSlave::_menuIndex].getLabel());
  LSlave::_lcd.setCursor(0, 1);
  LSlave::_lcd.write("TODO"); // TODO
}


void LSlave::printPulse()
{
  LOG("item P"); LOGLN(_menuIndex);
  LSlave::printTitle_P(Core::pulses[LSlave::_menuIndex].getLabel());
  LSlave::_lcd.setCursor(LCDPOSITION_ANALOG_X, 1);
  LSlave::_lcd.write(LCDCHAR_LEFTBAR);
  uint8_t valueLeft = Core::pulses[LSlave::_menuIndex].getValue();
  for (uint8_t i=0; i<LCDPOSITION_BAR_LENGTH; i++) {
    if (valueLeft > 2* ANALOGSTEP) {
      valueLeft-= 2* ANALOGSTEP;
      LSlave::_lcd.write(LCDCHAR_FULLBAR);
    } else if (valueLeft < ANALOGSTEP) {
      LSlave::_lcd.write(LCDCHAR_VOIDBAR);
    } else {
      valueLeft = 0;
      LSlave::_lcd.write(LCDCHAR_HALFBAR);
    }
  }
  LSlave::_lcd.write(LCDCHAR_RIGHTBAR);
  for (uint8_t i=0; i<LCDPOSITION_ANALOG_OFFSET; i++) {
    LSlave::_lcd.moveCursorRight();
  }
  LSlave::_lcd.print(Core::pulses[LSlave::_menuIndex].getValue());
}


void LSlave::printDigital()
{
  char on, off;
  LOG("item D"); LOGLN(_menuIndex);
  LSlave::printTitle_P(Core::digitals[LSlave::_menuIndex].getLabel());
  LSlave::_lcd.setCursor(LCDPOSITION_DIGITAL_X, 1);
  if (Core::digitals[LSlave::_menuIndex].getValue()) {
    on = '*';
    off = ' ';
  } else {
    on = ' ';
    off = '*';
  }
  LSlave::_lcd.write(on);
  LSlave::_lcd.write("ON");
  LSlave::_lcd.write(on);
  for (uint8_t i=0; i<LCDPOSITION_DIGITAL_INSET; i++) {
    LSlave::_lcd.moveCursorRight();
  }
  LSlave::_lcd.write(off);
  LSlave::_lcd.write("OFF");
  LSlave::_lcd.write(off);
}


const boolean LSlave::hasNewPulsedKey()
{
  Key currentKey = LSlave::getKey();
  if (LSlave::_key != currentKey) {
    LSlave::_key = currentKey;
    return true;
  }
  return false;
}


const LSlave::Key LSlave::getKey()
{
  int input = analogRead(LCD_ABTN);
  if (input > KEYPAD_MAXVALUE_SELECT) return KEYPAD_NONE  ;
  if (input > KEYPAD_MAXVALUE_LEFT  ) return KEYPAD_SELECT;
  if (input > KEYPAD_MAXVALUE_DOWN  ) return KEYPAD_LEFT  ;
  if (input > KEYPAD_MAXVALUE_UP    ) return KEYPAD_DOWN  ;
  if (input > KEYPAD_MAXVALUE_RIGHT ) return KEYPAD_UP    ;
  return KEYPAD_RIGHT ;
  /*
  if (input < KEYPAD_MAXVALUE_RIGHT ) return KEYPAD_RIGHT ;
  if (input < KEYPAD_MAXVALUE_UP    ) return KEYPAD_UP    ;
  if (input < KEYPAD_MAXVALUE_DOWN  ) return KEYPAD_DOWN  ;
  if (input < KEYPAD_MAXVALUE_LEFT  ) return KEYPAD_LEFT  ;
  if (input < KEYPAD_MAXVALUE_SELECT) return KEYPAD_SELECT;
  return KEYPAD_NONE;
  */
}


void LSlave::switchMenu()
{
  LSlave::_menuIndex = 0;
  LSlave::_menuType = (LSlave::_menuType+1) % LSlave::menu_len;
  if (LSlave::_menuType==LSlave::index_info) {
    LSlave::_menuMax = NUMBEROFMENU_HOME;
    goto _switchEnd;
  }
  if (LSlave::_menuType==LSlave::index_message) {
    LSlave::_menuMax = Core::messages_len;
    goto _switchEnd;
  }
  if (LSlave::_menuType==LSlave::index_pulse) {
    LSlave::_menuMax = Core::pulses_len;
    goto _switchEnd;
  }
  if (LSlave::_menuType==LSlave::index_digital) {
    LSlave::_menuMax = Core::digitals_len;
    goto _switchEnd;
  }
  _switchEnd:
  ;
}
