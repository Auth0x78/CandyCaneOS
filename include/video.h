#ifndef VIDEO_H
#define VIDEO_H
#include <stdint.h>

// Struct defines
// Color structure for passing color info
struct color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// Useful Color macro
#define COLOR(R, G, B) ((struct color){.r = R, .g = G, .b = B})
// Pre-defined color macros
#define COLOR_WHITE ((struct color){.r = 0xFF, .g = 0xFF, .b = 0xFF})
#define COLOR_BLACK ((struct color){.r = 0x00, .g = 0x00, .b = 0x00})
#define COLOR_RED ((struct color){.r = 0xFF, .g = 0x00, .b = 0x00})
#define COLOR_GREEN ((struct color){.r = 0x00, .g = 0xFF, .b = 0x00})
#define COLOR_BLUE ((struct color){.r = 0x00, .g = 0x00, .b = 0xFF})

// Framebuffer information structure
struct framebuffer_info {
  uint64_t addr;
  uint32_t pitch;
  uint32_t width;
  uint32_t height;

  uint8_t bitsPerPixel;

  // Channel Properties
  // Red Channel
  uint8_t red_pos;
  uint8_t red_mask_size;

  // Green Channel
  uint8_t green_pos;
  uint8_t green_mask_size;

  // Blue Channel
  uint8_t blue_pos;
  uint8_t blue_mask_size;
};

// Typedefs for structs used
typedef struct framebuffer_info framebuffer_info_t;
typedef struct color color_t;

// Initialize video driver
void video_init(framebuffer_info_t *pFrame_buffer_info);

// Clear screen
void clear_screen(color_t color);

// Draw pixel to screen
void video_draw_pixel(uint16_t x, uint16_t y, color_t color);

// Draw text to screen
void video_draw_char(char c, uint16_t x, uint16_t y, color_t color);

// Draw a 8x8 square
void video_clear_char(uint16_t x, uint16_t y, color_t color);

#endif