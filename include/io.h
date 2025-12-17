#pragma once
#include <stdint.h>

static inline uint8_t inByte(uint16_t port) {
  uint8_t value;
  asm volatile("inb %1, %0"
               : "=a"(value) // output: AL
               : "Nd"(port)  // input: immediate 8-bit or DX
  );
  return value;
}

static inline void outByte(uint16_t port, uint8_t value) {
  asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void io_wait(void) { outByte(0x80, 0); }