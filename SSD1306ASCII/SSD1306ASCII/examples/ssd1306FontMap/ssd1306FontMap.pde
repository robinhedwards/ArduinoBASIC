// print simple representation of ASCII characters in font
#include <SdFat.h>
#include <SSD1306ASCII.h>

SdFat sd;
SdFile file;
//------------------------------------------------------------------------------
void printChar(uint8_t c) {
  uint8_t bits[192];
  // get bit maps
  for (uint8_t i = 0; i < 32;  i++) {
    SSD1306ASCII::charBitmap(c + i, bits + 6*i);
    bits[6*i + 5] = 0;
  }
  // print characters
  for (uint8_t m = 1; m; m <<= 1) {
    for (uint8_t i = 0; i < 192; i++) {
      file.write(bits[i] & m ? 'X' : ' ');
    }
    file.println();
  }
}
//------------------------------------------------------------------------------
void setup()   {
  Serial.begin(9600);
  sd.init();
  char name[] = "FONT0.TXT";
  name[4] = '0' + FONT_TYPE;
  if (!file.open(name, O_CREAT | O_WRITE |O_TRUNC)) {
    Serial.println("OPEN ERROR");
  }
  for (uint8_t c = 32; c < 128; c += 32) printChar(c);
  file.close();
  Serial.println("DONE");
}
//------------------------------------------------------------------------------
void loop(){ }
