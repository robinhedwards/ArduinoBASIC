#include "host.h"
#include "basic.h"

#define PS2_DELETE  0x7f
#define PS2_BS      0x08
#define PS2_ENTER   0x0d
#define PS2_ESC     0x1b
#define PS2_CTRL_C  0x03

char lineBuffer[MAXTEXTLEN];    // ラインバッファ

int curX = 0;
volatile char flash = 1, redraw = 1;
char inputMode = 0;
char inkeyChar = 0;

const char bytesFreeStr[] PROGMEM = "bytes free";

// ホスト画面の初期化
void host_init() {

}

// スリープ
void host_sleep(long ms) {
  delay(ms);
}

// デジタル出力
void host_digitalWrite(int pin,int state) {
  digitalWrite(pin, state ? HIGH : LOW);
}

// デジタル入力
int host_digitalRead(int pin) {
  return digitalRead(pin);
}

// アナログ入力
int host_analogRead(int pin) {
  return analogRead(pin);
}

// ピンモードの設定
void host_pinMode(int pin, WiringPinMode mode) {
  pinMode(pin, mode);
}

// 文字入出力
#define c_getch()     Serial.read()
#define c_kbhit()     Serial.available()
#define c_putc(c)     Serial.write(c)
#define c_newLine(c)  Serial.println();

// スクリーンクリア
void host_cls() {
  memset(lineBuffer, 32, MAXTEXTLEN);  // ラインバッファのクリア
  curX = 0;                            // カーソル初期化
}

// 画面バッファに文字列を出力
void host_outputString(char *str) {
  int pos = curX;
  while (*str) {
    if (pos < MAXTEXTLEN-1) {
      lineBuffer[pos++] = *str;
      c_putc(*str);
    }
    str++;
  }
  curX = pos;
}

// フラシュメモリ上の文字列を出力
void host_outputProgMemString(const char *p) {
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    host_outputChar(c);
  }
}

// 文字を出力
void host_outputChar(char c) {
  int pos = curX;
  if (pos < MAXTEXTLEN-1) {
     lineBuffer[pos++] = c;
     c_putc(c);
  }
  curX = pos;
}

// 数値を出力
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

// フロート型を文字列変換
char *host_floatToStr(float f, char *buf) {
  // floats have approx 7 sig figs
  float a = fabs(f);
  if (f == 0.0f) {
    buf[0] = '0'; 
    buf[1] = 0;
  } else if (a<0.0001 || a>1000000) {
    // this will output -1.123456E99 = 13 characters max including trailing nul
#if defined(__AVR__) 
   dtostre(f, buf, 6, 0);
#else
   sprintf(buf, "%6.2e", f);
#endif
  } else {
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

// フロート型数値出力
void host_outputFloat(float f) {
  char buf[16];
  host_outputString(host_floatToStr(f, buf));
}

// 改行
void host_newLine() {
  curX = 0;
  memset(lineBuffer, 32, MAXTEXTLEN);
  c_newLine(c);
}

// ライン入力
char *host_readLine() {
  inputMode = 1;

  if (curX == 0) {
    // 行先頭なら、その行のバッファクリア
    memset(lineBuffer, 32, MAXTEXTLEN);
  } else {
    // そうでないなら、改行して次の行から入力
    host_newLine();
  }
  
  int startPos = curX; // バッファ書き込み先頭位置
  int pos = startPos;  // バッファ書き込み位置

  bool done = false;
  while (!done) {
    while (c_kbhit()) {
      // read the next key
      char c = c_getch();
      if (c>=32 && c<=126) {
        // 通常の文字の場合
        lineBuffer[pos++] = c;             // 画面バッファに書き込み
        c_putc(c);
      } else if ( (c==PS2_DELETE || c==PS2_BS) && pos > startPos) {
        // [DELETE]/[BS]の場合、１文字削除
        lineBuffer[--pos] = 0;
        c_putc(PS2_BS);c_putc(' ');c_putc(PS2_BS); //文字を消す
      } else if (c==PS2_ENTER) {
        // [ENTER]の場合、入力確定
        done = true;        
      }
      // カーソル更新
      curX = pos;
    }
  }
  
  lineBuffer[pos] = 0;         // 文字列終端設定
  inputMode = 0;
  return &lineBuffer[startPos]; // 入力文字列を返す
}

char host_getKey() {
  char c = inkeyChar;
  inkeyChar = 0;
  if (c >= 32 && c <= 126)
      return c;
  else return 0;
}

// 中断キー入力チェック
bool host_ESCPressed() {
  while (c_kbhit()) {
    inkeyChar = c_getch();
    if ( (inkeyChar == PS2_ESC) || (inkeyChar == PS2_CTRL_C))
      return true;
  }
  return false;
}

// 空き領域の表示
void host_outputFreeMem(unsigned int val) {
  host_newLine();
  host_outputInt(val);
  host_outputChar(' ');
  host_outputProgMemString(bytesFreeStr);      
}
