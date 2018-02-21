#include "config.h"

#ifdef SSD1306ASCII_OLED_DISPLAY_IN_USE
#include <font.h>
#include <SSD1306ASCII.h>
// ^ - modified for faster SPI
#endif

#ifdef I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#endif

#ifdef PS2_KEYBOARD_IN_USE
#include <PS2Keyboard.h>
#endif

#include <EEPROM.h>

#include "basic.h"
#include "host.h"

// Define in host.h if using an external EEPROM e.g. 24LC256
// Should be connected to the I2C pins
// SDA -> Analog Pin 4, SCL -> Analog Pin 5
// See e.g. http://www.hobbytronics.co.uk/arduino-external-eeprom

// If using an external EEPROM, you'll also have to initialise it by
// running once with the appropriate lines enabled in setup() - see below

#if EXTERNAL_EEPROM
#include <I2cMaster.h>
// Instance of class for hardware master with pullups enabled
TwiMaster rtc(true);
#endif

#ifdef PS2_KEYBOARD_IN_USE
// Keyboard
const int DataPin = 8;
const int IRQpin =  3;
PS2Keyboard keyboard;
#endif

#ifdef SSD1306ASCII_OLED_DISPLAY_IN_USE
// OLED
#define OLED_DATA 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RST 13
SSD1306ASCII oled(OLED_DATA, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);
#endif

// NB Keyboard needs a seperate ground from the OLED

#ifdef I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE
// LCD
#define LCD_SERIAL_ADDRESS                      0x27
#define LCD_COLS                                16
#define LCD_ROWS                                2

LiquidCrystal_I2C lcd(LCD_SERIAL_ADDRESS, LCD_COLS, LCD_ROWS);
#endif

// buzzer pin, 0 = disabled/not present
#define BUZZER_PIN    5

// BASIC
unsigned char mem[MEMORY_SIZE];
#define TOKEN_BUF_SIZE    64
unsigned char tokenBuf[TOKEN_BUF_SIZE];

const char welcomeStr[] PROGMEM = "Arduino BASIC";
char autorun = 0;

void setup() {
#ifdef PS2_KEYBOARD_IN_USE
    keyboard.begin(DataPin, IRQpin);
#endif

#ifdef SSD1306ASCII_OLED_DISPLAY_IN_USE
    oled.ssd1306_init(SSD1306_SWITCHCAPVCC);
#endif

#ifdef SERIAL_TERM_IN_USE
    Serial.begin(19200);
    while (!Serial);
#endif

#ifdef I2C_LCD1602_LCD_16x2_DISPLAY_IN_USE
    lcd.init();
#endif

    reset();
    host_init(BUZZER_PIN);
    host_cls();
    host_outputProgMemString(welcomeStr);
    // show memory size
    host_outputFreeMem(sysVARSTART - sysPROGEND);
    host_showBuffer();
    delay(1500);

    // IF USING EXTERNAL EEPROM
    // The following line 'wipes' the external EEPROM and prepares
    // it for use. Uncomment it, upload the sketch, then comment it back
    // in again and upload again, if you use a new EEPROM.
    // writeExtEEPROM(0,0); writeExtEEPROM(1,0);

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
            host_outputFreeMem(sysVARSTART - sysPROGEND);
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

