/**
 *  Adafruit SSD1306 library modified by William Greiman for
 *  unbuffered LiquidCrystal character mode.
 *  Faster (4x) software SPI patched in from Adafruit library by R.Edwards
 */
#ifndef SSD1306ASCII_h
#define SSD1306ASCII_h
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#ifdef __SAM3X8E__
typedef volatile RwReg PortReg;
typedef uint32_t PortMask;
#else
typedef volatile uint8_t PortReg;
typedef uint8_t PortMask;
#endif

/*=========================================================================
    SSD1306 Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (128x64, 128x32, etc.).
    Select the appropriate display below to create an appropriately
    sized framebuffer, etc.

    SSD1306_128_64  128x64 pixel display

    SSD1306_128_32  128x32 pixel display

    You also need to set the LCDWIDTH and LCDHEIGHT defines to an 
    appropriate size

    -----------------------------------------------------------------------*/
#define SSD1306_128_64
// #define SSD1306_128_32

/*=========================================================================
    Values for FONT_TYPE are:
    0 - 256 Character Extended ASCII Adafruit font
    1 -  96 Character ASCII Adafruit font
    2 -  96 Character ASCII option font A
    3 -  96 Character ASCII option font B
    4 -  96 Character ASCII option font C
    -----------------------------------------------------------------------*/
#define FONT_TYPE 2
/*=========================================================================*/

#if defined SSD1306_128_64 && defined SSD1306_128_32
  #error "Only one SSD1306 display can be specified at once in SSD1306.h"
#endif
#if !defined SSD1306_128_64 && !defined SSD1306_128_32
  #error "At least one SSD1306 display must be specified in SSD1306.h"
#endif

#if defined SSD1306_128_64
  #define SSD1306_LCDWIDTH  128
  #define SSD1306_LCDHEIGHT  64
#endif
#if defined SSD1306_128_32
  #define SSD1306_LCDWIDTH  128
  #define SSD1306_LCDHEIGHT  32
#endif

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_SETSTARTPAGE 0XB0
#define SSD1306_MEMORYMODE 0x20

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2
//------------------------------------------------------------------------------
class SSD1306ASCII : public Print {
 public:
  SSD1306ASCII(int8_t data, int8_t clk, int8_t dc, int8_t rst, int8_t cs)
    :data_(data), clk_(clk), dc_(dc), rst_(rst), cs_(cs) {}

  SSD1306ASCII(int8_t data, int8_t clk, int8_t dc, int8_t rst)
    :data_(data), clk_(clk), dc_(dc), rst_(rst), cs_(-1) {}
    
  static void charBitmap(uint8_t c, uint8_t* bits);
  void clear();
  uint8_t col() {return col_;}
  void home() {setCursor(0, 0);}
  uint8_t row() {return row_;}
  void setCursor(uint8_t col, uint8_t row);
  void ssd1306_command(uint8_t c);
  void ssd1306_data(uint8_t c);
  void ssd1306_init(uint8_t switchvcc);
#if ARDUINO >= 100
  size_t write(uint8_t c);
  size_t write(const char* s);
#else  // ARDUINO >= 100
  void write(uint8_t c);
  void write(const char* s);
#endif  // ARDUINO >= 100
 private:
  // cursor position
  int8_t col_, row_;
  // OLED pins
  int8_t data_, clk_, dc_, rst_, cs_;
  PortReg *mosiport, *clkport, *csport, *dcport;
  PortMask mosipinmask, clkpinmask, cspinmask, dcpinmask;
  
  void fastSPIwrite(uint8_t c);
};
#endif  // SSD1306ASCII_h
