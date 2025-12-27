#include <stdint.h>

#include "print.h"

// Helper functions
static inline void print_regs(const char *name, uint32_t val) {
  println("{s}: {u4h}", name, val);
}

// Used for when kernel panics, dumps all core info needed to screen
void k_panic(void) {
  uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr0, cr2, cr3;

  // Stage 1: Capture the main calculation registers
  __asm__ volatile("mov %%eax, %0" : "=m"(eax));
  __asm__ volatile("mov %%ebx, %0" : "=m"(ebx));
  __asm__ volatile("mov %%ecx, %0" : "=m"(ecx));
  __asm__ volatile("mov %%edx, %0" : "=m"(edx));

  // Stage 2: Capture index and stack pointers
  __asm__ volatile("mov %%esi, %0" : "=m"(esi));
  __asm__ volatile("mov %%edi, %0" : "=m"(edi));
  __asm__ volatile("mov %%esp, %0" : "=m"(esp));
  __asm__ volatile("mov %%ebp, %0" : "=m"(ebp));

  // Stage 3: System state registers
  __asm__ volatile("pushfl; popl %0" : "=m"(eflags));
  __asm__ volatile("mov %%cr0, %%eax; mov %%eax, %0" : "=m"(cr0) : : "eax");
  __asm__ volatile("mov %%cr2, %%eax; mov %%eax, %0" : "=m"(cr2) : : "eax");
  __asm__ volatile("mov %%cr3, %%eax; mov %%eax, %0" : "=m"(cr3) : : "eax");

  // Get EIP (current instruction pointer) using a local label
  __asm__ volatile("1: movl $1b, %0" : "=r"(eip));

  // Clear the screen to blue color and text color to white
  print_clear(COLOR_WHITE, COLOR(50, 50, 200));

  // Now print them to your Linear Framebuffer (LFB)
  println("--- Candycane CRASHED: REGISTER DUMP ---");

  // Print all the register content to screen
  print_regs("EAX", eax);
  print_regs("EBX", ebx);
  print_regs("ECX", ecx);
  print_regs("EDX", edx);
  print_regs("ESI", esi);
  print_regs("EDI", edi);
  print_regs("ESP", esp);
  print_regs("EBP", ebp);
  print_regs("EIP", eip);

  println("{s}: {u4b}", "EFLAGS", eflags);
  print_regs("CR0", cr0);
  print_regs("CR2", cr2);
  print_regs("C3", cr3);

  println("-----------------------------------");
}