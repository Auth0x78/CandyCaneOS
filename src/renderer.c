#include "renderer.h"

int8_t drawSprite(sprite_t *sprite) {
  uint16_t x = sprite->posX;
  uint16_t y = sprite->posY;

  for (uint32_t i = 0; sprite->data[i] != '\0'; ++i) {
    switch (sprite->data[i]) {
    case '\n':
      y = (y + 1) % VGA_HEIGHT;
      break;
    case '\r':
      x = sprite->posX;
      break;
    default:
      putcAt(sprite->data[i], x, y, sprite->color);

      x += 1;
      y += (x == VGA_WIDTH) ? 1 : 0;
      y %= VGA_HEIGHT;
      x %= VGA_WIDTH;
    }
  }

  /*    cursor_y += (cursor_x + 1) == VGA_WIDTH ? 1 : 0;
    cursor_y %= VGA_HEIGHT;
    cursor_x = (cursor_x + 1) % VGA_WIDTH;*/
  return 0;
}