// Display all characters in the font

// pin definitions
#define OLED_DATA 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RST 13

#define VIEW_DELAY 10000

#include <SSD1306ASCII.h>

SSD1306ASCII oled(OLED_DATA, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);
//------------------------------------------------------------------------------
void setup() {                
  // If you want to provide external 7-9V VCC, uncomment next line and comment the one after
  //oled.ssd1306_init(SSD1306_EXTERNALVCC);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  oled.ssd1306_init(SSD1306_SWITCHCAPVCC);
}
//------------------------------------------------------------------------------
void loop() {
  // set start character
  uint16_t c = FONT_TYPE ? 32 : 0;
  // set limit character
  uint16_t limit = FONT_TYPE ? 128 : 256;
  uint8_t row = 0;
  oled.clear();
  while (1) {
    for ( uint8_t i = 0; i < 16; i++) {
      oled.write(c++);
    }
    if (c == limit) break;
    if (++row == SSD1306_LCDHEIGHT/8) {
      delay(VIEW_DELAY);
      row = 0;
      oled.clear();
    }
    oled.setCursor(0, row);
  }
  delay(VIEW_DELAY);
}
