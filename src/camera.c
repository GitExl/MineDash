#include <cx16.h>

#include "camera.h"
#include "sfx.h"
#include "level.h"

signed int camerax = 0;
signed int cameray = 0;

void camera_update() {
  if (camerax < 0 || camerax < level_info.left) {
    camerax = level_info.left;
  }
  else if (camerax > level_info.right) {
    camerax = level_info.right;
  }

  if (cameray < 0 || cameray < level_info.top) {
    cameray = level_info.top;
  }
  else if (cameray > level_info.bottom) {
    cameray = level_info.bottom;
  }

  VERA.layer0.hscroll = camerax;
  VERA.layer0.vscroll = cameray;

  // Update sfx listener position to player.
  sfx_listen_x = (camerax + CAMERA_WIDTH_HALF) >> 4;
  sfx_listen_y = (cameray + CAMERA_HEIGHT_HALF) >> 4;
}
