#ifndef IDT_H
#define IDT_H
#include <stdint.h>

#define IDT_ENTRIES 256

struct IDTEntry {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t type_attr;
  uint16_t offset_high;
} __attribute__((packed));

struct IDTPtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

/* Extern declarations */
extern struct IDTEntry idt[IDT_ENTRIES];
extern struct IDTPtr idt_ptr;

/* Public API */
void idt_init(void);
void idt_set_gate(int n, uint32_t handler);
#endif