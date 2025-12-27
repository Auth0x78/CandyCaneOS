#include <stdint.h>

#include "PIC.h"
#include "common_intr.h"
#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "multiboot.h"
#include "print.h"
#include "video.h"

// MACROS
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit) ((flags) & (1 << (bit)))

// Define the kernel's end
extern uint8_t __kernel_end[];

// Start of 4K aligned free memory
extern uint8_t __free_mem_aligned[];

// Initialize the console and print a welcome message
void print_info(uint32_t mboot_magic, uint32_t *mboot_info_ptr_addr);

// Kernel main function impl
extern void kernel_main(uint32_t mboot_magic, uint32_t *mboot_info_ptr_addr) {
  // Cast Physical address to multiboot info struct
  multiboot_info_t *mbi = (multiboot_info_t *)mboot_info_ptr_addr;

  // Verify correct information is passed by the boot.asm
  if (mboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    return;
  }

  // Initialize IDT
  idt_init();

  // Setup IDT for common ISRs
  idt_set_gate(UD_INT_VECTOR, (uint32_t)isr_ud);
  idt_set_gate(GP_INT_VECTOR, (uint32_t)isr_gp);
  idt_set_gate(DF_INT_VECTOR, (uint32_t)isr_df);

  // Initialize video unit
  if (CHECK_FLAG(mbi->flags, 12)) {
    // Populate framebuffer information struct needed by video library
    framebuffer_info_t framebuffer_info = {
        .addr = mbi->framebuffer_addr,
        .pitch = mbi->framebuffer_pitch,
        .width = mbi->framebuffer_width,
        .height = mbi->framebuffer_height,
        .bitsPerPixel = mbi->framebuffer_bpp,

        // Position of each channel
        .red_pos = mbi->framebuffer_red_field_position,
        .green_pos = mbi->framebuffer_green_field_position,
        .blue_pos = mbi->framebuffer_blue_field_position,
        // Bits per color channel
        .red_mask_size = mbi->framebuffer_red_mask_size,
        .green_mask_size = mbi->framebuffer_green_mask_size,
        .blue_mask_size = mbi->framebuffer_blue_mask_size,

    };

    // Initialize the video library
    video_init(&framebuffer_info);

    // Initialize printer
    print_init(framebuffer_info.width, framebuffer_info.height, 8, 8,
               COLOR(0xFF, 0xFF, 0xFF));
  }

  // Initialize PIC
  PIC_Init();

  // Initialize keyboard
  keyboard_init();

  // Enable interrupts
  asm volatile("sti");

  // print welcome message
  print_info(mboot_magic, mboot_info_ptr_addr);

  // Prints the flags of mbi
  println("Multiboot info flags: {u4b}", mbi->flags);

  /* Are mem_* valid? */
  if (CHECK_FLAG(mbi->flags, 0)) {
    println("Mem_lower:Mem_upper is {u4h}:{u4h}", mbi->mem_lower,
            mbi->mem_upper);
  }

  uint16_t cursor_pos_x, cursor_pos_y;
  color_t blinkColor = COLOR_WHITE;
  while (1) {
    getCursorPosition(&cursor_pos_x, &cursor_pos_y);
    putcAt(' ', cursor_pos_x, cursor_pos_y, blinkColor);

    for (uint32_t iw = 0; iw < UINT32_MAX / 64; ++iw)
      io_wait();

    blinkColor.r = ~blinkColor.r;
    blinkColor.g = ~blinkColor.g;
    blinkColor.b = ~blinkColor.b;
  }
}

// Function definations
void print_info(uint32_t mboot_magic, uint32_t *mboot_info_ptr_addr) {

  const char *OSName = "Candy Cane OS";

  // Print welcome message
  puts("Welcome to ");
  for (int i = 0;; i++) {
    if (OSName[i] == '\0') {
      putc('\n');
      break;
    }

    if (i % 2 == 0) {
      setColorMode(COLOR(255, 180, 180));
      putc(OSName[i]); // Red
    } else {
      setColorMode(COLOR(180, 255, 180));
      putc(OSName[i]); // Light Green
    }
  }
  setColorMode(COLOR(255, 255, 255));

  // Print the magic number and multiboot info address
  println(
      "Multiboot Magic Number: {u4h}    Multiboot Info Struct Address: {u4h}",
      mboot_magic, (uint32_t)mboot_info_ptr_addr);

  println("Kernel end address: {u4h}", (uint32_t)__kernel_end);

  println("Kernel memory used: {u4h}", ((uint32_t)__kernel_end) - 0x100000);

  println("Free memory start address: {u4h}", (uint32_t)__free_mem_aligned);
}