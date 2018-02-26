#include "config.h"
#include "host.h"
#include "basic.h"

#ifdef I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>
#endif

#include <EEPROM.h>

#ifdef I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE
extern LiquidCrystal_I2C lcd;
#endif

int timer1_counter;

char screenBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
char lineDirty[SCREEN_HEIGHT];
int curX = 0, curY = 0;
volatile char flash = 0, redraw = 0;
char inputMode = 0;
char inkeyChar = 0;
char buzPin = 0;

#ifdef KEYPAD_8x5_IN_USE
#define KEY_PERIOD                                  2
#define KEY_PRESSED_PERIOD                          20

volatile uint8_t value_COL_1 = 0;
volatile uint8_t value_COL_2 = 0;
volatile uint8_t value_ROW = 0;
volatile char pressed_key_char = 0;
volatile uint8_t timer1_counter_ms;
volatile uint8_t key_pressed_counter = 0;
#endif

const char bytesFreeStr[] PROGMEM = "bytes free";

#ifdef KEYPAD_8x5_IN_USE
const char key_map[ROWS][COLS] PROGMEM = 
{//COL 0   1    2    3    4                         ROW
    {' ', '.', 'M', 'N', 'V'},                      // 0
    {SYMBOL_SHIFT, 'Z', 'X', 'C', 'G'},             // 1
    {'A', 'S', 'D', 'F', 'T'},                      // 2
    {'Q', 'W', 'E', 'R', '5'},                      // 3
    {'1', '2', '3', '4', '6'},                      // 4
    {'0', '9', '8', '7', 'Y'},                      // 5
    {'P', 'O', 'I', 'U', 'H'},                      // 6
    {KEY_ENTER, 'L', 'K', 'J', 'B'}                 // 7
};
#endif

void initTimers() 
{
    noInterrupts();             // Disable all interrupts

    // Timer 1
    TCCR1A = 0;
    TCCR1B = 0;
    timer1_counter = 34286;     // Preload timer 65536-16MHz/256/2Hz
    TCNT1 = timer1_counter;     // Preload timer
    TCCR1B |= (1 << CS12);      // 256 prescaler 
    TIMSK1 |= (1 << TOIE1);     // Enable timer overflow interrupt

#ifdef KEYPAD_8x5_IN_USE
    // Timer 2
    // Setup Timer2 overwlow to fire every 8 ms (125 Hz)
    // period[sec] = (1 / f_clock[sec]) * prescale * (255 - count)
    TCCR2B = 0;                 // Disable Timer2 while setting up
    TCNT2 = 178;                // Reset Timer Count 
    TIFR2 = 0;                  // Clear Timer Overflow flag
    TIMSK2 = 0x01;              // Timer2 Overflow Interrupt Enable
    TCCR2A = 0;                 // Wave Gen Mode normal
    TCCR2B = 0x07;              // Timer Prescaler set to 1024
    timer1_counter_ms = KEY_PERIOD;
#endif
    interrupts();               // Enable all interrupts
}


ISR(TIMER1_OVF_vect)     
{
    TCNT1 = timer1_counter;     // preload timer
    flash = !flash;
    redraw = 1;
}

#ifdef KEYPAD_8x5_IN_USE
ISR(TIMER2_OVF_vect)   
{
    TCNT2 = 178;                // Reset Timer Count 
    TIFR2 = 0;                  // Clear Timer Overflow flag

    if(--timer1_counter_ms == 0)
    {
        timer1_counter_ms = KEY_PERIOD;
        value_COL_1 = get_col_value(value_ROW);

        if((value_COL_1 != 32) && (key_pressed_counter == 0))
        {
            pressed_key_char = get_key(value_ROW, value_COL_1);
            key_pressed_counter = KEY_PRESSED_PERIOD;
        }

        if(key_pressed_counter >0)
            key_pressed_counter--;
        
        value_ROW++;
        value_ROW = (value_ROW < ROWS) ? value_ROW : 0;
    }
}
#endif

#ifdef KEYPAD_8x5_IN_USE
uint8_t get_col_value(uint8_t row)
{
    uint8_t value;
    uint8_t col_value = 0; 
    uint8_t value_KBD;
    
    value = PORTC;
    PORTC = ((value & 0xF8) | row);
    value_KBD =  ((PIND & 0xF0) >> 3) | (PINB & 1);

    if (value_KBD == 29) 
        col_value = 0;
    else if (value_KBD == 27) 
        col_value = 1;
    else if (value_KBD == 23) 
        col_value = 2;
    else if (value_KBD == 15) 
        col_value = 3;
    else if (value_KBD == 30) 
        col_value = 4;
    else
        col_value = 32;  

    return col_value;
}


char get_key(uint8_t row, uint8_t col)
{
    char key = 0;
    char *ptr = (char *)key_map;
    
    if((col < COLS) && (row < ROWS))
        key = pgm_read_byte(ptr + (col + row * 5));

    return key;
}
#endif

void host_init(int buzzerPin) 
{
    buzPin = buzzerPin;

#ifdef I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE
    lcd.clear();
    lcd.setCursor(0, 0);
#endif

    if (buzPin)
        pinMode(buzPin, OUTPUT);

    initTimers();
}

void host_sleep(long ms)
{
    delay(ms);
}

void host_digitalWrite(int pin,int state)
{
    digitalWrite(pin, state ? HIGH : LOW);
}

int host_digitalRead(int pin)
{
    return digitalRead(pin);
}

int host_analogRead(int pin)
{
    return analogRead(pin);
}

void host_pinMode(int pin,int mode)
{
    pinMode(pin, mode);
}

void host_click()
{
    if (!buzPin) return;

    digitalWrite(buzPin, HIGH);
    delay(1);
    digitalWrite(buzPin, LOW);
}

void host_startupTone()
{
    if (!buzPin) return;

    for (int i=1; i<=2; i++)
    {
        for (int j=0; j<50*i; j++)
        {
            digitalWrite(buzPin, HIGH);
            delay(3-i);
            digitalWrite(buzPin, LOW);
            delay(3-i);
        }
        
        delay(100);
    }    
}

void host_cls()
{
    memset(screenBuffer, 32, SCREEN_WIDTH*SCREEN_HEIGHT);
    memset(lineDirty, 1, SCREEN_HEIGHT);
    curX = 0;
    curY = 0;
}

void host_moveCursor(int x, int y)
{
    if (x<0)
        x = 0;

    if (x>=SCREEN_WIDTH)
        x = SCREEN_WIDTH-1;
    
    if (y<0)
        y = 0;
    
    if (y>=SCREEN_HEIGHT)
        y = SCREEN_HEIGHT-1;
    
    curX = x;
    curY = y; 
}

void host_showBuffer()
{
    for (int y=0; y<SCREEN_HEIGHT; y++)
    {
        if (lineDirty[y] || (inputMode && y==curY))
        {

#ifdef I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE
            lcd.setCursor(0, y);
#endif
            for (int x=0; x<SCREEN_WIDTH; x++)
            {
                char c = screenBuffer[y * SCREEN_WIDTH + x];
                if (c<32) 
                    c = ' ';
                
                if (x==curX && y==curY && inputMode && flash)
                    c = CURSOR_CHR;

#ifdef I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE
                lcd.print(c);
#endif
            }

            lineDirty[y] = 0;
        }
    }
}

void scrollBuffer()
{
    memcpy(screenBuffer, screenBuffer + SCREEN_WIDTH, SCREEN_WIDTH*(SCREEN_HEIGHT-1));
    memset(screenBuffer + SCREEN_WIDTH*(SCREEN_HEIGHT-1), 32, SCREEN_WIDTH);
    memset(lineDirty, 1, SCREEN_HEIGHT);
    curY--;
}

void host_outputString(char *str)
{
    int pos = curY * SCREEN_WIDTH + curX;
    while (*str)
    {
        lineDirty[pos / SCREEN_WIDTH] = 1;
        screenBuffer[pos++] = *str++;
        if (pos >= SCREEN_WIDTH*SCREEN_HEIGHT)
        {
            scrollBuffer();
            pos -= SCREEN_WIDTH;
        }
    }
    
    curX = pos % SCREEN_WIDTH;
    curY = pos / SCREEN_WIDTH;
}

void host_outputProgMemString(const char *p)
{
    while (1)
    {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0) 
            break;
        
        host_outputChar(c);
    }
}

void host_outputChar(char c)
{
    int pos = curY * SCREEN_WIDTH + curX;
    lineDirty[pos / SCREEN_WIDTH] = 1;
    
    screenBuffer[pos++] = c;
    if (pos >= SCREEN_WIDTH*SCREEN_HEIGHT)
    {
        scrollBuffer();
        pos -= SCREEN_WIDTH;
    }
    
    curX = pos % SCREEN_WIDTH;
    curY = pos / SCREEN_WIDTH;
}


int host_outputInt(long num)
{
    // returns len
    long i = num, xx = 1;
    int c = 0;
    do {
        c++;
        xx *= 10;
        i /= 10;
    } 
    while (i);

    for (int i=0; i<c; i++)
    {
        xx /= 10;
        char digit = ((num/xx) % 10) + '0';
        host_outputChar(digit);
    }
    
    return c;
}

char *host_floatToStr(float f, char *buf)
{
    // floats have approx 7 sig figs
    float a = fabs(f);
    if (f == 0.0f)
    {
        buf[0] = '0'; 
        buf[1] = 0;
    }
    else if (a<0.0001 || a>1000000)
    {
        // this will output -1.123456E99 = 13 characters max including trailing nul
        dtostre(f, buf, 6, 0);
    }
    else
    {
        int decPos = 7 - (int)(floor(log10(a))+1.0f);
        dtostrf(f, 1, decPos, buf);
    
        if (decPos)
        {
            // remove trailing 0s
            char *p = buf;
            while (*p) p++;
            p--;
            while (*p == '0')
            {
                *p-- = 0;
            }
            
            if (*p == '.')
                *p = 0;
        }   
    }
    
    return buf;
}

void host_outputFloat(float f)
{
    char buf[16];
    host_outputString(host_floatToStr(f, buf));
}

void host_newLine()
{
    curX = 0;
    curY++;

    if (curY == SCREEN_HEIGHT)
        scrollBuffer();
    
    memset(screenBuffer + SCREEN_WIDTH*(curY), 32, SCREEN_WIDTH);
    lineDirty[curY] = 1;
}

char *host_readLine()
{
    inputMode = 1;

    if (curX == 0) 
        memset(screenBuffer + SCREEN_WIDTH*(curY), 32, SCREEN_WIDTH);
    else 
        host_newLine();

    int startPos = curY * SCREEN_WIDTH + curX;
    int pos = startPos;
    bool done = false;

    while (!done) 
    {
#ifdef SERIAL_TERM_IN_USE
        while(Serial.available() > 0)
#endif

#ifdef KEYPAD_8x5_IN_USE
        if(pressed_key_char != 0)
#endif
        {
            host_click();

            // read the next key
            lineDirty[pos / SCREEN_WIDTH] = 1;

#ifdef SERIAL_TERM_IN_USE
            char c = Serial.read();

            if (c >= 32 && c <= 126)
                screenBuffer[pos++] = c;
            else if (c == SERIAL_DELETE && pos > startPos)
                 screenBuffer[--pos] = 0;
            else if (c == SERIAL_CR)
                done = true;
#endif

#ifdef KEYPAD_8x5_IN_USE
            if (pressed_key_char >= 32 && pressed_key_char <= 126)
            {
                screenBuffer[pos++] = pressed_key_char;
            }
//            else if (pressed_key_char == SERIAL_DELETE && pos > startPos)
//                 screenBuffer[--pos] = 0;
            else if (pressed_key_char == KEY_ENTER)
            {
                done = true;
            }

            pressed_key_char = 0;
#endif
            curX = pos % SCREEN_WIDTH;
            curY = pos / SCREEN_WIDTH;

            // scroll if we need to
            if (curY == SCREEN_HEIGHT)
            {
                if (startPos >= SCREEN_WIDTH)
                {
                    startPos -= SCREEN_WIDTH;
                    pos -= SCREEN_WIDTH;
                    scrollBuffer();
                }
                else
                {
                    screenBuffer[--pos] = 0;
                    curX = pos % SCREEN_WIDTH;
                    curY = pos / SCREEN_WIDTH;
                }
            }
            
            redraw = 1;
        }
        
        if (redraw)
            host_showBuffer();
    }
    
    screenBuffer[pos] = 0;
    inputMode = 0;

    // remove the cursor
    lineDirty[curY] = 1;
    host_showBuffer();
    return &screenBuffer[startPos];
}


char host_getKey()
{
    char c = inkeyChar;
    inkeyChar = 0;

    if (c >= 32 && c <= 126)
        return c;
    else 
        return 0;
}

bool host_ESCPressed()
{
#ifdef SERIAL_TERM_IN_USE
    while(Serial.available() > 0)
#endif
    {

        // read the next key
#ifdef SERIAL_TERM_IN_USE
        inkeyChar = Serial.read();
    
        if (inkeyChar == SERIAL_ESC)
            return true;
#endif 
    }
    return false;
}


void host_outputFreeMem(unsigned int val)
{
    host_newLine();
    host_outputInt(val);
    host_outputChar(' ');
    host_outputProgMemString(bytesFreeStr);      
}

void host_saveProgram(bool autoexec)
{
    EEPROM.write(0, autoexec ? MAGIC_AUTORUN_NUMBER : 0x00);
    EEPROM.write(1, sysPROGEND & 0xFF);
    EEPROM.write(2, (sysPROGEND >> 8) & 0xFF);

    for (int i=0; i<sysPROGEND; i++)
        EEPROM.write(3+i, mem[i]);
}

void host_loadProgram()
{
    // skip the autorun byte
    sysPROGEND = EEPROM.read(1) | (EEPROM.read(2) << 8);

    for (int i=0; i<sysPROGEND; i++)
        mem[i] = EEPROM.read(i+3);
}

