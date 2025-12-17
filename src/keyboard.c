#include "keyboard.h"
#include "PIC.h"
#include "idt.h"
#include "io.h"

#define KBD_DBG_PRINT
#ifdef KBD_DBG_PRINT
#include "print.h"
#endif

// External ASM ISR handler for keyboard interrupts
extern void keyboard_isr();

// Current key pressed state
volatile uint8_t key_state[256] = {0};
volatile uint8_t specialKeys = 0;

// Internal state of keyboard
static uint8_t kbd_init = 0;
static uint8_t kbd_handler_enabled = 0;
static uint8_t kbd_caps_released = 1;
static uint8_t kbd_numlock_released = 1;

// NOTE: Scan codes not mappable to ASCII are mapped to 0
static const uint8_t scan_code_to_key[256] = {
    // Row 1
    0,
    27,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8', // 0x00 - 0x09
    '9',
    '0',
    '-',
    '=',
    '\b', // Backspace
    '\t', // Tab

    // Row 2
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n', // Enter key
    0,    // Left Control key
    // Row 3
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0, // Left Shift
    '\\',
    // Row 4
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0, // Right Shift
    '*',
    0,   // Left Alt
    ' ', // Space bar
    // Row 5
    0,   // Caps Lock
    0,   // F1
    0,   // F2
    0,   // F3
    0,   // F4
    0,   // F5
    0,   // F6
    0,   // F7
    0,   // F8
    0,   // F9
    0,   // F10
    0,   // Num Lock
    0,   // Scroll Lock
    0,   // Keypad 7
    0,   // Keypad 8
    0,   // Keypad 9
    '-', // Keypad -
    0,   // Keypad 4
    0,   // Keypad 5
    0,   // Keypad 6
    '+', // Keypad +
    0,   // Keypad 1
    0,   // Keypad 2
    0,   // Keypad 3
    0,   // Keypad 0
    '.', // Keypad . (0x53)
    0,   // 0x54
    0,   // 0x55
    0,   // 0x56
    0,   // F11
    0,   // F12
};

// Private helper functions
static void handle_asciiKey(uint8_t scancode);

// Public API
void keyboard_init() {
  // Setup keyboard interrupts
  idt_set_gate(KBD_INT_VECTOR, (uint32_t)keyboard_isr);

  // Enable keyboard IRQ in PIC
  PIC_ClearMask(1);

  // Setup kdb internal states
  kbd_caps_released = 1;
  kbd_numlock_released = 1;

  // Set initialized flag
  kbd_init = 1;
  kbd_handler_enabled = 1;
}

// Enable keyboard interrupts and handle key events
void keyboard_enable() { kbd_handler_enabled = 1; }

// Disable keyboard interrupts handling, read, send EOI but ignore key events
void keyboard_disable() {}

// C ISR handler for keyboard interrupts
void keyboard_handler() {
  if (kbd_init == 0)
    return; // Keyboard not initialized

  if (((inByte(KBD_STATUS_PORT) & 0x01) == 0) || (kbd_handler_enabled == 0))
    goto send_eoi; // No data to read

  // Read the scan code from keyboard data port
  uint8_t scan_code = inByte(KBD_DATA_PORT);

  // TODO: Handle all extended scan codes (0xE0 prefix)
  switch (scan_code) {
  case 0x2A:
    specialKeys |= KBD_KEY_LSHIFT;
    break;
  case 0x36:
    specialKeys |= KBD_KEY_RSHIFT;
    break;
  case 0x1D:
    specialKeys |= KBD_KEY_CTRL;
    break;
  case 0x38:
    specialKeys |= KBD_KEY_ALT;
    break;
  case 0x3A:
    if (kbd_caps_released == 1) {
      specialKeys ^= KBD_KEY_CAPSLOCK;
      kbd_caps_released = 0;
    }
    break;
  case 0x45:
    if (kbd_numlock_released == 1) {
      specialKeys ^= KBD_KEY_NUMLOCK;
      kbd_numlock_released = 0;
    }
    break;
  case 0x5B:
    if (specialKeys & KBD_EXTENDED_CODE) {
      specialKeys |= KBD_KEY_SUPER;
      // Consume the extended code flag
      specialKeys &= ~KBD_EXTENDED_CODE;
    }
    break;
  case 0xE0:
    specialKeys |= KBD_EXTENDED_CODE;
    break;

  // Release events
  case 0xAA:
    specialKeys &= ~KBD_KEY_LSHIFT;
    break;
  case 0xB6:
    specialKeys &= ~KBD_KEY_RSHIFT;
    break;
  case 0x9D:
    specialKeys &= ~KBD_KEY_CTRL;
    break;
  case 0xB8:
    specialKeys &= ~KBD_KEY_ALT;
    break;
  case 0xBA:
    kbd_caps_released = 1;
    break;
  case 0xC5:
    kbd_numlock_released = 1;
    break;
  case 0xDB:
    // Consume the extended code flag and also clear SUPER key
    if (specialKeys & KBD_EXTENDED_CODE)
      specialKeys &= ~(KBD_EXTENDED_CODE | KBD_KEY_SUPER);
    break;
  default:
    handle_asciiKey(scan_code);
    break;
  }

send_eoi:
#ifdef KBD_DBG_PRINT
  uint16_t oldX = 0, oldY = 0;

  setColorMode(0x0E);
  getCursorPosition(&oldX, &oldY);
  print("{u1h} ", scan_code);

  if (scan_code == 0x0E) {
    // Remove the backspace character and the space before it
    // Then after that will remove the previously printed character
    putBackspace();
    putBackspace();
    putBackspace();
    putBackspace();
  }

  print("{u1b} ", specialKeys);

  setColorMode(0x0F);
  // setCursorPosition(oldX, oldY);
#endif
  PIC_SendEOI(1);
}

// Private helper functions
static void handle_asciiKey(uint8_t scancode) {

  // Key release event
  if (scancode > 0x80) {
    uint8_t key = scan_code_to_key[scancode - 0x80];
    key_state[key] = 0x0;
    return;
  }

  // Key is pressed
  uint8_t key = scan_code_to_key[scancode];
  key_state[key] = 0x1;
}