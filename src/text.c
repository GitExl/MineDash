#include <cx16.h>
#include <cbm.h>
#include <string.h>

static unsigned char len;

void text_load() {

  // 64x32 layer1, text
  VERA.layer1.config = 0b00010000;
  VERA.layer1.mapbase = 0xD8;
  VERA.layer1.tilebase = 0xF8 | 0b00;
  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;

  // Load font.
  cbm_k_setnam("text.til");
  cbm_k_setlfs(0, 8, 1);
  cbm_k_load(3, 0);
}

void text_clear() {
  register unsigned char i = 240;

  VERA.address = 0xB000;
  VERA.address_hi = 0x01 | VERA_INC_1;
  do {
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;

    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
    VERA.data0 = 0;
  } while (--i);
}

void text_hud_clear() {
  register unsigned char i = 128;

  VERA.address = 0xBD00;
  VERA.address_hi = 0x01 | VERA_INC_1;
  do {
    VERA.data0 = 0x20;
    VERA.data0 = 0b11110010;
    VERA.data0 = 0x20;
    VERA.data0 = 0b11110010;
    VERA.data0 = 0x20;
    VERA.data0 = 0b11110010;
    VERA.data0 = 0x20;
    VERA.data0 = 0b11110010;
  } while (--i);
}

void text_write(const unsigned char x, const unsigned char y, const unsigned char colors, char* str) {
  VERA.address = 0xB000 + (y * 128) + x * 2;
  VERA.address_hi = 0x01 | VERA_INC_1;
  do {
    VERA.data0 = *str;
    VERA.data0 = colors;
    ++str;
  } while (*str);
}

void text_write_center(const unsigned char y, const unsigned char colors, char* str) {
  len = strlen(str) >> 1;
  text_write(20 - len, y, colors, str);
}
