#include "idt.h"

/* Actual storage allocated ONCE */
struct IDTEntry idt[IDT_ENTRIES];
struct IDTPtr idt_ptr;

void load_idt(struct IDTPtr *idt_ptr) {
  asm volatile("lidt (%0)" : : "r"(idt_ptr) : "memory");
}

void idt_set_gate(int n, uint32_t handler) {
  idt[n].offset_low = handler & 0xFFFF;
  idt[n].selector = 0x08; // kernel CS
  idt[n].zero = 0;
  idt[n].type_attr = 0x8E; // interrupt gate
  idt[n].offset_high = handler >> 16;
}

void idt_init(void) {
  idt_ptr.limit = sizeof(idt) - 1;
  idt_ptr.base = (uint32_t)&idt;
  load_idt(&idt_ptr);
}
