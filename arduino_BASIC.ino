#include <font.h>
#include <SSD1306ASCII.h>
// ^ - modified for faster SPI
#include <PS2Keyboard.h>
#include <EEPROM.h>

#include "basic.h"
#include "host.h"

// Keyboard
const int DataPin = 8;
const int IRQpin =  3;
PS2Keyboard keyboard;

// OLED
#define OLED_DATA 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RST 13
SSD1306ASCII oled(OLED_DATA, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

// NB Keyboard needs a seperate ground from the OLED

// buzzer pin, 0 = disabled/not present
#define BUZZER_PIN    5

// basic
unsigned char mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    50
unsigned char tokenBuf[TOKEN_BUF_SIZE];

prog_char welcomeStr[] PROGMEM = "Arduino BASIC";
prog_char bytesFreeStr[] PROGMEM = "bytes free";
char autorun = 0;

void outputFreeMem() {
    host_newLine();
    host_outputInt(sysVARSTART - sysPROGEND);
    host_outputChar(' ');
    host_outputProgMemString(bytesFreeStr);      
}

void setup() {
    keyboard.begin(DataPin, IRQpin);
    oled.ssd1306_init(SSD1306_SWITCHCAPVCC);

    reset();
    host_init(BUZZER_PIN);
    host_cls();
    host_outputProgMemString(welcomeStr);
    // show memory size
    outputFreeMem();
    host_showBuffer();
    if (EEPROM.read(0) == MAGIC_AUTORUN_NUMBER)
        autorun = 1;
    else
        host_startupTone();
}

void loop() {
    int ret = ERROR_NONE;

    if (!autorun) {
        // get a line from the user
        char *input = host_readLine();
        // special editor commands
        if (input[0] == '?' && input[1] == 0) {
            outputFreeMem(); 
            host_showBuffer();
            return;
        }
        // otherwise tokenize
        ret = tokenize((unsigned char*)input, tokenBuf, TOKEN_BUF_SIZE);
    }
    else {
        host_loadProgram();
        tokenBuf[0] = TOKEN_RUN;
        tokenBuf[1] = 0;
        autorun = 0;
    }
    // execute the token buffer
    if (ret == ERROR_NONE) {
        host_newLine();
        ret = processInput(tokenBuf);
    }
    if (ret != ERROR_NONE) {
        host_newLine();
        if (lineNumber !=0) {
            host_outputInt(lineNumber);
            host_outputChar('-');
        }
        host_outputProgMemString((char *)pgm_read_word(&(errorTable[ret])));
    }
}

