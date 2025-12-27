#include "video.h"
#include "font8x8_basic.h"
#include "memory.h"

// Macros
#define BPP framebuffer_info.bitsPerPixel

// Internal state flags
static uint8_t flags = 0x00;
#define FLAGS_INIT 0x01

// Frame buffer info struct
static framebuffer_info_t framebuffer_info;

// Sets up the video framebuffer settings
void video_init(framebuffer_info_t *pFrame_buffer_info) {
  framebuffer_info = *pFrame_buffer_info;
  flags |= (FLAGS_INIT & 1);
}

// Clear video framebuffer
void clear_screen(color_t color) {
  uint8_t bpp = framebuffer_info.bitsPerPixel;
  volatile uint8_t *base_addr =
      (volatile uint8_t *)(uintptr_t)framebuffer_info.addr;
  uint32_t total_pixels = framebuffer_info.width * framebuffer_info.height;

  // Handle different bits per pixels
  switch (bpp) {
  case 32: {
    uint32_t packed_col = (color.r << framebuffer_info.red_pos) |
                          (color.g << framebuffer_info.green_pos) |
                          (color.b << framebuffer_info.blue_pos);

    // Treat the framebuffer as a linear array of 32-bit integers
    volatile uint32_t *dest = (volatile uint32_t *)base_addr;

    // Optimization: Process 4 pixels at a time (Loop Unrolling)
    uint32_t count = total_pixels;
    while (count >= 4) {
      dest[0] = packed_col;
      dest[1] = packed_col;
      dest[2] = packed_col;
      dest[3] = packed_col;
      dest += 4;
      count -= 4;
    }
    // Handle remaining pixels
    while (count--) {
      *dest++ = packed_col;
    }
    break;
  }

  case 24: {
    // 24bpp must be written byte-by-byte as pixels are 3 bytes wide
    for (uint32_t i = 0; i < total_pixels; ++i) {
      base_addr[i * 3 + 0] = color.b;
      base_addr[i * 3 + 1] = color.g;
      base_addr[i * 3 + 2] = color.r;
    }
    break;
  }

  case 16: {
    uint16_t packed_col =
        (uint16_t)(((color.r >> (8 - framebuffer_info.red_mask_size))
                    << framebuffer_info.red_pos) |
                   ((color.g >> (8 - framebuffer_info.green_mask_size))
                    << framebuffer_info.green_pos) |
                   ((color.b >> (8 - framebuffer_info.blue_mask_size))
                    << framebuffer_info.blue_pos));

    volatile uint16_t *dest = (volatile uint16_t *)base_addr;
    for (uint32_t i = 0; i < total_pixels; ++i) {
      dest[i] = packed_col;
    }
    break;
  }
  }
}

// Draw pixel to screen
void video_draw_pixel(uint16_t x, uint16_t y, color_t color) {
  // Calculate address
  uint8_t bpp = framebuffer_info.bitsPerPixel;
  volatile uint8_t *pixel_addr =
      (volatile uint8_t *)(uintptr_t)framebuffer_info.addr +
      (y * framebuffer_info.pitch) + (x * (bpp >> 3));

  // Re-pack color based on hardware masks
  // Shifting right by (8 - mask_size) scales the 8-bit color down to hardware
  // depth
  uint32_t packed_color =
      ((uint32_t)(color.r >> (8 - framebuffer_info.red_mask_size))
       << framebuffer_info.red_pos) |
      ((uint32_t)(color.g >> (8 - framebuffer_info.green_mask_size))
       << framebuffer_info.green_pos) |
      ((uint32_t)(color.b >> (8 - framebuffer_info.blue_mask_size))
       << framebuffer_info.blue_pos);

  // Handle different cases for different modes

  if (bpp == 32) {
    *(volatile uint32_t *)pixel_addr = packed_color;
  } else if (bpp == 16 || bpp == 15) {
    *(volatile uint16_t *)pixel_addr = (uint16_t)packed_color;
  } else if (bpp == 24) {
    // 24-bit is rare but dangerous; must write 3 bytes to avoid buffer overflow
    pixel_addr[0] = (packed_color >> 0) & 0xFF;
    pixel_addr[1] = (packed_color >> 8) & 0xFF;
    pixel_addr[2] = (packed_color >> 16) & 0xFF;
  }
}

// Draw character to screen
void video_draw_char(char c, uint16_t x, uint16_t y, color_t color) {
  // Safety Bounds Check: Ensure we don't draw outside the framebuffer
  // 8U -> So that x is casted up to int before adding
  if (x + 8U > framebuffer_info.width || y + 8U > framebuffer_info.height) {
    return;
  }
  if (c == ' ') {
    return video_clear_char(x, y, color);
  }

  uint8_t *glyph = font8x8_basic[(uint8_t)c];

  uint8_t bpp = framebuffer_info.bitsPerPixel;

  // Calculate base byte address once for all modes
  uint8_t *base_addr = (uint8_t *)(uintptr_t)framebuffer_info.addr +
                       (y * framebuffer_info.pitch) + (x * (bpp >> 3));

  switch (bpp) {
  case 32: {
    // 32bpp (True Color / Aligned)
    volatile uint32_t *dest = (volatile uint32_t *)base_addr;
    uint32_t stride = framebuffer_info.pitch >> 2; // Pixel-stride

    uint32_t packed_col = (color.r << framebuffer_info.red_pos) |
                          (color.g << framebuffer_info.green_pos) |
                          (color.b << framebuffer_info.blue_pos);

    for (int i = 0; i < 8; i++) {
      uint8_t row = glyph[i];
      dest[0] = ((row >> 0) & 1) * packed_col;
      dest[1] = ((row >> 1) & 1) * packed_col;
      dest[2] = ((row >> 2) & 1) * packed_col;
      dest[3] = ((row >> 3) & 1) * packed_col;
      dest[4] = ((row >> 4) & 1) * packed_col;
      dest[5] = ((row >> 5) & 1) * packed_col;
      dest[6] = ((row >> 6) & 1) * packed_col;
      dest[7] = ((row >> 7) & 1) * packed_col;
      dest += stride;
    }
    break;
  }

  case 16: {
    // 16bpp (High Color / Aligned)
    volatile uint16_t *dest = (volatile uint16_t *)base_addr;
    uint32_t stride = framebuffer_info.pitch >> 1; // Pixel-stride

    uint16_t col = (uint16_t)((((color.r >> 16) & 0xFF) >>
                               (8 - framebuffer_info.red_mask_size)
                                   << framebuffer_info.red_pos) |
                              (((color.g >> 8) & 0xFF) >>
                               (8 - framebuffer_info.green_mask_size)
                                   << framebuffer_info.green_pos) |
                              (((color.b >> 0) & 0xFF) >>
                               (8 - framebuffer_info.blue_mask_size)
                                   << framebuffer_info.blue_pos));

    for (int i = 0; i < 8; i++) {
      uint8_t row = glyph[i];
      dest[0] = ((row >> 0) & 1) * col;
      dest[1] = ((row >> 1) & 1) * col;
      dest[2] = ((row >> 2) & 1) * col;
      dest[3] = ((row >> 3) & 1) * col;
      dest[4] = ((row >> 4) & 1) * col;
      dest[5] = ((row >> 5) & 1) * col;
      dest[6] = ((row >> 6) & 1) * col;
      dest[7] = ((row >> 7) & 1) * col;
      dest += stride;
    }
    break;
  }

  case 24: {
    // 24bpp (Packed Pixels / Unaligned)
    volatile uint8_t *dest = (volatile uint8_t *)base_addr;

    for (int i = 0; i < 8; i++) {
      uint8_t row = glyph[i];
      for (int col = 0; col < 8; col++) {
        uint8_t mask = (row >> col) & 1;

        dest[col * 3 + 0] = mask * color.b;
        dest[col * 3 + 1] = mask * color.g;
        dest[col * 3 + 2] = mask * color.r;
      }
      dest += framebuffer_info.pitch;
    }
    break;
  }
  }
}

void video_clear_char(uint16_t x, uint16_t y,
                      color_t color) { // Safety Bounds Check: Ensure we don't
                                       // draw outside the framebuffer
  // 8U -> So that x is casted up to int before adding
  if (x + 8U > framebuffer_info.width || y + 8U > framebuffer_info.height) {
    return;
  }
  uint8_t bpp = framebuffer_info.bitsPerPixel;

  // Calculate base byte address once for all modes
  uint8_t *base_addr = (uint8_t *)(uintptr_t)framebuffer_info.addr +
                       (y * framebuffer_info.pitch) + (x * (bpp >> 3));

  switch (bpp) {
  case 32: {
    // 32bpp (True Color / Aligned)
    volatile uint32_t *dest = (volatile uint32_t *)base_addr;
    uint32_t stride = framebuffer_info.pitch >> 2; // Pixel-stride

    uint32_t packed_col = (color.r << framebuffer_info.red_pos) |
                          (color.g << framebuffer_info.green_pos) |
                          (color.b << framebuffer_info.blue_pos);

    for (int i = 0; i < 8; i++) {
      dest[0] = packed_col;
      dest[1] = packed_col;
      dest[2] = packed_col;
      dest[3] = packed_col;
      dest[4] = packed_col;
      dest[5] = packed_col;
      dest[6] = packed_col;
      dest[7] = packed_col;
      dest += stride;
    }
    break;
  }

  case 16: {
    // 16bpp (High Color / Aligned)
    volatile uint16_t *dest = (volatile uint16_t *)base_addr;
    uint32_t stride = framebuffer_info.pitch >> 1; // Pixel-stride

    uint16_t col = (uint16_t)((((color.r >> 16) & 0xFF) >>
                               (8 - framebuffer_info.red_mask_size)
                                   << framebuffer_info.red_pos) |
                              (((color.g >> 8) & 0xFF) >>
                               (8 - framebuffer_info.green_mask_size)
                                   << framebuffer_info.green_pos) |
                              (((color.b >> 0) & 0xFF) >>
                               (8 - framebuffer_info.blue_mask_size)
                                   << framebuffer_info.blue_pos));

    for (int i = 0; i < 8; i++) {
      dest[0] = col;
      dest[1] = col;
      dest[2] = col;
      dest[3] = col;
      dest[4] = col;
      dest[5] = col;
      dest[6] = col;
      dest[7] = col;
      dest += stride;
    }
    break;
  }

  case 24: {
    // 24bpp (Packed Pixels / Unaligned)
    volatile uint8_t *dest = (volatile uint8_t *)base_addr;

    for (int i = 0; i < 8; i++) {
      for (int col = 0; col < 8; col++) {
        dest[col * 3 + 0] = color.b;
        dest[col * 3 + 1] = color.g;
        dest[col * 3 + 2] = color.r;
      }
      dest += framebuffer_info.pitch;
    }
    break;
  }
  }
}
