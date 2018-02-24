#ifndef _CONFIG_H_
#define _CONFIG_H_

///////////// Displays /////////////
#define I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE

///////////// Input method /////////////
#define SERIAL_TERM_IN_USE
//#define KEYPAD_8x5_IN_USE

#if defined(PS2_KEYBOARD_IN_USE) && defined(KEYPAD_8x5_IN_USE)
#error Only one input method should be defined!
#endif

#endif /* _CONFIG_H_ */
