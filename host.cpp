#include "host.h"
#include "basic.h"

#include <SSD1306ASCII.h>
#include <PS2Keyboard.h>
#include <EEPROM.h>

extern SSD1306ASCII oled;
extern PS2Keyboard keyboard;
extern EEPROMClass EEPROM;
int timer1_counter;

char screenBuffer[SCREEN_WIDTH*SCREEN_HEIGHT];
char lineDirty[SCREEN_HEIGHT];
int curX = 0, curY = 0;
volatile char flash = 0, redraw = 0;
char inputMode = 0;
char inkeyChar = 0;
char buzPin = 0;

void initTimer() {
    noInterrupts();           // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
    TCNT1 = timer1_counter;   // preload timer
    TCCR1B |= (1 << CS12);    // 256 prescaler 
    TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
    interrupts();             // enable all interrupts
}

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
    TCNT1 = timer1_counter;   // preload timer
    flash = !flash;
    redraw = 1;
}


void host_init(int buzzerPin) {
    buzPin = buzzerPin;
    oled.clear();
    if (buzPin)
        pinMode(buzPin, OUTPUT);
    initTimer();
}

void host_sleep(long ms) {
    delay(ms);
}

void host_digitalWrite(int pin,int state) {
    digitalWrite(pin, state ? HIGH : LOW);
}

int host_digitalRead(int pin) {
    return digitalRead(pin);
}

int host_analogRead(int pin) {
    return analogRead(pin);
}

void host_pinMode(int pin,int mode) {
    pinMode(pin, mode);
}

void host_click() {
    if (!buzPin) return;
    digitalWrite(buzPin, HIGH);
    delay(1);
    digitalWrite(buzPin, LOW);
}

void host_cls() {
    memset(screenBuffer, 32, SCREEN_WIDTH*SCREEN_HEIGHT);
    memset(lineDirty, 1, SCREEN_HEIGHT);
    curX = 0;
    curY = 0;
}

void host_moveCursor(int x, int y) {
    if (x<0) x = 0;
    if (x>=SCREEN_WIDTH) x = SCREEN_WIDTH-1;
    if (y<0) y = 0;
    if (y>=SCREEN_HEIGHT) y = SCREEN_HEIGHT-1;
    curX = x;
    curY = y; 
}

void host_showBuffer() {
    for (int y=0; y<SCREEN_HEIGHT; y++) {
        if (lineDirty[y] || (inputMode && y==curY)) {
            oled.setCursor(0,y);
            for (int x=0; x<SCREEN_WIDTH; x++) {
                char c = screenBuffer[y*SCREEN_WIDTH+x];
                if (c<32) c = ' ';
                if (x==curX && y==curY && inputMode && flash) c = 127;
                oled.print(c);
            }
            lineDirty[y] = 0;
        }
    }
}

void scrollBuffer() {
    memcpy(screenBuffer, screenBuffer + SCREEN_WIDTH, SCREEN_WIDTH*(SCREEN_HEIGHT-1));
    memset(screenBuffer + SCREEN_WIDTH*(SCREEN_HEIGHT-1), 32, SCREEN_WIDTH);
    memset(lineDirty, 1, SCREEN_HEIGHT);
    curY--;
}

void host_outputString(char *str) {
    int pos = curY*SCREEN_WIDTH+curX;
    while (*str) {
        lineDirty[pos / SCREEN_WIDTH] = 1;
        screenBuffer[pos++] = *str++;
        if (pos >= SCREEN_WIDTH*SCREEN_HEIGHT) {
            scrollBuffer();
            pos -= SCREEN_WIDTH;
        }
    }
    curX = pos % SCREEN_WIDTH;
    curY = pos / SCREEN_WIDTH;
}

void host_outputProgMemString(const char *p) {
    while (1) {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0) break;
        host_outputChar(c);
    }
}

void host_outputChar(char c) {
    int pos = curY*SCREEN_WIDTH+curX;
    lineDirty[pos / SCREEN_WIDTH] = 1;
    screenBuffer[pos++] = c;
    if (pos >= SCREEN_WIDTH*SCREEN_HEIGHT) {
        scrollBuffer();
        pos -= SCREEN_WIDTH;
    }
    curX = pos % SCREEN_WIDTH;
    curY = pos / SCREEN_WIDTH;
}

int host_outputInt(long num) {
    // returns len
    long i = num, xx = 1;
    int c = 0;
    do {
        c++;
        xx *= 10;
        i /= 10;
    } 
    while (i);

    for (int i=0; i<c; i++) {
        xx /= 10;
        char digit = ((num/xx) % 10) + '0';
        host_outputChar(digit);
    }
    return c;
}

char *host_floatToStr(float f, char *buf) {
    // floats have approx 7 sig figs
    float a = fabs(f);
    if (f == 0.0f) {
        buf[0] = '0'; 
        buf[1] = 0;
    }
    else if (a<0.0001 || a>1000000) {
        // this will output -1.123456E99 = 13 characters max including trailing nul
        dtostre(f, buf, 6, 0);
    }
    else {
        int decPos = 7 - (int)(floor(log10(a))+1.0f);
        dtostrf(f, 1, decPos, buf);
        if (decPos) {
            // remove trailing 0s
            char *p = buf;
            while (*p) p++;
            p--;
            while (*p == '0') {
                *p-- = 0;
            }
            if (*p == '.') *p = 0;
        }   
    }
    return buf;
}

void host_outputFloat(float f) {
    char buf[16];
    host_outputString(host_floatToStr(f, buf));
}

void host_newLine() {
    curX = 0;
    curY++;
    if (curY == SCREEN_HEIGHT)
        scrollBuffer();
    memset(screenBuffer + SCREEN_WIDTH*(curY), 32, SCREEN_WIDTH);
    lineDirty[curY] = 1;
}

char *host_readLine() {
    inputMode = 1;

    if (curX == 0) memset(screenBuffer + SCREEN_WIDTH*(curY), 32, SCREEN_WIDTH);
    else host_newLine();

    int startPos = curY*SCREEN_WIDTH+curX;
    int pos = startPos;

    bool done = false;
    while (!done) {
        while (keyboard.available()) {
            host_click();
            // read the next key
            lineDirty[pos / SCREEN_WIDTH] = 1;
            char c = keyboard.read();
            if (c>=32 && c<=126)
                screenBuffer[pos++] = c;
            else if (c==PS2_DELETE && pos > startPos)
                screenBuffer[--pos] = 0;
            else if (c==PS2_ENTER)
                done = true;
            curX = pos % SCREEN_WIDTH;
            curY = pos / SCREEN_WIDTH;
            // scroll if we need to
            if (curY == SCREEN_HEIGHT) {
                if (startPos >= SCREEN_WIDTH) {
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

char host_getKey() {
    char c = inkeyChar;
    inkeyChar = 0;
    if (c >= 32 && c <= 126)
        return c;
    else return 0;
}

bool host_ESCPressed() {
    while (keyboard.available()) {
        // read the next key
        inkeyChar = keyboard.read();
        if (inkeyChar == PS2_ESC)
            return true;
    }
    return false;
}

void host_saveProgram(bool autoexec) {
    EEPROM.write(0, autoexec ? MAGIC_AUTORUN_NUMBER : 0x00);
    EEPROM.write(1, sysPROGEND & 0xFF);
    EEPROM.write(2, (sysPROGEND >> 8) & 0xFF);
    for (int i=0; i<sysPROGEND; i++)
        EEPROM.write(3+i, mem[i]);
}

void host_loadProgram() {
    // skip the autorun byte
    sysPROGEND = EEPROM.read(1) | (EEPROM.read(2) << 8);
    for (int i=0; i<sysPROGEND; i++)
        mem[i] = EEPROM.read(i+3);
}


