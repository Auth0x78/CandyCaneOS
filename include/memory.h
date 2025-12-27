#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

/* Handle 'restrict' for older C standards or C++ */
#ifndef restrict
#if __STDC_VERSION__ >= 199901L
/* Standard C99 and newer */
#else
/* Older C or C++ */
#define restrict __restrict
#endif
#endif

// Fills memory block with a byte value.
void *memset(void *dest, int c, size_t n);

// Copies memory (Non-overlapping only).
void *memcpy(void *restrict dest, const void *restrict src, size_t n);

// Copies memory (Safe for overlapping regions).
void *memmove(void *dest, const void *src, size_t n);

// Compares two memory blocks.
int memcmp(const void *s1, const void *s2, size_t n);

#endif