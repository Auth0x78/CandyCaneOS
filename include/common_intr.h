#ifndef COMMON_INTR_H
#define COMMON_INTR_H

#define UD_INT_VECTOR 6  /* Invalid Opcode */
#define DF_INT_VECTOR 8  /* Double Fault */
#define GP_INT_VECTOR 13 /* General Protection */
#define KBD_INT_VECTOR 0x21

// External links to ISRs defined in isr.asm file
extern void isr_ud();
extern void isr_df();
extern void isr_gp();

// External ASM ISR handler for keyboard interrupts
extern void isr_keyboard();

#endif