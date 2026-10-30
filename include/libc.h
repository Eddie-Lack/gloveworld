#ifndef LIBC_H
#define LIBC_H

void *memcpy(void *dst, const void *src, unsigned int n);
unsigned int strlen(const char *s);
float sqrtf(float);
float __sinf(float);
float __cosf(float);
// GCC treats sinf and cosf as builtins. It does not evaluate their calls
// ahead of the rest of an expression. libultra provides them as aliases.
#pragma weak sinf = __sinf
#pragma weak cosf = __cosf
float sinf(float);
float cosf(float);

#endif
