#include "print.h"
#include <stdarg.h>

// VGA text mode memory start
static volatile uint16_t *const VGA_MEMORY = (uint16_t *)0xB8000;

// Current cursor offset in VGA memory
static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

// Current color of the text
static uint8_t default_color_mode = 0x0F;

// Helper function declaration
static int16_t print_int(uint32_t value, uint8_t base, bool signed_type,
                         uint8_t bytes);

// Initalizes the printer
void print_init(uint16_t cursorX, uint16_t cursorY, uint8_t colorMode) {
  cursor_x = cursorX;
  cursor_y = cursorY;
  default_color_mode = colorMode;

  // Also clear screen for printing
  cls(cursor_x, cursor_y);
}

// Clear screen from (startX, startY) to end with given color mode
void cls(uint16_t startX, uint16_t startY) {
  for (uint16_t row = startY; row < VGA_HEIGHT; ++row) {
    for (uint16_t col = startX; col < VGA_WIDTH; ++col) {
      VGA_MEMORY[VGA_OFFSET(col, row)] =
          (uint16_t)' ' | (default_color_mode << 8);
    }
  }
  cursor_x = startX;
  cursor_y = startY;
}

// Set color mode of screen
void setColorMode(uint8_t colorMode) { default_color_mode = colorMode; }

// Get current color
uint8_t getColorMode() { return default_color_mode; }

// Prints a character at current cursor position with given color mode
void putc(char c) {
  if (c == '\n') {
    cursor_y = (cursor_y + 1) % VGA_HEIGHT;
    cursor_x = 0;
  } else {
    VGA_MEMORY[VGA_OFFSET(cursor_x, cursor_y)] =
        (uint16_t)c | (default_color_mode << 8);

    cursor_y += (cursor_x + 1) == VGA_WIDTH ? 1 : 0;
    cursor_y %= VGA_HEIGHT;
    cursor_x = (cursor_x + 1) % VGA_WIDTH;
  }
}

// Prints a string until null terminator (unsafe)
uint32_t puts(const char *str) {
  uint32_t cnt = 0;
  while (*(str + cnt) && cnt != UINT32_MAX) {
    char c = *(str + cnt++);

    if (c == '\n') {
      cursor_y = (cursor_y + 1) % VGA_HEIGHT;
      cursor_x = 0;
    } else {
      VGA_MEMORY[VGA_OFFSET(cursor_x, cursor_y)] =
          (uint16_t)c | (default_color_mode << 8);

      cursor_y += (cursor_x + 1) == VGA_WIDTH ? 1 : 0;
      cursor_y %= VGA_HEIGHT;
      cursor_x = (cursor_x + 1) % VGA_WIDTH;
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

int32_t printLn(const char *fmt, ...) {
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
    cursor_x = VGA_WIDTH - 1;
  } else {
    cursor_x--;
  }

  VGA_MEMORY[VGA_OFFSET(cursor_x, cursor_y)] =
      (uint16_t)' ' | (0xf << 8); // Clear with default color
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
  *vgaX = VGA_WIDTH;
  *vgaY = VGA_HEIGHT;
}

// Set cursor position
// Returns: 0 = success, -1 = out of bounds
int8_t setCursorPosition(uint16_t newX, uint16_t newY) {
  if (newX >= VGA_WIDTH || newY >= VGA_HEIGHT)
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