// Simple Hello world demo

// pin definitions
#define OLED_DATA 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RST 13

#include <SSD1306ASCII.h>

SSD1306ASCII oled(OLED_DATA, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);
//------------------------------------------------------------------------------
void setup() {                
  // If you want to provide external 7-9V VCC, uncomment next line and comment the one after
  //oled.ssd1306_init(SSD1306_EXTERNALVCC);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  oled.ssd1306_init(SSD1306_SWITCHCAPVCC);
  
  oled.clear();
  oled.print("Hello world!");
  
  oled.setCursor(0, 1);
  oled.write("A long line is truncated");
  
  oled.setCursor(5, 3);
  double x = 123.45;
  oled.write("x = ");
  oled.print(x);
}
//------------------------------------------------------------------------------
void loop() {}
