#include "level.h"
#include "text.h"

void info_show() {
  switch (level_current) {
    case 0:
      text_write(10, 10, 0b10001010, "collect enough gems");
      text_write(10, 11, 0b10001010, "to open the exit.");
      text_write(10, 12, 0b10001010, "beware of falling");
      text_write(10, 13, 0b10001010, "boulders!");
      break;

    case 1:
      text_write(10, 11, 0b10001010, "use \x08A+\x08F to touch");
      text_write(10, 12, 0b10001010, "a nearby tile.");
      break;

    case 2:
      text_write(10, 10, 0b10001010, "press \x08B+\x08F to place");
      text_write(10, 11, 0b10001010, "tnt. it will explode");
      text_write(10, 12, 0b10001010, "very quickly!");
      break;

    case 3:
      text_write(10, 10, 0b10001010, "watch how they move");
      text_write(10, 11, 0b10001010, "and use boulders to");
      text_write(10, 12, 0b10001010, "kill them if needed.");
      break;

    default:
      text_write(10, 10, 0b10001010, "nope");
  }

  text_write(27, 15, 0b10001001, "\x08C\x08D\x08E");
}
