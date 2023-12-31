#include <time.h>
#include <cx16.h>
#include <cbm.h>
#include <string.h>

#include "camera.h"
#include "level.h"
#include "entities.h"
#include "text.h"
#include "input.h"
#include "random.h"
#include "main.h"
#include "level_names.h"
#include "sfx.h"

unsigned char game_state = GAMESTATE_LEVEL;
unsigned char game_action = GAMEACTION_LOAD_LEVEL;

void main() {
  clock_t start;


  text_clear();
  text_load();

  // Load palette.
  cbm_k_setnam("main.pal");
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(3, 0xFA00);

  // Setup display.
  // 320x240, l0, l1, sprites
  VERA.display.hscale = 64;
  VERA.display.vscale = 64;
  VERA.display.hstop = 64;
  VERA.display.vstop = 128;
  VERA.display.video = 0b00100001;

  text_box(8, 9, 24, 7, 0b10001001);
  text_write_center(12, 0b10001010, "loading game");

  random_init();

  sfx_init();
  sfx_load("lvl.sfx");

  entities_load_states("lvl.sta");
  level_load_graphics();


  while(1) {
    if (game_action != GAMEACTION_NONE) {
      switch (game_action) {

        // Load new level.
        case GAMEACTION_LOAD_LEVEL:
          VERA.display.video &= ~0b01010000;

          text_hud_clear();
          text_box(8, 9, 24, 7, 0b10001001);
          text_write_center(11, 0b10001010, "entering");
          text_write_center(13, 0b10000110, level_names[level_next]);

          level_load(level_next);
          camera_update();

          text_clear();
          level_hud_build();

          VERA.display.video |= 0b01010000;

          break;
      }

      game_action = GAMEACTION_NONE;
    }

    input_read();
    entities_update();
    level_update();
    sfx_update();

    // Center camera on player.
    camerax = (entities.tile_x[entity_player] << 4) + entities.p_x[entity_player] - CAMERA_WIDTH_HALF;
    cameray = (entities.tile_y[entity_player] << 4) + entities.p_y[entity_player] - CAMERA_HEIGHT_HALF;
    camera_update();

    // Update VERA sprite positions.
    entities_update_vera_sam();

    // Wait for vsync.
    start = clock();
    do {} while (start == clock());
  }
}
