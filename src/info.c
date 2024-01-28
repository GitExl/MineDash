#include "level.h"
#include "text.h"

static char* info_0[] = {
  "collect enough gems",
  "to open the exit.",
  "beware of falling",
  "boulders!",
};

static char* info_1[] = {
  "",
  "use \x08A + \x08F to touch",
  "a nearby tile.",
  "",
};

static char* info_4[] = {
  "press \x08B + \x08F to place",
  "tnt. it will explode",
  "very quickly!",
  "",
};

static char* info_5[] = {
  "watch their movement.",
  "use boulders to kill",
  "them if needed.",
  "",
};

static char* info_7[] = {
  "",
  "  ghosts follow you.",
  "  don't get caught!",
  "",
};

void info_show() {
  unsigned char i;
  char** info;

  switch (level_current) {
    case 0: info = info_0; break;
    case 1: info = info_1; break;
    case 4: info = info_4; break;
    case 5: info = info_5; break;
    case 7: info = info_7; break;
  }

  for (i = 0; i < 4; i++) {
    text_write(10, 10 + i, 0b10001010, info[i]);
  }
  text_write(27, 15, 0b10001001, "\x08C\x08D\x08E");
}
