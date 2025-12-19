#ifndef SPRITE_H
#define SPRITE_H
#include <stdint.h>

// Anchor point of the sprite is Upper Left corner of sprite
// posX and posY point to anchor point
struct sprite {
  uint16_t posX;
  uint16_t posY;
  const char *data;
  uint8_t color;
};

typedef struct sprite sprite_t;

#endif