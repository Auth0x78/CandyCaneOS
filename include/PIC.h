#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/* * Public Interface for PIC
 * IRQs 0-7 are remapped to IDT vectors 0x20-0x27 (Master)
 * IRQs 8-15 are remapped to IDT vectors 0x28-0x2F (Slave)
 */

// Initialize the PIC (remaps vectors and sets up cascading)
void PIC_Init(void);

// Send End-Of-Interrupt signal.
// 'irq' is the IRQ number (0-15), NOT the IDT vector.
void PIC_SendEOI(uint8_t irq);

// Mask (disable) a specific IRQ (0-15)
void PIC_SetMask(uint8_t irq);

// Unmask (enable) a specific IRQ (0-15)
void PIC_ClearMask(uint8_t irq);

// Read the Interrupt Request Register (IRR)
// Returns a 16-bit bitmap of interrupts that have been raised but not yet
// acknowledged. (Lower 8 bits = Master, Upper 8 bits = Slave)
uint16_t PIC_ReadIRR(void);

// Read the In-Service Register (ISR)
// Returns a 16-bit bitmap of interrupts currently being serviced
// (Lower 8 bits = Master, Upper 8 bits = Slave)
uint16_t PIC_ReadISR(void);

#endif