#ifndef _HOST_H_
#define _HOST_H_
#include "config.h"
#include <stdint.h>

#ifdef I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE
#define LCD_SERIAL_ADDRESS                      0x27
#define SCREEN_WIDTH                            16
#define SCREEN_HEIGHT                           2
#define CURSOR_CHR                              255
#endif

#ifdef SERIAL_TERM_IN_USE
#define SERIAL_DELETE                           127
#define SERIAL_CR                               13
#define SERIAL_ESC                              27
#endif

#ifdef KEYPAD_8x5_IN_USE
#define KEY_ENTER                               13
#define SYMBOL_SHIFT                            1
#define COLS                                    5
#define ROWS                                    8
#define IC_74HCT138_A0                          14
#define IC_74HCT138_A1                          15
#define IC_74HCT138_A2                          16
#define KBD_0                                   4
#define KBD_1                                   5
#define KBD_2                                   6
#define KBD_3                                   7
#define KBD_4                                   8
#endif

// buzzer pin, 0 = disabled/not present
#define BUZZER_PIN    0

#define MAGIC_AUTORUN_NUMBER    0xFC

void host_init(int buzzerPin);
void host_sleep(long ms);
void host_digitalWrite(int pin,int state);
int host_digitalRead(int pin);
int host_analogRead(int pin);
void host_pinMode(int pin, int mode);
void host_click();
void host_startupTone();
void host_cls();
void host_showBuffer();
void host_moveCursor(int x, int y);
void host_outputString(char *str);
void host_outputProgMemString(const char *str);
void host_outputChar(char c);
void host_outputFloat(float f);
char *host_floatToStr(float f, char *buf);
int host_outputInt(long val);
void host_newLine();
char *host_readLine();
char host_getKey();
bool host_ESCPressed();
void host_outputFreeMem(unsigned int val);
void host_saveProgram(bool autoexec);
void host_loadProgram();
#endif /* _HOST_H_ */
