#include <cx16.h>
#include <cbm.h>
#include <string.h>

#include "text.h"

static unsigned char len;
static unsigned char i, j;

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
  i = 240;

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

void text_clear_line(const unsigned char line) {
  i = 64;

  VERA.address = 0xB000 + line * 128;
  VERA.address_hi = 0x01 | VERA_INC_1;
  do {
    VERA.data0 = 0;
    VERA.data0 = 0;
  } while (--i);
}

void text_hud_clear() {
  i = 128;

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

void text_box(const unsigned char x, const unsigned char y, unsigned char width, unsigned char height, const unsigned char colors) {
  const unsigned int step = 128 - (width * 2);

  width -= 2;
  height -= 2;

  VERA.address = 0xB000 + (y * 128) + (x * 2);
  VERA.address_hi = 0x01 | VERA_INC_1;

  // Top.
  VERA.data0 = 0x70;
  VERA.data0 = colors;
  for (i = 0; i < width; i++) {
    VERA.data0 = 0x71;
    VERA.data0 = colors;
  }
  VERA.data0 = 0x72;
  VERA.data0 = colors;

  // Sides.
  for (i = 0; i < height; i++) {
    VERA.address += step;

    VERA.data0 = 0x76;
    VERA.data0 = colors;
    for (j = 0; j < width; j++) {
      VERA.data0 = 0x00;
      VERA.data0 = colors;
    }
    VERA.data0 = 0x77;
    VERA.data0 = colors;
  }

  // Bottom.
  VERA.address += step;
  VERA.data0 = 0x73;
  VERA.data0 = colors;
  for (i = 0; i < width; i++) {
    VERA.data0 = 0x74;
    VERA.data0 = colors;
  }
  VERA.data0 = 0x75;
  VERA.data0 = colors;
}
