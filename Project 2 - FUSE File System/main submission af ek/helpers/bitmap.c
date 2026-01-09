/**
 * @file bitmap.c
 * @author CS3650 staff
 *
 * Bitmap implementation.
 */
#include <stdint.h>
#include <stdio.h>
#include "blocks.h"
#include "bitmap.h"

#define nth_bit_mask(n) (1 << (n))
#define byte_index(n) ((n) / 8)
#define bit_index(n) ((n) % 8)

// Get the given bit from the bitmap.
int bitmap_get(void *bm, int i) {
  uint8_t *base = (uint8_t *) bm;

  return (base[byte_index(i)] >> bit_index(i)) & 1;
}

// Set the given bit in the bitmap to the given value.
void bitmap_put(void *bm, int i, int v) {
  uint8_t *base = (uint8_t *) bm;

  long bit_mask = nth_bit_mask(bit_index(i));

  if (v) {
    base[byte_index(i)] |= bit_mask;
  } else {
    bit_mask = ~bit_mask;
    base[byte_index(i)] &= bit_mask;
  }
}

/*
 * Allocate the first free bit in the on-disk bitmap which we store
 * in block 0.
  */
int bitmap_alloc(void) {
  void *bm = blocks_get_block(0);     // block 0 used as global bitmap
  if (!bm) return -1;

  int nbits = BLOCK_SIZE * 8;
  for (int i = 0; i < nbits; i++) {
    if (bitmap_get(bm, i) == 0) {
      bitmap_put(bm, i, 1);
      return i;
    }
  }
  return -1;
}

/* Free the given bit in block 0 bitmap. */
void bitmap_free(int i) {
  void *bm = blocks_get_block(0);
  if (!bm) return;
  bitmap_put(bm, i, 0);
}

// Pretty-print the bitmap (with the given no. of bits).
void bitmap_print(void *bm, int size) {

  for (int i = 0; i < size; i++) {
    putchar(bitmap_get(bm, i) ? '1' : '0');

    if ((i + 1) % 64 == 0) {
      putchar('\n');
    } else if ((i + 1) % 8 == 0) {
      putchar(' ');
    }
  }
}
