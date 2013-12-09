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
  // breakMin = 0;
  // breakSec = 0;
  length = 5;
  depthShallow = 3;
  depthDeep = 3;
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

  // drawChar(4, 13, 'P', BLACK, WHITE, 2);
  // drawChar(16, 13, 'A', BLACK, WHITE, 2);
  // drawChar(28, 13, 'C', BLACK, WHITE, 2);
  // drawChar(40, 13, 'E', BLACK, WHITE, 2);

  drawBitmap(4, 13, paceSmall, 50, 14, BLACK);

  // drawChar(82, 13, 'L', BLACK, WHITE, 2);
  // drawChar(94, 13, 'E', BLACK, WHITE, 2);
  // drawChar(106, 13, 'N', BLACK, WHITE, 2);
  // drawChar(118, 13, 'G', BLACK, WHITE, 2);
  // drawChar(130, 13, 'T', BLACK, WHITE, 2);
  // drawChar(142, 13, 'H', BLACK, WHITE, 2);
  
  drawBitmap(82, 13, lengthSmall, 78, 14, BLACK);

  // drawChar(163, 13, 'D', BLACK, WHITE, 2);
  // drawChar(175, 13, 'E', BLACK, WHITE, 2);
  // drawChar(187, 13, 'P', BLACK, WHITE, 2);
  // drawChar(199, 13, 'T', BLACK, WHITE, 2);
  // drawChar(211, 13, 'H', BLACK, WHITE, 2);

  drawBitmap(163, 13, depthSmall, 64, 14, BLACK);

  // drawChar(249, 13, 'U', BLACK, WHITE, 2);
  // drawChar(261, 13, 'N', BLACK, WHITE, 2);
  // drawChar(273, 13, 'I', BLACK, WHITE, 2);
  // drawChar(285, 13, 'T', BLACK, WHITE, 2);
  // drawChar(297, 13, 'S', BLACK, WHITE, 2);

  drawBitmap(249, 13, unitsSmall, 57, 14, BLACK);

  drawFastVLine(0, 0, 40, BLACK);
  drawFastVLine(79, 0, 40, BLACK);
  drawFastVLine(159, 0, 40, BLACK);
  drawFastVLine(239, 0, 40, BLACK);
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
  drawFastVLine(79, 0, 40, BLACK);
  drawFastVLine(159, 0, 40, BLACK);
  drawFastVLine(239, 0, 40, BLACK);
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

  // draw len
  if (length < 10){
    drawChar(96, 13, toUC(length), BLACK, WHITE, 2);

  } else {
    drawChar(84, 13, toUC(length/10), BLACK, WHITE, 2);
    drawChar(96, 13, toUC(length%10), BLACK, WHITE, 2);

  }
  if (imperial){
    drawBitmap(116, 13, ydsSmall, 32, 17, BLACK);

    // drawChar(116, 13, 'Y', BLACK, WHITE, 2);
    // drawChar(128, 13, 'D', BLACK, WHITE, 2);
    // drawChar(140, 13, 'S', BLACK, WHITE, 2);

  }else{
    drawBitmap(116, 17, mSmall, 15, 10, BLACK);
    // drawChar(116, 13, 'M', BLACK, WHITE, 2);
  }

  // draw depth
  // TODO: draw both depths, account for 
  if (depthDeep < 10){
    drawChar(162, 13, toUC(depthDeep), BLACK, WHITE, 2);
    drawFastHLine(174, 20, 10, BLACK);
    drawFastHLine(174, 21, 10, BLACK);


    if (depthShallow < 10){
      drawChar(186, 13, toUC(depthShallow), BLACK, WHITE, 2);
      drawBitmap(208, 13, ftSmall, 15, 16, BLACK);

      // drawChar(208, 13, 'F', BLACK, WHITE, 2);
      // drawChar(220, 13, 'T', BLACK, WHITE, 2);
    } else {
      drawChar(186, 13, toUC(depthShallow/10), BLACK, WHITE, 2);
      drawChar(198, 13, toUC(depthShallow%10), BLACK, WHITE, 2);
      // drawChar(220, 13, 'F', BLACK, WHITE, 2);
      // drawChar(232, 13, 'T', BLACK, WHITE, 2);
      drawBitmap(220, 13, ftSmall, 15, 16, BLACK);

    } 
  } else {
    drawChar(162, 13, toUC(depthDeep/10), BLACK, WHITE, 2);
    drawChar(174, 13, toUC(depthDeep%10), BLACK, WHITE, 2);
    drawFastHLine(186, 20, 10, BLACK);
    drawFastHLine(186, 21, 10, BLACK);
    if (depthShallow < 10){
      drawChar(198, 13, toUC(depthShallow), BLACK, WHITE, 2);
      // drawChar(215, 13, 'F', BLACK, WHITE, 2);
      // drawChar(227, 13, 'T', BLACK, WHITE, 2);
      drawBitmap(215, 13, ftSmall, 15, 16, BLACK);

    } else {
      drawChar(198, 13, toUC(depthShallow/10), BLACK, WHITE, 2);
      drawChar(210, 13, toUC(depthShallow%10), BLACK, WHITE, 2);
      // drawChar(225, 13, 'F', BLACK, WHITE, 2);
      // drawChar(230, 13, 'T', BLACK, WHITE, 2);
      drawBitmap(225, 13, ftSmall, 15, 16, BLACK);

    } 
  }

  // draw units
  if (imperial){
    drawBitmap(249, 13, yardsSmall, 50, 14, BLACK);
    // drawChar(249, 13, 'Y', BLACK, WHITE, 2);
    // drawChar(261, 13, 'A', BLACK, WHITE, 2);
    // drawChar(273, 13, 'R', BLACK, WHITE, 2);
    // drawChar(285, 13, 'D', BLACK, WHITE, 2);
    // drawChar(297, 13, 'S', BLACK, WHITE, 2);
  }else{
    drawBitmap(243, 13, metersSmall, 65, 14, BLACK);
    // drawChar(243, 13, 'M', BLACK, WHITE, 2);
    // drawChar(255, 13, 'E', BLACK, WHITE, 2);
    // drawChar(267, 13, 'T', BLACK, WHITE, 2);
    // drawChar(279, 13, 'E', BLACK, WHITE, 2);
    // drawChar(291, 13, 'R', BLACK, WHITE, 2);
    // drawChar(303, 13, 'S', BLACK, WHITE, 2);
  }

  switch(position){
    // Pace
    case 0:
      drawFastHLine(SHARPMEM_LCDWIDTH/4, 0, SHARPMEM_LCDWIDTH, BLACK);
      drawFastHLine(SHARPMEM_LCDWIDTH/4, 39, SHARPMEM_LCDWIDTH, BLACK);
      break;
    // Length
    case 1:
      drawFastHLine(0, 0, SHARPMEM_LCDWIDTH/4, BLACK);
      drawFastHLine((2*SHARPMEM_LCDWIDTH/4), 0, SHARPMEM_LCDWIDTH, BLACK);

      drawFastHLine(0, 39, SHARPMEM_LCDWIDTH/4, BLACK);
      drawFastHLine((2*SHARPMEM_LCDWIDTH/4), 39, SHARPMEM_LCDWIDTH, BLACK);
      break;
    // Depth
    case 2:
      drawFastHLine(0, 0, (2*SHARPMEM_LCDWIDTH/4), BLACK);
      drawFastHLine((3*SHARPMEM_LCDWIDTH/4), 0, SHARPMEM_LCDWIDTH, BLACK);

      drawFastHLine(0, 39, (2*SHARPMEM_LCDWIDTH/4), BLACK);
      drawFastHLine((3*SHARPMEM_LCDWIDTH/4), 39, SHARPMEM_LCDWIDTH, BLACK);
      break;
    // Units
    case 3:
      drawFastHLine(0, 0, (3*SHARPMEM_LCDWIDTH/4), BLACK);
      drawFastHLine(0, 39, (3*SHARPMEM_LCDWIDTH/4), BLACK);

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
void Adafruit_SharpMem::drawDenominator(){
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  drawFastHLine(54, 0, 212, BLACK);
  drawFastHLine(54, 1, 212, BLACK);

  drawBitmap(55, 10, hundred, 91, 40, BLACK);
  // drawChar(55, 10, toUC(1), BLACK, WHITE, 3);
  // drawChar(70, 10, toUC(0), BLACK, WHITE, 3);
  // drawChar(90, 10, toUC(0), BLACK, WHITE, 3);

  if (imperial){
    drawBitmap(156, 10, ydsLarge, 55, 34, BLACK);
    // drawChar(125, 10, 'Y', BLACK, WHITE, 3);
    // drawChar(145, 10, 'D', BLACK, WHITE, 3);
    // drawChar(165, 10, 'S', BLACK, WHITE, 3);    
  } else {
    drawBitmap(156, 20, mLarge, 30, 20, BLACK);
    // drawChar(125, 10, 'M', BLACK, WHITE, 3);
  }

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

//TODO: update for 2-depth
void Adafruit_SharpMem::renderDepth(uint8_t deep, uint8_t shallow, uint8_t side){
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);

  if (deep < 10){
    drawNum(55, 0, deep);    
  } else {
    drawNum(30, 0, deep/10);
    drawNum(80, 0, deep%10);
  }

  drawFastVLine(160, 0, 80, BLACK);

  if (shallow < 10){
    drawNum(200, 0, shallow);    
  } else {
    drawNum(200, 0, shallow/10);
    drawNum(250, 0, shallow%10);
  }

  refreshCentral();

  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  drawBitmap(50, 10, deepLarge, 96, 37, BLACK);
  // drawChar(50, 10, 'D', BLACK, WHITE, 3);
  // drawChar(70, 10, 'E', BLACK, WHITE, 3);
  // drawChar(90, 10, 'E', BLACK, WHITE, 3);
  // drawChar(110, 10, 'P', BLACK, WHITE, 3);

  drawBitmap(170, 10, shallowLarge, 144, 30, BLACK);
  // drawChar(170, 10, 'S', BLACK, WHITE, 3);
  // drawChar(190, 10, 'H', BLACK, WHITE, 3);
  // drawChar(210, 10, 'A', BLACK, WHITE, 3);
  // drawChar(230, 10, 'L', BLACK, WHITE, 3);
  // drawChar(250, 10, 'L', BLACK, WHITE, 3);
  // drawChar(270, 10, 'O', BLACK, WHITE, 3);
  // drawChar(290, 10, 'W', BLACK, WHITE, 3);

  switch(side){
    case 0:
      drawFastHLine(50, 35, 80, BLACK);
      drawFastHLine(50, 36, 80, BLACK);
      break;
    case 1:
      drawFastHLine(170, 35, 140, BLACK);
      drawFastHLine(170, 36, 140, BLACK);
      break;
  }

  refreshDenominator();
}

void Adafruit_SharpMem::renderLength(uint8_t len){
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);

  if (len < 10){
    drawNum(150, 0, len);    
  } else {
    drawNum(120, 0, len/10);
    drawNum(190, 0, len%10);
  }
  refreshCentral();

  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  if (imperial){
    drawBitmap(130, 10, yardsLarge, 105, 30, BLACK);
    // drawChar(130, 10, 'Y', BLACK, WHITE, 3);
    // drawChar(150, 10, 'A', BLACK, WHITE, 3);
    // drawChar(170, 10, 'R', BLACK, WHITE, 3);
    // drawChar(190, 10, 'D', BLACK, WHITE, 3);
    // drawChar(210, 10, 'S', BLACK, WHITE, 3);
  } else {
    drawBitmap(130, 10, metersLarge, 133, 28, BLACK);
    // drawChar(130, 10, 'M', BLACK, WHITE, 3);
    // drawChar(150, 10, 'E', BLACK, WHITE, 3);
    // drawChar(170, 10, 'T', BLACK, WHITE, 3);
    // drawChar(190, 10, 'E', BLACK, WHITE, 3);
    // drawChar(210, 10, 'R', BLACK, WHITE, 3);
    // drawChar(230, 10, 'S', BLACK, WHITE, 3);
  }
  refreshDenominator();
}

void Adafruit_SharpMem::renderUnits(bool imp){
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  drawBitmap(100, 0, yardsLarge, 105, 30, BLACK);

  // drawChar(55, 0, 'Y', BLACK, WHITE, 3);
  // drawChar(75, 0, 'A', BLACK, WHITE, 3);
  // drawChar(95, 0, 'R', BLACK, WHITE, 3);
  // drawChar(115, 0, 'D', BLACK, WHITE, 3);
  // drawChar(135, 0, 'S', BLACK, WHITE, 3);

  drawBitmap(100, 40, metersLarge, 133, 28, BLACK);
  // drawChar(55, 40, 'M', BLACK, WHITE, 3);
  // drawChar(75, 40, 'E', BLACK, WHITE, 3);
  // drawChar(95, 40, 'T', BLACK, WHITE, 3);
  // drawChar(115, 40, 'E', BLACK, WHITE, 3);
  // drawChar(135, 40, 'R', BLACK, WHITE, 3);
  // drawChar(155, 40, 'S', BLACK, WHITE, 3);

  if (imp){ // draw selector triangle for imperial (yards)
    drawFastVLine(45, 0, 20, BLACK);
    drawFastVLine(46, 1, 18, BLACK);
    drawFastVLine(47, 2, 16, BLACK);
    drawFastVLine(48, 3, 14, BLACK);
    drawFastVLine(49, 4, 12, BLACK);
    drawFastVLine(50, 5, 10, BLACK);
    drawFastVLine(51, 6, 8, BLACK);
    drawFastVLine(52, 7, 6, BLACK);
    drawFastVLine(53, 8, 4, BLACK);
    drawFastVLine(54, 9, 2, BLACK);
  } else {  // draw selector triangle for metric (meters)
    drawFastVLine(45, 40, 20, BLACK);
    drawFastVLine(46, 41, 18, BLACK);
    drawFastVLine(47, 42, 16, BLACK);
    drawFastVLine(48, 43, 14, BLACK);
    drawFastVLine(49, 44, 12, BLACK);
    drawFastVLine(50, 45, 10, BLACK);
    drawFastVLine(51, 46, 8, BLACK);
    drawFastVLine(52, 47, 6, BLACK);
    drawFastVLine(53, 48, 4, BLACK);
    drawFastVLine(54, 49, 2, BLACK);
  }
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

  // TODO: make sure always over 100 yds
  drawDenominator();
  refreshDenominator();
}

void Adafruit_SharpMem::renderScreenLength(void){
  drawTabs();
  refreshTabs();  

  // draw length tab line
  drawTabSettings(1);
  refreshTabSettings();

  renderLength(length);
}

void Adafruit_SharpMem::renderScreenDepth(uint8_t side){
  drawTabs();
  refreshTabs();  

  // draw depth tab line
  drawTabSettings(2);
  refreshTabSettings();

  renderDepth(depthDeep, depthShallow, side);
}


void Adafruit_SharpMem::renderScreenUnits(void){
  drawTabs();
  refreshTabs();  

  // draw units tab line
  drawTabSettings(3);
  refreshTabSettings();

  renderUnits(imperial);
  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  refreshDenominator();
}

void Adafruit_SharpMem::renderScreenSwim(uint8_t laps){
  drawTabs();
  refreshTabs();

  drawTabSettings(4);
  refreshTabSettings();

  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);

  if (laps < 10){
    drawNum(150, 0, laps);    
  } else {
    drawNum(120, 0, laps/10);
    drawNum(190, 0, laps%10);
  }
  refreshCentral();

  memset(sectionBuffer, 0xff, (SHARPMEM_LCDWIDTH * BUFFER_HEIGHT) / 8);
  drawBitmap(130, 10, lapsLarge, 128, 40, BLACK);
  // drawChar(130, 10, 'L', BLACK, WHITE, 3);
  // drawChar(150, 10, 'A', BLACK, WHITE, 3);
  // drawChar(170, 10, 'P', BLACK, WHITE, 3);
  // drawChar(190, 10, 'S', BLACK, WHITE, 3);
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
// uint8_t Adafruit_SharpMem::getBreakMin(void){
//   return breakMin;
// }
// uint8_t Adafruit_SharpMem::getBreakSec(void){
//   return breakSec;
// }
uint8_t Adafruit_SharpMem::getLength(void){
  return length;
}
uint8_t Adafruit_SharpMem::getDepthShallow(void){
  return depthShallow;
}

uint8_t Adafruit_SharpMem::getDepthDeep(void){
  return depthDeep;
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
// void Adafruit_SharpMem::setBreakMin(uint8_t min){
//   breakMin = min;
// }
// void Adafruit_SharpMem::setBreakSec(uint8_t sec){
//   breakSec = sec;
// }
void Adafruit_SharpMem::setLength(uint8_t len){
  length = len;
}
void Adafruit_SharpMem::setDepthShallow(uint8_t dep){
  depthShallow = dep;
}
void Adafruit_SharpMem::setDepthDeep(uint8_t dep){
  depthDeep = dep;
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
