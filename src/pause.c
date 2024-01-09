#include <cx16.h>

#include "pause.h"
#include "text.h"
#include "input.h"
#include "main.h"
#include "sfx.h"
#include "sfx_labels.h"

#define CF_EXIT 0x01
#define CF_HOLD 0x02

#define CHOICE_MAX 2

static char* choice_text[] = {
  "resume",
  "restart level",
  "exit",
};

static unsigned char choice_flags[] = {
  0,
  CF_HOLD,
  CF_HOLD,
};

static unsigned char pause_choice;
static unsigned char pause_hold;

void pause_init() {
  pause_choice = 0;

  text_blind();
  text_box(10, 8, 20, 12, 0b10001001);
  text_write_center(10, 0b10001001, "paused");

  pause_redraw();
}

void pause_update() {
  if (input1_change) {
    pause_hold = 0;

    if (input1 & JOY_DOWN_MASK) {
      if (pause_choice == CHOICE_MAX) {
        pause_choice = 0;
      } else {
        ++pause_choice;
        sfx_play(SFX_LVL_BIP, 63, 63, 0xFF);
      }

    } else if (input1 & JOY_UP_MASK) {
      if (!pause_choice) {
        pause_choice = CHOICE_MAX;
      } else {
        --pause_choice;
        sfx_play(SFX_LVL_BIP, 63, 63, 0xFF);
      }

    }

    pause_redraw();
  }

  if (input2_change) {
    if (!(choice_flags[pause_choice] & CF_HOLD) && input2 & JOY_BTN_A_MASK) {
      pause_trigger();
      return;
    }
  }

  if (choice_flags[pause_choice] & CF_HOLD) {
    if (input2 & JOY_BTN_A_MASK) {
      ++pause_hold;
      if (pause_hold == 48) {
        pause_trigger();
        return;
      }
    } else if (pause_hold) {
      pause_hold = 0;
    }

    pause_redraw();
  }
}

void pause_redraw() {
  unsigned char y;
  unsigned char i;
  char* hold_str = "\x090";

  for (i = 0; i <= CHOICE_MAX; i++) {
    y = 13 + (i << 1);
    if (i == pause_choice) {
      if (pause_hold) {
        hold_str[0] = 0x90 + (pause_hold >> 2);
        text_write(12, y, 0b10001001, hold_str);
      } else {
        text_write(12, y, 0b10001001, " ");
      }
      text_write(14, y, 0b10000110, choice_text[i]);
    } else {
      text_write(12, y, 0b10001010, " ");
      text_write(14, y, 0b10001010, choice_text[i]);
    }
  }
}

void pause_trigger() {
  text_clear();

  if (pause_choice == 0) {
    game_state = GAMESTATE_LEVEL;
  } else if (pause_choice == 1) {
    game_action = GAMEACTION_LOAD_LEVEL;
  } else if (pause_choice == 2) {
    game_state = GAMESTATE_LEVEL;
  }
}
