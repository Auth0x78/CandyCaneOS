#include <stdint.h>

#include "PIC.h"
#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "multiboot.h"
#include "print.h"

// MACROS
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit) ((flags) & (1 << (bit)))

// Define the kernel's end
extern uint8_t __kernel_end[];

// Start of 4K aligned free memory
extern uint8_t __free_mem_aligned[];

// Initialize the console and print a welcome message
void console_init(uint32_t mboot_magic, uint32_t *mboot_info_ptr_addr);

// Kernel main function impl
extern void kernel_main(uint32_t mboot_magic, uint32_t *mboot_info_ptr_addr) {
  // Cast Physical address to multiboot info struct
  multiboot_info_t *mbi = (multiboot_info_t *)mboot_info_ptr_addr;

  // Verify correct information is passed by the boot.asm
  if (mboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    printLn("Invalid magic number found: {u4h}", mboot_magic);
    return;
  }

  // Initialize console and print welcome message
  console_init(mboot_magic, mboot_info_ptr_addr);

  // Prints the flags of mbi
  printLn("Multiboot info flags: {u4h}", mbi->flags);

  /* Are mem_* valid? */
  if (CHECK_FLAG(mbi->flags, 0)) {
    printLn("Mem_lower:Mem_upper is {u4h}:{u4h}", mbi->mem_lower,
            mbi->mem_upper);
  }

  // Initialize IDT
  idt_init();

  // Initialize PIC
  PIC_Init();

  // Initialize keyboard
  keyboard_init();

  // Enable interrupts
  asm volatile("sti");

  while (1) {
    asm volatile("hlt");
  }
}

// Function definations
void console_init(uint32_t mboot_magic, uint32_t *mboot_info_ptr_addr) {
  // Clear the screen to white on black
  print_init(0, 0, 0x0F);

  const char *OSName = "Candy Cane OS";

  // Print welcome message
  puts("Welcome to ");
  for (int i = 0;; i++) {
    if (OSName[i] == '\0') {
      putc('\n');
      break;
    }

    if (i % 2 == 0) {
      setColorMode(0x0C);
      putc(OSName[i]); // Red
    } else {
      setColorMode(0x0A);
      putc(OSName[i]); // Light Green
    }
  }
  setColorMode(0x0F);

  // Print the magic number and multiboot info address
  printLn(
      "Multiboot Magic Number: {u4h}    Multiboot Info Struct Address: {u4h}",
      mboot_magic, (uint32_t)mboot_info_ptr_addr);

  printLn("Kernel end address: {u4h}", (uint32_t)__kernel_end);

  printLn("Kernel memory used: {u4h}", ((uint32_t)__kernel_end) - 0x100000);

  printLn("Free memory start address: {u4h}", (uint32_t)__free_mem_aligned);
}