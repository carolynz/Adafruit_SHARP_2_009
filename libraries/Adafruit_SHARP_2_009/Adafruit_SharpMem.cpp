/*********************************************************************
This is an Arduino library for our Monochrome SHARP Memory Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1393

These displays use SPI to communicate, 3 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include "Adafruit_SharpMem.h"

/**************************************************************************
    Sharp Memory Display Connector
    -----------------------------------------------------------------------
    Pin   Function        Notes
    ===   ==============  ===============================
      1   VIN             3.3-5.0V (into LDO supply)
      2   3V3             3.3V out
      3   GND
      4   SCLK            Serial Clock
      5   MOSI            Serial Data Input
      6   CS              Serial Chip Select
      9   EXTMODE         COM Inversion Select (Low = SW clock/serial)
      7   EXTCOMIN        External COM Inversion Signal
      8   DISP            Display On(High)/Off(Low)

 **************************************************************************/

#define SHARPMEM_BIT_WRITECMD   (0x80)
#define SHARPMEM_BIT_VCOM       (0x40)
#define SHARPMEM_BIT_CLEAR      (0x20)
#define TOGGLE_VCOM             do { _sharpmem_vcom = _sharpmem_vcom ? 0x00 : SHARPMEM_BIT_VCOM; } while(0);

byte sectionBuffer[(SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8];
uint16_t BLACK = 0;
uint16_t WHITE = 1;

/* ************* */
/* CONSTRUCTORS  */
/* ************* */
Adafruit_SharpMem::Adafruit_SharpMem(uint8_t clk, uint8_t mosi, uint8_t ss) :
Adafruit_GFX(SHARPMEM_LCDWIDTH, SHARPMEM_LCDHEIGHT) {
  _clk = clk;
  _mosi = mosi;
  _ss = ss;

  // Set pin state before direction to make sure they start this way (no glitching)
  digitalWrite(_ss, HIGH);  
  digitalWrite(_clk, LOW);  
  digitalWrite(_mosi, HIGH);  
  
  pinMode(_ss, OUTPUT);
  pinMode(_clk, OUTPUT);
  pinMode(_mosi, OUTPUT);
  
  clkport     = portOutputRegister(digitalPinToPort(_clk));
  clkpinmask  = digitalPinToBitMask(_clk);
  dataport    = portOutputRegister(digitalPinToPort(_mosi));
  datapinmask = digitalPinToBitMask(_mosi);
  
  // Set the vcom bit to a defined state
  _sharpmem_vcom = SHARPMEM_BIT_VCOM;

  // Set defaults for pace, break, units, etc.
  paceMin = 1;
  paceSec = 0;
  breakMin = 0;
  breakSec = 0;
  length = 25;
  depth = 3;
  imperial = true;

}

void Adafruit_SharpMem::begin() {
  setRotation(2);
}

/* *************** */
/* PRIVATE METHODS */
/* *************** */

/**************************************************************************/
/*!
    @brief  Sends a single byte in pseudo-SPI.
*/
/**************************************************************************/
void Adafruit_SharpMem::sendbyte(uint8_t data) 
{
  uint8_t i = 0;

  // LCD expects LSB first
  for (i=0; i<8; i++) 
  { 
    // Make sure clock starts low
    //digitalWrite(_clk, LOW);
    *clkport &= ~clkpinmask;
    if (data & 0x80) 
      //digitalWrite(_mosi, HIGH);
      *dataport |=  datapinmask;
    else 
      //digitalWrite(_mosi, LOW);
      *dataport &= ~datapinmask;

    // Clock is active high
    //digitalWrite(_clk, HIGH);
    *clkport |=  clkpinmask;
    data <<= 1; 
  }
  // Make sure clock ends low
  //digitalWrite(_clk, LOW);
  *clkport &= ~clkpinmask;
}

void Adafruit_SharpMem::sendbyteLSB(uint8_t data) 
{
  uint8_t i = 0;

  // LCD expects LSB first
  for (i=0; i<8; i++) 
  { 
    // Make sure clock starts low
    //digitalWrite(_clk, LOW);
    *clkport &= ~clkpinmask;
    if (data & 0x01) 
      //digitalWrite(_mosi, HIGH);
      *dataport |=  datapinmask;
    else 
      //digitalWrite(_mosi, LOW);
      *dataport &= ~datapinmask;
    // Clock is active high
    //digitalWrite(_clk, HIGH);
    *clkport |=  clkpinmask;
    data >>= 1; 
  }
  // Make sure clock ends low
  //digitalWrite(_clk, LOW);
  *clkport &= ~clkpinmask;
}


/* ************** */
/* PUBLIC METHODS */
/* ************** */

/**************************************************************************/
/*! 
    @brief Draws a single pixel in image buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)
*/
/**************************************************************************/
void Adafruit_SharpMem::drawPixel(int16_t x, int16_t y, uint16_t color) 
{
  if ((x >= SHARPMEM_LCDWIDTH) || (y >= SHARPMEM_LCDHEIGHT))
    return;
  // TODO: this code is weird for different sections that don't start at 1.
  // Make sure to increment sectionBuffer index appropriately.
  if (color)
    sectionBuffer[(y*SHARPMEM_LCDWIDTH + x) /8] |= (1 << x % 8);
  else
    sectionBuffer[(y*SHARPMEM_LCDWIDTH + x) /8] &= ~(1 << x % 8);
}

/**
* Fills buffer with the tab outlines & labels
* @modifies - sectionBuffer
*/
void Adafruit_SharpMem::drawTabs(void){
  int16_t x, y;
  y = 0;
  // clear buffer
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  // draw top line of tabs
  drawFastHLine(0, 0, 320, BLACK);

  //TODO: change x-coords for 5-tab layout
  drawChar(4, 13, 'P', BLACK, WHITE, 2);
  drawChar(16, 13, 'A', BLACK, WHITE, 2);
  drawChar(28, 13, 'C', BLACK, WHITE, 2);
  drawChar(40, 13, 'E', BLACK, WHITE, 2);

  drawChar(68, 13, 'B', BLACK, WHITE, 2);
  drawChar(80, 13, 'R', BLACK, WHITE, 2);
  drawChar(92, 13, 'E', BLACK, WHITE, 2);
  drawChar(104, 13, 'A', BLACK, WHITE, 2);
  drawChar(116, 13, 'K', BLACK, WHITE, 2);

  drawChar(135, 13, 'L', BLACK, WHITE, 2);
  drawChar(147, 13, 'E', BLACK, WHITE, 2);
  drawChar(159, 13, 'N', BLACK, WHITE, 2);
  
  drawChar(193, 13, 'D', BLACK, WHITE, 2);
  drawChar(205, 13, 'E', BLACK, WHITE, 2);
  drawChar(217, 13, 'P', BLACK, WHITE, 2);
  drawChar(229, 13, 'T', BLACK, WHITE, 2);
  drawChar(241, 13, 'H', BLACK, WHITE, 2);

  drawChar(258, 13, 'U', BLACK, WHITE, 2);
  drawChar(270, 13, 'N', BLACK, WHITE, 2);
  drawChar(282, 13, 'I', BLACK, WHITE, 2);
  drawChar(294, 13, 'T', BLACK, WHITE, 2);
  drawChar(306, 13, 'S', BLACK, WHITE, 2);


  drawFastVLine(0, 0, 40, BLACK);
  drawFastVLine(63, 0, 40, BLACK);
  drawFastVLine(127, 0, 40, BLACK);
  drawFastVLine(190, 0, 40, BLACK);
  drawFastVLine(255, 0, 40, BLACK);
  drawFastVLine(319, 0, 40, BLACK);
}

/*!
 Fills buffer with the settings that go under each tab
 @param position - unsigned 8-bit integer, possible values of 0, 1, 2, 3, 4:
                   0 - Pace menu position
                   1 - Break menu position
                   2 - Length menu position
                   3 - Depth menu position
                   4 - Units menu position
*/
void Adafruit_SharpMem::drawTabSettings(uint8_t position){
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);

  // draw divider lines
  drawFastVLine(0, 0, 40, BLACK);
  drawFastVLine(63, 0, 40, BLACK);
  drawFastVLine(127, 0, 40, BLACK);
  drawFastVLine(190, 0, 40, BLACK);
  drawFastVLine(255, 0, 40, BLACK);
  drawFastVLine(319, 0, 40, BLACK);

  // draw pace
  drawChar(4, 13, toUC(paceMin), BLACK, WHITE, 2);
  drawChar(16, 13, ':', BLACK, WHITE, 2);
  if (paceSec < 10){
    drawChar(28, 13, '0', BLACK, WHITE, 2);
    drawChar(40, 13, toUC(paceSec), BLACK, WHITE, 2);
  }else{
    drawChar(28, 13, toUC(paceSec/10), BLACK, WHITE, 2);
    drawChar(40, 13, toUC(paceSec%10), BLACK, WHITE, 2);
  }

  // draw break
  drawChar(65, 13, toUC(breakMin), BLACK, WHITE, 2);
  drawChar(77, 13, ':', BLACK, WHITE, 2);
  if (breakSec < 10){
    drawChar(89, 13, '0', BLACK, WHITE, 2);
    drawChar(101, 13, toUC(breakSec), BLACK, WHITE, 2);
  }else{
    drawChar(89, 13, toUC(breakSec/10), BLACK, WHITE, 2);
    drawChar(101, 13, toUC(breakSec%10), BLACK, WHITE, 2);
  }

  // draw len
  drawChar(131, 13, toUC(length/10), BLACK, WHITE, 2);
  drawChar(142, 13, toUC(length%10), BLACK, WHITE, 2);
  if (imperial){
    drawChar(154, 13, 'Y', BLACK, WHITE, 2);
    drawChar(166, 13, 'D', BLACK, WHITE, 2);
    drawChar(178, 13, 'S', BLACK, WHITE, 2);
  }else{
    drawChar(154, 13, 'M', BLACK, WHITE, 2);
  }

  // draw depth
  drawChar(192, 13, toUC(depth), BLACK, WHITE, 2);
  if (imperial){
    drawChar(205, 13, 'F', BLACK, WHITE, 2);
    drawChar(217, 13, 'T', BLACK, WHITE, 2);
  }else{
    drawChar(205, 13, 'M', BLACK, WHITE, 2);
  }

  // draw units
  if (imperial){
    drawChar(258, 13, 'Y', BLACK, WHITE, 2);
    drawChar(270, 13, 'D', BLACK, WHITE, 2);
    drawChar(282, 13, 'S', BLACK, WHITE, 2);
  }else{
    drawChar(258, 13, 'M', BLACK, WHITE, 2);
  }

  switch(position){
    // Pace
    case 0:
      drawFastHLine(SHARPMEM_LCDWIDTH/5, 0, SHARPMEM_LCDWIDTH, BLACK);
      drawFastHLine(SHARPMEM_LCDWIDTH/5, 39, SHARPMEM_LCDWIDTH, BLACK);
      break;
    // Break
    case 1:
      drawFastHLine(0, 0, SHARPMEM_LCDWIDTH/5, BLACK);
      drawFastHLine((2*SHARPMEM_LCDWIDTH/5), 0, SHARPMEM_LCDWIDTH, BLACK);

      drawFastHLine(0, 39, SHARPMEM_LCDWIDTH/5, BLACK);
      drawFastHLine((2*SHARPMEM_LCDWIDTH/5), 39, SHARPMEM_LCDWIDTH, BLACK);
      break;
    // Length
    case 2:
      drawFastHLine(0, 0, (2*SHARPMEM_LCDWIDTH/5), BLACK);
      drawFastHLine((3*SHARPMEM_LCDWIDTH/5), 0, SHARPMEM_LCDWIDTH, BLACK);

      drawFastHLine(0, 39, (2*SHARPMEM_LCDWIDTH/5), BLACK);
      drawFastHLine((3*SHARPMEM_LCDWIDTH/5), 39, SHARPMEM_LCDWIDTH, BLACK);
      break;
    // Depth
    case 3:
      drawFastHLine(0, 0, (3*SHARPMEM_LCDWIDTH/5), BLACK);
      drawFastHLine((4*SHARPMEM_LCDWIDTH/5), 0, SHARPMEM_LCDWIDTH, BLACK);

      drawFastHLine(0, 39, (2*SHARPMEM_LCDWIDTH/5), BLACK);
      drawFastHLine((3*SHARPMEM_LCDWIDTH/5), 39, SHARPMEM_LCDWIDTH, BLACK);
      break;
    // Units
    case 4:
      drawFastHLine(0, 0, (4*SHARPMEM_LCDWIDTH/5), BLACK);
      drawFastHLine(0, 39, (4*SHARPMEM_LCDWIDTH/5), BLACK);
      break;
    default: // if in swim/pause mode
      drawFastHLine(0, 0, SHARPMEM_LCDWIDTH, BLACK);
      drawFastHLine(0, 39, SHARPMEM_LCDWIDTH, BLACK);
      break;
  }
}

void Adafruit_SharpMem::drawTime(uint8_t min, uint8_t sec){
  // clear buffer
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  int16_t x = 54;
  
  // draw minute
  drawNum(x, 0, min);
  // move x over 64 pixels
  // TODO: change this for better kerning
  x += 64;

  // draw colon
  drawColon(x, 0);
  x += 21; // move x over 21 pixels (width of colon)

  //draw second
  if (sec < 10){
    drawNum(x, 0, 0); // first digit is 0
    x += 64;          // move x position over
    drawNum(x, 0, sec); // second digit is sec
  } else {
    drawNum(x, 0, sec / 10);  // first digit of sec
    x += 64;    // move x position over
    drawNum(x, 0, sec % 10);  // second digit of sec
  }
}

void Adafruit_SharpMem::drawColon(int16_t x, int16_t y){
  drawBitmap(x, y, colon, 20, 75, BLACK);
}

void Adafruit_SharpMem::drawNum(int16_t x, int16_t y, uint8_t num){
  switch (num){
    case 0:
      drawBitmap(x, y, zero, 63, 75, BLACK);
      break;
    case 1:
      drawBitmap(x, y, one, 55, 75, BLACK);
      break;
    case 2:
      drawBitmap(x, y, two, 57, 75, BLACK);
      break;
    case 3:
      drawBitmap(x, y, three, 57, 75, BLACK);
      break;
    case 4:
      drawBitmap(x, y, four, 63, 75, BLACK);
      break;
    case 5:
      drawBitmap(x, y, five, 61, 75, BLACK);
      break;
    case 6:
      drawBitmap(x, y, six, 64, 75, BLACK);
      break;
    case 7:
      drawBitmap(x, y, seven, 59, 75, BLACK);
      break;
    case 8:
      drawBitmap(x, y, eight, 58, 75, BLACK);
      break;
    case 9:
      drawBitmap(x, y, nine, 64, 75, BLACK);
      break;
    default:
      break;
  }
}
void Adafruit_SharpMem::drawDenominator(int16_t length, bool imperial){
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  drawFastHLine(54, 0, 212, BLACK);
  drawFastHLine(54, 1, 212, BLACK);

  drawChar(55, 10, '1', BLACK, WHITE, 3);
  drawChar(70, 10, '0', BLACK, WHITE, 3);
  drawChar(90, 10, '0', BLACK, WHITE, 3);

  drawChar(125, 10, 'Y', BLACK, WHITE, 3);
  drawChar(145, 10, 'D', BLACK, WHITE, 3);
  drawChar(165, 10, 'S', BLACK, WHITE, 3);
}

/**************************************************************************/
/*! 
    @brief Clears the screen
*/
/**************************************************************************/
void Adafruit_SharpMem::clearDisplay() 
{
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  // Send the clear screen command rather than doing a HW refresh (quicker)
  digitalWrite(_ss, HIGH);
  sendbyte(_sharpmem_vcom | SHARPMEM_BIT_CLEAR);
  sendbyteLSB(0x00);
  TOGGLE_VCOM;
  digitalWrite(_ss, LOW);
}

/**************************************************************************/
/*! 
    @brief Renders the contents of the pixel buffer on the LCD
*/
/**************************************************************************/
void Adafruit_SharpMem::refresh(uint8_t section){
  uint16_t i, totalbytes, currentline, oldline, startline;

  switch(section){
    case 0: // first section - Tabs
      startline = 1;
      totalbytes = (SHARPMEM_LCDWIDTH * 40) / 8;
      break;
    case 1: // second section - Tab line
      startline = 41;
      totalbytes = (SHARPMEM_LCDWIDTH * 40) / 8;
      break;
    case 2: // third section - set values under tabs
      startline = 42;
      totalbytes = (SHARPMEM_LCDWIDTH * 39) / 8;
      break;
    case 3: // fourth section - central panel (time set, units set, depth set)
      startline = 101;
      totalbytes = (SHARPMEM_LCDWIDTH * 75) / 8;
      break;
    case 4: // fifth section - denominator
      startline = 181;
      totalbytes = (SHARPMEM_LCDWIDTH * 40) / 8;
      break;
    default:
      startline = 1;
      totalbytes = (SHARPMEM_LCDWIDTH * 40) / 8;
      break;
  }

  digitalWrite(_ss, HIGH);
  sendbyte(SHARPMEM_BIT_WRITECMD | _sharpmem_vcom);
  TOGGLE_VCOM;

  oldline = currentline = startline;
  sendbyteLSB(currentline);

  for (i=0; i<totalbytes; i++){
    sendbyteLSB(sectionBuffer[i]);
    currentline = ((i+1)/(SHARPMEM_LCDWIDTH/8)) + startline;
    if(currentline != oldline){
      // Send end of line and address bytes
      sendbyteLSB(0x00);
      sendbyteLSB(currentline);
      oldline = currentline;
    }
  }
  // Send another trailing 8 bits for the last line
  sendbyteLSB(0x00);
  digitalWrite(_ss, LOW);
} 

void Adafruit_SharpMem::refreshTabs(void){
  refresh(0);
}

void Adafruit_SharpMem::refreshTabSettings(void){
  refresh(1);
}

void Adafruit_SharpMem::refreshSetValues(void){
  refresh(2);
}

void Adafruit_SharpMem::refreshCentral(void){
  refresh(3);
}

void Adafruit_SharpMem::refreshDenominator(void){
  refresh(4);
}

void Adafruit_SharpMem::renderTime(uint8_t min, uint8_t sec){
  drawTime(min, sec);
  refreshCentral();
}

/*! 
  Displays the Pace screen
  - uses stored values for the tab settings numbers and drawTime numbers
*/
void Adafruit_SharpMem::renderScreenPace(void){
  drawTabs();
  refreshTabs();

  drawTabSettings(0);
  refreshTabSettings();

  drawTime(paceMin, paceSec);    
  refreshCentral();

  drawDenominator();
  refreshDenominator();
}

/*! 
  Displays the Break screen
  - uses stored values for the tab settings numbers and drawTime numbers
*/
void Adafruit_SharpMem::renderScreenBreak(void){
  drawTabs();
  refreshTabs();  

  // draw pace tab line
  drawTabSettings(1);
  refreshTabSettings();

  drawTime(breakMin, breakSec);    
  refreshCentral();

  //TODO: draw the 100 yds
  drawDenominator();
  refreshDenominator();
}

void Adafruit_SharpMem::renderScreenLength(void){
  drawTabs();
  refreshTabs();  


  // draw length tab line
  drawTabSettings(2);
  refreshTabSettings();

  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  refreshCentral();
}


void Adafruit_SharpMem::renderScreenDepth(void){
  drawTabs();
  refreshTabs();  

  // draw depth tab line
  drawTabSettings(3);
  refreshTabSettings();

  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  refreshCentral();
}


void Adafruit_SharpMem::renderScreenUnits(void){
  drawTabs();
  refreshTabs();  

  // draw units tab line
  drawTabSettings(4);
  refreshTabSettings();

  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  refreshCentral();
}

void Adafruit_SharpMem::renderScreenSwim(uint8_t laps){
  drawTabs();
  refreshTabs();

  drawTabSettings(5);
  refreshTabSettings();

  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);

  if (laps < 10){
    drawNum(150, 0, laps);    
  } else {
    drawNum(120, 0, laps/10);
    drawNum(190, 0, laps%10);
  }
  refreshCentral();

  drawChar(55, 10, 'L', BLACK, WHITE, 3);
  drawChar(70, 10, 'A', BLACK, WHITE, 3);
  drawChar(90, 10, 'P', BLACK, WHITE, 3);
  drawChar(110, 10, 'S', BLACK, WHITE, 3);
  refreshDenominator();
}

void Adafruit_SharpMem::renderScreenPause(uint8_t seconds){
  drawTabs();
  refreshTabs();

  drawTabSettings(5);
  refreshTabSettings();

  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  

  if (seconds < 10){
    drawNum(150, 0, seconds);    
  } else {
    drawNum(120, 0, seconds/10);
    drawNum(190, 0, seconds%10);
  }

  drawChar(55, 10, 'S', BLACK, WHITE, 3);
  drawChar(70, 10, 'E', BLACK, WHITE, 3);
  drawChar(90, 10, 'C', BLACK, WHITE, 3);
  drawChar(110, 10, 'O', BLACK, WHITE, 3);
  drawChar(130, 10, 'N', BLACK, WHITE, 3);
  drawChar(150, 10, 'D', BLACK, WHITE, 3);
  drawChar(170, 10, 'S', BLACK, WHITE, 3);

  drawChar(210, 10, 'L', BLACK, WHITE, 3);
  drawChar(230, 10, 'E', BLACK, WHITE, 3);
  drawChar(250, 10, 'F', BLACK, WHITE, 3);
  drawChar(270, 10, 'T', BLACK, WHITE, 3);
  refreshDenominator();

}


/**************************************************************************/
/*! 
    @brief Gets the value (1 or 0) of the specified pixel from the buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)

    @return     1 if the pixel is enabled, 0 if disabled
*/
/**************************************************************************/
uint8_t Adafruit_SharpMem::getPixel(uint16_t x, uint16_t y)
{
  if ((x >=SHARPMEM_LCDWIDTH) || (y >=SHARPMEM_LCDHEIGHT)) return 0;
  if ((x >=SHARPMEM_LCDWIDTH) || (y >=BUFFER_HEIGHT)) return 0;
  //TODO: handle section logic here
  return sectionBuffer[(y*SHARPMEM_LCDWIDTH + x) /8] & (1 << x % 8) ? 1 : 0;
}

uint8_t Adafruit_SharpMem::getPaceMin(void){
  return paceMin;
}
uint8_t Adafruit_SharpMem::getPaceSec(void){
  return paceSec;
}
uint8_t Adafruit_SharpMem::getBreakMin(void){
  return breakMin;
}
uint8_t Adafruit_SharpMem::getBreakSec(void){
  return breakSec;
}
uint8_t Adafruit_SharpMem::getLength(void){
  return length;
}
uint8_t Adafruit_SharpMem::getDepth(void){
  return depth;
}
bool Adafruit_SharpMem::getImperial(void){
  return imperial;
}

void Adafruit_SharpMem::setPaceMin(uint8_t min){
  paceMin = min;
}
void Adafruit_SharpMem::setPaceSec(uint8_t sec){
  paceSec = sec;
}
void Adafruit_SharpMem::setBreakMin(uint8_t min){
  breakMin = min;
}
void Adafruit_SharpMem::setBreakSec(uint8_t sec){
  breakSec = sec;
}
void Adafruit_SharpMem::setLength(uint8_t len){
  length = len;
}
void Adafruit_SharpMem::setDepth(uint8_t dep){
  depth = dep;
}
void Adafruit_SharpMem::setImperial(bool imp){
  imperial = imp;
}

unsigned char Adafruit_SharpMem::toUC(uint8_t in){
  switch (in) {
    case 0:
      return '0';
      break;
    case 1:
      return '1';
      break;
    case 2:
      return '2';
      break;
    case 3:
      return '3';
      break;
    case 4:
      return '4';
      break;
    case 5:
      return '5';
      break;
    case 6:
      return '6';
      break;
    case 7:
      return '7';
      break;
    case 8:
      return '8';
      break;
    case 9:
      return '9';
      break;
    default:
      break;
  }
}
