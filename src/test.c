#include "../include/ct1.h"
#include "lz4.h"

// Custom implementation of the memmove function
void* my_memmove(void* dest, const void* src, s32 n) {
  // Cast the destination and source pointers to char pointers
  char* d = (char*) dest;
  const char* s = (const char*) src;

  // If the source and destination pointers are the same, we don't need to do anything
  if (d == s) return dest;

  // If the source and destination pointers overlap, we need to copy the data in reverse order
  // to avoid overwriting data that hasn't been copied yet
  if (d > s && d < s + n) {
    for (size_t i = n; i > 0; i--) {
      d[i - 1] = s[i - 1];
    }
  }
  // Otherwise, we can just copy the data in the normal order
  else {
    for (size_t i = 0; i < n; i++) {
      d[i] = s[i];
    }
  }

  // Return the destination pointer
  return dest;
}

void* my_memset(void* s, int c, s32 n) {
  // Cast the input pointer to a char pointer
  char* p = (char*) s;

  // Set each byte to the given value
  for (size_t i = 0; i < n; i++) {
    p[i] = c;
  }

  // Return the input pointer
  return s;
}

void* my_memcpy(void* dst, const void* src, s32 n) {
  // Convert the void pointers to character pointers so that we can
  // perform arithmetic on them. This is allowed because characters
  // are the smallest addressable unit in C.
  char* dst_char = (char*) dst;
  const char* src_char = (const char*) src;

  // Copy the data one byte at a time until we have copied the
  // specified number of bytes.
  for (size_t i = 0; i < n; i++) {
    dst_char[i] = src_char[i];
  }

  // Return a pointer to the destination memory location.
    return dst;
}

// int compress_lz4(const u8* srcData, int srcSize, void* compressionBuffer) {
//     // Compress
//     //static uint8_t compressBuffer[1024 * 1024];
//     const int compressedSize = LZ4_compress_default((const char*) srcData, (char*) compressionBuffer, srcSize, sizeof(compressBuffer));
//     if (compressedSize <= 0) {
//         return 0;
//     }

//     return compressedSize;
// }

// void decompress_lz4 (const u8* srcData, int srcSize, int compressedSize, void* decompressionBuffer) {
//     const int decompressedSize = LZ4_decompress_safe(
//         (const char*) compressBuffer,
//         (char*) decompressionBuffer,
//         compressedSize,
//         sizeof(ramEndAddr - ramStartAddr)
//     );
//     if (decompressedSize <= 0) {
//         return;
//     }

//     // Verify
//     if (decompressedSize != srcSize) {
//         return;
//     }
//     // if (memcmp(srcData, decompressBuffer, srcSize) != 0) {
//     //     return;
//     // }
// }

void decompress_lz4_test2(int srcSize) {
    // Decompress
    //static uint8_t decompressBuffer[1024 * 1024];
    const int decompressedSize = LZ4_decompress_safe(
        (const char*) compressBuffer,
        (char*) decompressBuffer,
        savestateCompressedSize,
        sizeof(decompressBuffer)
    );
    if (decompressedSize <= 0) {
        return;
    }

    // Verify
    if (decompressedSize != srcSize) {
        return;
    }
    // if (memcmp(srcData, decompressBuffer, srcSize) != 0) {
    //     return;
    // }
}


void compress_lz4_test2(const u8* srcData, int srcSize) {
    // Compress
    //static uint8_t compressBuffer[1024 * 1024];
    savestateCompressedSize = LZ4_compress_default((const char*) srcData, (char*) compressBuffer, srcSize, sizeof(compressBuffer));
    if (savestateCompressedSize <= 0) {
        return;
    }
}


// void lz4TestingCompression(const u8* srcData, int srcSize) {
//     test_lz4(srcData, srcSize);
// }