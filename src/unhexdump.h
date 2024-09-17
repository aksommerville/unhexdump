#ifndef UNHEXDUMP_H
#define UNHEXDUMP_H

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define UHD_INPUT_SANITY_LIMIT (128<<20) /* 128 MB should be plenty, right? */

extern struct uhd {
  const char *exename;
  const char *srcpath; // null for stdin
  const char *dstpath; // null for stdout
  uint8_t *src;
  int srcc;
  uint8_t *dst;
  int dstc,dsta;
} uhd;

#endif
