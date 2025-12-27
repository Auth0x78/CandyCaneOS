#ifndef PRINT_H
#define PRINT_H

#include "video.h"
#include <stdarg.h>
#include <stdint.h>

// Initalizer printer
void print_init(uint16_t screen_width, uint16_t screen_height,
                uint8_t font_width, uint8_t font_height, color_t colorMode);

// Set color mode of screen
void setColorMode(color_t colorMode);

// Get current color
color_t getColorMode();

// Prints a character at current cursor position with set color mode
void putc(char c);

// Prints a character at given position with given color mode
void putcAt(char c, uint16_t x, uint16_t y, color_t colorMode);

// Prints a string until null terminator (unsafe)
uint32_t puts(const char *str);

/* Format of fmt string must be:
 *  {<type>}
 *  <type>: Allowed type are:
 *      Number:
 *              u<size>[h/b] => Unsigned <size> bytes
 *              i<size>[h/b] => Signed <size> bytes
 *      NOTE: Allowed sizes are 1, 2 and 4, Prefix can be h (hex) or b (bin)
 *      Character: c
 *      String: s[<len>], Length optional when null terminated
 *  To print '{' you can escape using '{', for '}' you can escape using '}'
 */

// Formated print function with no new line
int32_t print(const char *fmt, ...);

// Formated print function with new line ending
int32_t println(const char *fmt, ...);

// Print backspace at current cursor position
void putBackspace();

// Get current cursor position (unsafe)
// TODO: Avoid taking pointers, instead return a struct
void getCursorPosition(uint16_t *outX, uint16_t *outY);

// Get console size (unsafe)
// TODO: Avoid taking pointers, instead return a struct
void getConsoleSize(uint16_t *vgaX, uint16_t *vgaY);

// Set cursor position
// Returns: 0 = success, -1 = out of bounds
int8_t setCursorPosition(uint16_t newX, uint16_t newY);
#endif