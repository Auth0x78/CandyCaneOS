#ifndef RENDERER_H
#define RENDERER_H

#include "print.h"
#include "sprite.h"

// Draw a ascii sprite to screen at a certain position
// Returns: 1 => Error: Failed to draw sprite, 0 => Success
int8_t drawSprite(sprite_t *sprite);

#endif