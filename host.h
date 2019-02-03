#include <Arduino.h>
#include <stdint.h>

#define MAXTEXTLEN          128  // 1行の最大文字数
#if !defined (__STM32F1__)
#define WiringPinMode  int
#endif

void host_init();
void host_sleep(long ms);
void host_digitalWrite(int pin,int state);
int host_digitalRead(int pin);
int host_analogRead(int pin);
void host_pinMode(int pin, WiringPinMode mode);
void host_click();
void host_cls();
void host_showBuffer();
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
