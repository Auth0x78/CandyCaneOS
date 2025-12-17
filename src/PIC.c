#include "PIC.h"
#include "io.h"

// PIC Constants
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI 0x20

// Initialization Command Words
#define ICW1_INIT 0x11 // Init command + ICW4 needed
#define ICW4_8086 0x01 // 8086/88 (MCS-80/85) mode

// Vector Offsets (User requested)
#define PIC1_OFFSET 0x20 // Master maps to 0x20-0x27
#define PIC2_OFFSET 0x28 // Slave maps to 0x28-0x2F

// Operation Command Words for reading registers
#define PIC_READ_IRR 0x0A // OCW3 irq ready next CMD read
#define PIC_READ_ISR 0x0B // OCW3 isr ready next CMD read

// Internal state
static char pic1_mask = 0xFF; // IRQs 0-7
static char pic2_mask = 0xFF; // IRQs 8-15

// --- Implementation ---

void PIC_Init(void) {
  // Start initialization sequence (ICW1)
  outByte(PIC1_COMMAND, ICW1_INIT);
  io_wait();
  outByte(PIC2_COMMAND, ICW1_INIT);
  io_wait();

  // Set Vector Offsets (ICW2)
  outByte(PIC1_DATA, PIC1_OFFSET);
  io_wait();
  outByte(PIC2_DATA, PIC2_OFFSET);
  io_wait();

  // Configure Cascading (ICW3)
  // Master: tell it there is a slave at IRQ2 (0000 0100)
  outByte(PIC1_DATA, 4);
  io_wait();
  // Slave: tell it its cascade identity (0000 0010)
  outByte(PIC2_DATA, 2);
  io_wait();

  // Set Mode (ICW4) - 8086 mode
  outByte(PIC1_DATA, ICW4_8086);
  io_wait();
  outByte(PIC2_DATA, ICW4_8086);
  io_wait();

  // Set all IRQs disabled (mask = 0xFF)
  outByte(PIC1_DATA, pic1_mask);
  outByte(PIC2_DATA, pic2_mask);
}

void PIC_SendEOI(uint8_t irq) {
  // If IRQ comes from the Slave (8-15), we must send EOI to both.
  if (irq >= 8) {
    outByte(PIC2_COMMAND, PIC_EOI);
  }
  // Always send EOI to Master
  outByte(PIC1_COMMAND, PIC_EOI);
}

void PIC_SetMask(uint8_t irq) {
  // IRQ 0-7
  if (irq < 8) {
    pic1_mask |= (1 << irq);
    outByte(PIC1_DATA, pic1_mask);
  }
  // IRQ 8-15
  else {
    irq -= 8;
    pic2_mask |= (1 << irq);
    outByte(PIC2_DATA, pic2_mask);
  }
}

void PIC_ClearMask(uint8_t irq) {
  // IRQ 0-7
  if (irq < 8) {
    pic1_mask &= ~(1 << irq);
    outByte(PIC1_DATA, pic1_mask);
  }
  // IRQ 8-15
  else {
    irq -= 8;
    pic2_mask &= ~(1 << irq);
    outByte(PIC1_DATA, pic2_mask);
  }
}

// Helper to read the specific register via OCW3
static uint16_t __pic_get_irq_reg(int ocw3) {
  // Issue the command to both PICS
  outByte(PIC1_COMMAND, ocw3);
  outByte(PIC2_COMMAND, ocw3);

  // Read the result
  // Combine: Slave is high byte, Master is low byte
  return (inByte(PIC2_COMMAND) << 8) | inByte(PIC1_COMMAND);
}

uint16_t PIC_ReadIRR() { return __pic_get_irq_reg(PIC_READ_IRR); }

uint16_t PIC_ReadISR() { return __pic_get_irq_reg(PIC_READ_ISR); }