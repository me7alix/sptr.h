/*
  sptr.h - A header-only library for safe dynamic memory management.

  This library provides smart pointers (sptr) that help detect memory 
  errors such as out-of-bounds access, double free and use after free.
  In debug mode (default), sptr verifies index validity and tracks 
  memory deallocation, reporting errors with file and line information.
  In RELEASE_MODE (enabled via #define), these checks are disabled 
  for better performance.

  Usage:
    sptr my_array = sptr_alloc(int, 10);  // Allocate memory
    sptr_at(int, my_array, 5) = 42;       // Access an element
    sptr_free(my_array);                  // Free memory

  For working with matrices, use the sptr_mat(type, sp, i, j) macro.

  Author: me7alix
*/

#ifndef SPTR_H_
#define SPTR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define sptr_mat(type, sp, i, j) sptr_at(type, sptr_at(sptr, sp, i), j)

#ifdef RELEASE_MODE

typedef struct { void* ptr; int size;} sptr;
#define sptr_alloc(type, size) ((sptr){malloc(sizeof(type)*size),size})
#define sptr_at(type, sp, ind) (*(((type*)sp.ptr)+ind))
#define sptr_free(sp) free(sp.ptr)

#else

typedef struct {
  void* ptr; 
  size_t size; 
  char *isf; 
} sptr;

#define sptr_alloc(type, size) ({ \
    void *p = malloc(sizeof(type) * size); \
    char *isf = malloc(sizeof(char)); \
    *isf = 0; \
    if (!p) { \
      fprintf(stderr, "%s:%d error: memory allocation failed\n", __FILE__, __LINE__); \
      exit(1); \
    } \
    (sptr){p, size, isf}; \
}) 

#define sptr_at(type, sp, ind) ( \
  *((ind>=sp.size||ind<0) ? \
     fprintf(stderr, "%s:%i error: index out of the range\n", __FILE__, __LINE__), \
     exit(1), \
     ((type*)sp.ptr) \
  : (*sp.isf || !sp.isf) ? \
    fprintf(stderr,"%s:%i error: use after free\n", __FILE__, __LINE__), \
    exit(1), \
    ((type*)sp.ptr) \
  : (((type*)sp.ptr)+ind)) \
) 

#define sptr_free(sp) \
  do { \
    if(*sp.isf || !sp.isf){ \
      fprintf(stderr, "%s:%i error: double free\n", __FILE__, __LINE__); \
      exit(1); \
    } \
    (*sp.isf) = 1; \
    free(sp.ptr); \
    free(sp.isf); \
  } while(0)

#endif

#endif
