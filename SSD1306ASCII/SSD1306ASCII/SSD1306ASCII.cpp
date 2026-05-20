// some of this code was written by <cstone@pobox.com> originally; 
// it is in the public domain.
/**
 *  Adafruit SSD1306 library modified by William Greiman for
 *  unbuffered LiquidCrystal character mode.
 *  Faster (4x) software SPI patched in from Adafruit library by R.Edwards
 */
#include <avr/pgmspace.h>
#include <SSD1306ASCII.h>

#if defined(FONT_TYPE) && FONT_TYPE  < 2
#if FONT_TYPE == 1
#define ASCII96
#endif  // FONT_TYPE == 1
#include "glcdfont.c"
#elif FONT_TYPE == 2
#include "font.h"
#elif FONT_TYPE == 3
#include "font5x7.c"
#elif FONT_TYPE == 4
#include "font_whsw.c"
#else  // FONT_TYPE
#error Bad FONT_TYPE
#endif  // FONT_TYPE
//------------------------------------------------------------------------------
// return bitmap for a character
void SSD1306ASCII::charBitmap(uint8_t c, uint8_t* bits) {
#if FONT_TYPE
  if (c < 32 || c > 127) c = 127;
  c -= 32;
#endif  // FONT_TYPE
  const uint8_t *base = font + 5 * c;
  for (uint8_t i =0; i < 5; i++ ) {
    bits[i] = pgm_read_byte(base + i);
  }
}
//------------------------------------------------------------------------------
// clear the screen
void SSD1306ASCII::clear() {
  for (uint8_t r = 0; r < SSD1306_LCDHEIGHT/8; r++) {
   setCursor(0, r);
   for (uint8_t i = 0; i < SSD1306_LCDWIDTH; i++) {
     ssd1306_data(0);
   }
  }
  setCursor(0, 0);
}
//------------------------------------------------------------------------------
// send a command
void SSD1306ASCII::ssd1306_command(uint8_t c) {
    // SPI
    //digitalWrite(cs, HIGH);
    *csport |= cspinmask;
    //digitalWrite(dc, LOW);
    *dcport &= ~dcpinmask;
    //digitalWrite(cs, LOW);
    *csport &= ~cspinmask;
    fastSPIwrite(c);
    //digitalWrite(cs, HIGH);
    *csport |= cspinmask;
}
//------------------------------------------------------------------------------
// send data
void SSD1306ASCII::ssd1306_data(uint8_t c) {
    // SPI
    //digitalWrite(cs, HIGH);
    *csport |= cspinmask;
    //digitalWrite(dc, HIGH);
    *dcport |= dcpinmask;
    //digitalWrite(cs, LOW);
    *csport &= ~cspinmask;
    fastSPIwrite(c);
    //digitalWrite(cs, HIGH);
    *csport |= cspinmask;
}
//------------------------------------------------------------------------------
void SSD1306ASCII::ssd1306_init(uint8_t vccstate) {
  // initial cursor position
  col_ = 0;
  row_ = 0;
  
  // set pin directions
  pinMode(dc_, OUTPUT);
  pinMode(cs_, OUTPUT);
  csport      = portOutputRegister(digitalPinToPort(cs_));
  cspinmask   = digitalPinToBitMask(cs_);
  dcport      = portOutputRegister(digitalPinToPort(dc_));
  dcpinmask   = digitalPinToBitMask(dc_);
  pinMode(data_, OUTPUT);
  pinMode(clk_, OUTPUT);
  clkport     = portOutputRegister(digitalPinToPort(clk_));
  clkpinmask  = digitalPinToBitMask(clk_);
  mosiport    = portOutputRegister(digitalPinToPort(data_));
  mosipinmask = digitalPinToBitMask(data_);

  pinMode(rst_, OUTPUT);
  digitalWrite(rst_, HIGH);
  // VDD (3.3V) goes high at start, lets just chill for a ms
  delay(1);
  // bring0xset low
  digitalWrite(rst_, LOW);
  // wait 10ms
  delay(10);
  // bring out of reset
  digitalWrite(rst_, HIGH);
  // turn on VCC (9V?)

   #if defined SSD1306_128_32
    // Init sequence for 128x32 OLED module
    ssd1306_command(SSD1306_DISPLAYOFF);          // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);  // 0xD5
    ssd1306_command(0x80);                        // the suggested ratio 0x80
    ssd1306_command(SSD1306_SETMULTIPLEX);        // 0xA8
    ssd1306_command(0x1F);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);    // 0xD3
    ssd1306_command(0x0);                         // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);  // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);          // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x10); }
    else 
      { ssd1306_command(0x14); }
    ssd1306_command(SSD1306_MEMORYMODE);          // 0x20
    ssd1306_command(0x02);                        // 0x2 page mode
	ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);          // 0xDA
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);         // 0x81
    ssd1306_command(0x8F);
    ssd1306_command(SSD1306_SETPRECHARGE);        // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x22); }
    else 
      { ssd1306_command(0xF1); }
    ssd1306_command(SSD1306_SETVCOMDETECT);       // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME); // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);       // 0xA6
  #endif

  #if defined SSD1306_128_64
    // Init sequence for 128x64 OLED module
    ssd1306_command(SSD1306_DISPLAYOFF);          // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);  // 0xD5
    ssd1306_command(0x80);                        // the suggested ratio 0x80
    ssd1306_command(SSD1306_SETMULTIPLEX);        // 0xA8
    ssd1306_command(0x3F);
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);    // 0xD3
    ssd1306_command(0x0);                         // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);  // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);          // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x10); }
    else 
      { ssd1306_command(0x14); }
    ssd1306_command(SSD1306_MEMORYMODE);          // 0x20
    ssd1306_command(0x02);                        // 0x2 page mode
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);          // 0xDA
    ssd1306_command(0x12);
    ssd1306_command(SSD1306_SETCONTRAST);         // 0x81
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x9F); }
    else 
      { ssd1306_command(0xCF); }
    ssd1306_command(SSD1306_SETPRECHARGE);        // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC) 
      { ssd1306_command(0x22); }
    else 
      { ssd1306_command(0xF1); }
    ssd1306_command(SSD1306_SETVCOMDETECT);       // 0xDB
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME); // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);       // 0xA6
  #endif
  
  ssd1306_command(SSD1306_DISPLAYON);//--turn on oled panel
}
//------------------------------------------------------------------------------
void SSD1306ASCII::setCursor(uint8_t col, uint8_t row) {
  if (row >= SSD1306_LCDHEIGHT/8) {
    row = SSD1306_LCDHEIGHT/8 - 1;
  }
  if (col >= SSD1306_LCDWIDTH/6) {
    col = SSD1306_LCDWIDTH/6 - 1;
  }
  col_ = col;
  row_ = row;
  // convert character column to display column
  col *= 6;
  ssd1306_command(SSD1306_SETLOWCOLUMN | (col & 0XF));
  ssd1306_command(SSD1306_SETHIGHCOLUMN | (col >> 4));
  ssd1306_command(SSD1306_SETSTARTPAGE | row);
}
//------------------------------------------------------------------------------
inline void SSD1306ASCII::fastSPIwrite(uint8_t d) {
  for(uint8_t bit = 0x80; bit; bit >>= 1) {
    *clkport &= ~clkpinmask;
    if(d & bit) *mosiport |=  mosipinmask;
    else        *mosiport &= ~mosipinmask;
    *clkport |=  clkpinmask;
  }
}
//------------------------------------------------------------------------------
#if ARDUINO >= 100
size_t SSD1306ASCII::write(uint8_t c) {
  if (col_ >= SSD1306_LCDWIDTH/6) return 0;
#else  // ARDUINO >= 100
void SSD1306ASCII::write(uint8_t c) {
  if (col_ >= SSD1306_LCDWIDTH/6) return;
#endif  // ARDUINO >= 100
  col_++;
#if FONT_TYPE
  if (c < 32 || c > 127) c = 127;
  c -= 32;
#endif  // FONT_TYPE
  const uint8_t *base = font + 5 * c;
  for (uint8_t i = 0; i < 5; i++ ) {
    uint8_t b = pgm_read_byte(base + i);
    ssd1306_data(b);
  }
  ssd1306_data(0);
#if ARDUINO >= 100
  return 1;
#endif //  ARDUINO >= 100
}
//------------------------------------------------------------------------------
#if ARDUINO >= 100
size_t SSD1306ASCII::write(const char* s) {
  size_t n = strlen(s);
  for (size_t i = 0; i < n; i++) {
    write(s[i]);
  }
  return n;
}
#else
void SSD1306ASCII::write(const char* s) {
  while (*s) write(*s++);
}
#endif //  ARDUINO >= 100
