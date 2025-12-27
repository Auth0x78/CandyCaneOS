#include "memory.h"
#include <stdint.h>

void *memset(void *dest, int c, size_t n) {
  if (n == 0)
    return dest;

  void *temp = dest;
  __asm__ volatile("rep stosb"
                   : "+D"(temp), "+c"(n)
                   : "a"((uint8_t)c)
                   : "memory");
  return dest;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
  if (n == 0 || dest == src)
    return dest;

  uint32_t d = (uintptr_t)dest;
  uint32_t s = (uintptr_t)src;
  uint32_t num_dwords = n >> 2;
  uint32_t num_bytes = n & 3;

  // Fast 4-byte copy
  if (num_dwords > 0) {
    __asm__ volatile("rep movsl"
                     : "+D"(d), "+S"(s), "+c"(num_dwords)
                     :
                     : "memory");
  }

  // Mop up remaining bytes
  if (num_bytes > 0) {
    __asm__ volatile("rep movsb"
                     : "+D"(d), "+S"(s), "+c"(num_bytes)
                     :
                     : "memory");
  }

  return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
  if (n == 0 || dest == src)
    return dest;

  if (dest < src) {
    // Forward copy is safe
    return memcpy(dest, src, n);
  } else {
    // Backward copy required for overlap safety
    uint8_t *d = (uint8_t *)dest + n - 1;
    const uint8_t *s = (const uint8_t *)src + n - 1;

    __asm__ volatile("std\n\t"
                     "rep movsb\n\t"
                     "cld"
                     : "+D"(d), "+S"(s), "+c"(n)
                     :
                     : "memory", "cc");
  }
  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }
  return 0;
}