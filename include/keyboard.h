#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64
#define KBD_INT_VECTOR 0x21

#define KBD_CMD_SET_LEDS 0xED

typedef enum {
  KBD_KEY_LSHIFT = 0x01,
  KBD_KEY_RSHIFT = 0x02,
#define KDB_KEY_ASHIFT 0x03
  KBD_KEY_CTRL = 0x04,
  KBD_KEY_ALT = 0x08,
  KBD_KEY_CAPSLOCK = 0x10,
  KBD_KEY_NUMLOCK = 0x20,
  KBD_KEY_SUPER = 0x40,
  KBD_EXTENDED_CODE = 0x80
} KBD_SPECIAL_KEYS;

// Current key pressed statea
extern volatile uint8_t key_state[256];
extern volatile uint8_t specialKeys;

// Initialize the keyboard
void keyboard_init();

// Enable keyboard interrupts and handle key events
void keyboard_enable();

// Disable keyboard interrupts
void keyboard_disable();

#endif