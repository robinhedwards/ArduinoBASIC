#ifndef _CONFIG_H_
#define _CONFIG_H_

///////////// Displays /////////////
// #define SSD1306ASCII_OLED_DISPLAY_IN_USE
#define I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE

///////////// Input method /////////////
// #define PS2_KEYBOARD_IN_USE
#define SERIAL_TERM_IN_USE

//////////// Misc. /////////////
//#define EXTERNAL_EEPROM_IN_USE


#if defined(SSD1306ASCII_OLED_DISPLAY_IN_USE) && defined(I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE)
#error Only one disp. should be defined!
#endif

#if defined(PS2_KEYBOARD_IN_USE) && defined(SERIAL_TERM_IN_USE)
#error Only one input method should be defined!
#endif

#endif /* _CONFIG_H_ */
