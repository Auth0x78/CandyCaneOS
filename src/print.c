#include "print.h"

// Screen size (in char)
static uint16_t max_char_x = 0;
static uint16_t max_char_y = 0;

// NOTE: Screen coordinates and char coordinates are different
// Current character coordinates of cursor
static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

// Font resolution used
static uint8_t font_size_x = 0;
static uint8_t font_size_y = 0;

// Current color of the text
static color_t default_color_mode = COLOR(0xFF, 0xFF, 0xFF);
static color_t background_color = COLOR_BLACK;

// Helper function declaration
static int16_t print_int(uint32_t value, uint8_t base, bool signed_type,
                         uint8_t bytes);

// Initalizes the printer
void print_init(uint16_t screen_width, uint16_t screen_height,
                uint8_t font_width, uint8_t font_height, color_t colorMode) {
  cursor_x = 0;
  cursor_y = 0;

  // Font resolution
  font_size_x = font_width;
  font_size_y = font_height;

  // Max characters per axis
  max_char_x = screen_width / font_width;
  max_char_y = screen_height / font_height;

  // Set default color of print
  default_color_mode = colorMode;

  // Also clear screen for printing
  background_color = COLOR_BLACK;
  clear_screen(background_color);
}

// Clears the print window
void print_clear(color_t text_color, color_t bg_color) {
  background_color = bg_color;
  text_color = bg_color;

  // Dispatch call to video library to clear screen
  clear_screen(bg_color);

  // Reset cursor position
  cursor_x = 0;
  cursor_y = 0;
}

// Set color mode of screen
void setColorMode(color_t colorMode) { default_color_mode = colorMode; }

// Get current color
color_t getColorMode() { return default_color_mode; }

// Prints a character at current cursor position with given color mode
void putc(char c) {
  if (c == '\n') {
    cursor_x = 0;
    cursor_y += 1;
    // Wrap around logic
    cursor_y %= max_char_y;

  } else {
    color_t font_color = (c == ' ') ? background_color : default_color_mode;

    video_draw_char(c, cursor_x * font_size_x, cursor_y * font_size_y,
                    font_color);

    // Increment x and y
    cursor_x += 1;
    // Auto new line logic, when cursor_x goes beyond screen_width
    cursor_y += (cursor_x >= max_char_x);

    // Handle cursor wrap around logic
    cursor_y %= max_char_y;
    cursor_x %= max_char_x;
  }
}

void putcAt(char c, uint16_t x, uint16_t y, color_t colorMode) {
  if (c == '\n') {
    return;
  } else if (c == ' ') {
    video_clear_char(x * font_size_x, y * font_size_y, colorMode);
    return;
  }

  video_draw_char(c, x * font_size_x, y * font_size_y, colorMode);
}

// Prints a string until null terminator (unsafe)
uint32_t puts(const char *str) {
  uint32_t cnt = 0;
  while (*(str + cnt) && cnt != UINT32_MAX) {
    char c = *(str + cnt++);

    if (c == '\n') {
      cursor_y = (cursor_y + 1) % max_char_y;
      cursor_x = 0;
    } else {
      color_t font_color = (c == ' ') ? background_color : default_color_mode;
      video_draw_char(c, cursor_x * font_size_x, cursor_y * font_size_y,
                      font_color);

      // Increment x and y
      cursor_x += 1;
      // Auto new line logic, when cursor_x goes beyond screen_width
      cursor_y += (cursor_x >= max_char_x);

      // Handle cursor wrap around logic
      cursor_y %= max_char_y;
      cursor_x %= max_char_x;
    }
  }

  return cnt;
}

// Print formatted output without new line
int32_t print(const char *fmt, ...) {
  uint32_t cnt = 0;
  va_list apList;
  va_start(apList, fmt);

  for (uint32_t i = 0; fmt[i] != '\0'; ++i) {
    // --- ESCAPING & TAG START ---
    if (fmt[i] == '{') {
      if (fmt[i + 1] == '{') { // Escaped '{'
        putc('{');
        cnt++;
        i++;
        continue;
      }

      i++; // Skip '{', now at type char ('u', 'i', 'c', 's')
      char type = fmt[i];

      if (type == 'c') {
        char c = (char)va_arg(apList, int); // Promoted to int
        putc(c);
        cnt++;
      } else if (type == 's') {
        const char *s = va_arg(apList, const char *);
        uint32_t max_len = 0xFFFFFFFF; // Default to "until null"

        // Check for optional [<len>]
        if (fmt[i + 1] == '[') {
          i += 2; // skip 's['
          max_len = 0;
          while (fmt[i] >= '0' && fmt[i] <= '9') {
            max_len = max_len * 10 + (fmt[i] - '0');
            i++;
          }
          // Current fmt[i] is now ']'
        }

        for (uint32_t k = 0; s[k] != '\0' && k < max_len; ++k) {
          putc(s[k]);
          cnt++;
        }
      } else if (type == 'u' || type == 'i') {
        bool is_signed = (type == 'i');
        i++; // move to size digit

        uint32_t size = fmt[i] - '0';
        i++; // move to prefix or '}'

        uint32_t base = 10;
        if (fmt[i] == 'h') {
          base = 16;
          i++;
        } else if (fmt[i] == 'b') {
          base = 2;
          i++;
        }

        // Fetch argument.
        // C Promotion: types < 32bit arrive as 32bit.
        uint32_t val = va_arg(apList, uint32_t);

        cnt += print_int(val, base, is_signed, size);
      }

      // Skip until closing '}' to exit the tag context
      while (fmt[i] != '}' && fmt[i] != '\0')
        i++;
      continue;
    }

    // --- ESCAPING FOR '}' ---
    if (fmt[i] == '}') {
      if (fmt[i + 1] == '}') { // Escaped '}'
        putc('}');
        cnt++;
        i++;
        continue;
      }
      // If it's a single '}', we treat it as literal or ignore it
      // depending on if we are currently "inside" a tag.
    }

    // Regular character
    putc(fmt[i]);
    cnt++;
  }

  va_end(apList);
  return cnt;
}

int32_t println(const char *fmt, ...) {
  uint32_t cnt = 0;
  va_list apList;
  va_start(apList, fmt);

  for (uint32_t i = 0; fmt[i] != '\0'; ++i) {
    // --- ESCAPING & TAG START ---
    if (fmt[i] == '{') {
      if (fmt[i + 1] == '{') { // Escaped '{'
        putc('{');
        cnt++;
        i++;
        continue;
      }

      i++; // Skip '{', now at type char ('u', 'i', 'c', 's')
      char type = fmt[i];

      if (type == 'c') {
        char c = (char)va_arg(apList, int); // Promoted to int
        putc(c);
        cnt++;
      } else if (type == 's') {
        const char *s = va_arg(apList, const char *);
        uint32_t max_len = 0xFFFFFFFF; // Default to "until null"

        // Check for optional [<len>]
        if (fmt[i + 1] == '[') {
          i += 2; // skip 's['
          max_len = 0;
          while (fmt[i] >= '0' && fmt[i] <= '9') {
            max_len = max_len * 10 + (fmt[i] - '0');
            i++;
          }
          // Current fmt[i] is now ']'
        }

        for (uint32_t k = 0; s[k] != '\0' && k < max_len; ++k) {
          putc(s[k]);
          cnt++;
        }
      } else if (type == 'u' || type == 'i') {
        bool is_signed = (type == 'i');
        i++; // move to size digit

        uint32_t size = fmt[i] - '0';
        i++; // move to prefix or '}'

        uint32_t base = 10;
        if (fmt[i] == 'h') {
          base = 16;
          i++;
        } else if (fmt[i] == 'b') {
          base = 2;
          i++;
        }

        // Fetch argument.
        // C Promotion: types < 32bit arrive as 32bit.
        uint64_t val;
        if (size == 8)
          val = va_arg(apList, uint64_t);
        else
          val = (uint64_t)va_arg(apList, uint32_t);

        cnt += print_int(val, base, is_signed, size);
      }

      // Skip until closing '}' to exit the tag context
      while (fmt[i] != '}' && fmt[i] != '\0')
        i++;
      continue;
    }

    // --- ESCAPING FOR '}' ---
    if (fmt[i] == '}') {
      if (fmt[i + 1] == '}') { // Escaped '}'
        putc('}');
        cnt++;
        i++;
        continue;
      }
      // If it's a single '}', we treat it as literal or ignore it
      // depending on if we are currently "inside" a tag.
    }

    // Regular character
    putc(fmt[i]);
    cnt++;
  }

  va_end(apList);

  putc('\n');

  return cnt + 1;
}

// Print backspace at current cursor position
void putBackspace() {
  if (cursor_x == 0 && cursor_y == 0)
    return;

  if (cursor_x == 0) {
    cursor_y--;
    cursor_x = max_char_x - 1;
  } else {
    cursor_x--;
  }

  video_clear_char(cursor_x * font_size_x, cursor_y * font_size_y,
                   background_color);
}

// Get current cursor position (unsafe)
// TODO: Avoid taking pointers, instead return a struct
void getCursorPosition(uint16_t *outX, uint16_t *outY) {
  *outX = cursor_x;
  *outY = cursor_y;
}

// Get console size (unsafe)
// TODO: Avoid taking pointers, instead return a struct
void getConsoleSize(uint16_t *vgaX, uint16_t *vgaY) {
  *vgaX = max_char_x;
  *vgaY = max_char_y;
}

// Set cursor position
// Returns: 0 = success, -1 = out of bounds
int8_t setCursorPosition(uint16_t newX, uint16_t newY) {
  if (newX >= max_char_x || newY >= max_char_y)
    return -1;

  cursor_x = newX;
  cursor_y = newY;
  return 0;
}

/**
 * Helper to convert an integer to a string in a specific base.
 * Handles sign, base (2, 10, 16), and bit-masking for sizes.
 */
static int16_t print_int(uint32_t value, uint8_t base, bool signed_type,
                         uint8_t bytes) {
  char buf[33];
  char *ptr = &buf[32];
  *ptr = '\0';
  int16_t cnt = 0;

  // Calculate Mask
  uint32_t mask;
  if (bytes >= 4) {
    mask = 0xFFFFFFFF;
  } else {
    mask = (1U << (bytes * 8)) - 1;
  }
  uint32_t masked_val = value & mask;

  // Handle Sign (Only for Base 10)
  if (base == 10 && signed_type) {
    uint32_t msb = 1U << (bytes * 8 - 1);
    if (masked_val & msb) {
      putc('-');
      cnt++;
      masked_val = (~masked_val + 1) & mask;
    }
  }

  // Perform Radix Conversion
  // This fills the buffer from right to left
  do {
    uint32_t m = masked_val % base;
    *--ptr = (m < 10) ? (m + '0') : (m - 10 + 'A');
    masked_val /= base;
  } while (masked_val > 0);

  // Handle Prefixes for Hex and Binary
  if (base == 16) {
    puts("0x");
    cnt += 2;
  } else if (base == 2) {
    puts("0b");
    cnt += 2;
  }

  // Output the converted number string
  cnt += puts(ptr);
  return cnt;
}