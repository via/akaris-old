#ifndef I386_TYPES_H
#define I386_TYPES_H

#define NULL 0

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef signed long long int64;
typedef signed int int32;
typedef signed short int16;
typedef signed char int8;
 
typedef int64 int64_t;
typedef int32 int32_t;
typedef int16 int16_t;
typedef int8 int8_t;
typedef uint64 uint64_t;
typedef uint32 uint32_t;
typedef uint16 uint16_t;
typedef uint8 uint8_t;

/* Paging defs*/
typedef uint32 pte;
typedef uint32 pde;
#endif
