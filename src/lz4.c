#include "../include/ct1.h"

/* mculib libgcc routines of Andes NDS32 cpu for GNU compiler
   Copyright (C) 2012-2022 Free Software Foundation, Inc.
   Contributed by Andes Technology Corporation.
   This file is part of GCC.
   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.
   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.
   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.
   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

int __clzsi2 (int val)
{
  int i = 32;
  int j = 16;
  int temp;

  for (; j; j >>= 1)
    {
      if (temp = val >> j)
	{
	  if (j == 1)
	    {
	      return (i - 2);
	    }
	  else
	    {
	      i -= j;
	      val = temp;
	    }
	}
    }
  return (i - val);
}

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 800)) || defined(__clang__)
#  define expect(expr,value)    (__builtin_expect ((expr),(value)) )
#else
#  define expect(expr,value)    (expr)
#endif

#ifndef likely
#define likely(expr)     expect((expr) != 0, 1)
#endif
#ifndef unlikely
#define unlikely(expr)   expect((expr) != 0, 0)
#endif

#ifndef likely
#define likely(expr)     expect((expr) != 0, 1)
#endif
#ifndef unlikely
#define unlikely(expr)   expect((expr) != 0, 0)
#endif

#define EXTERN_C

typedef s32 reg_t;   /* 32-bits in x32 mode */

typedef enum { clearedTable = 0, byPtr, byu32, byu16 } tableType_t;

#define LZ4_MAX_INPUT_SIZE        0x7E000000   /* 2 113 929 216 bytes */
#define LZ4_COMPRESSBOUND(isize)  ((unsigned)(isize) > (unsigned)LZ4_MAX_INPUT_SIZE ? 0 : (isize) + ((isize)/255) + 16)

#define LZ4_DISTANCE_ABSOLUTE_MAX 65535
#if (LZ4_DISTANCE_MAX > LZ4_DISTANCE_ABSOLUTE_MAX)   /* max supported by LZ4 format */
#  error "LZ4_DISTANCE_MAX is too big : must be <= 65535"
#endif

#define ML_BITS  4
#define ML_MASK  ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)

#define LZ4_memset(p,v,s) memset((p),(v),(s))
#define MEM_INIT(p,v,s)   LZ4_memset((p),(v),(s))

EXTERN_C void *memset(void *s, int c, s32 n) {
    u8* p = (u8*) s;
    while (n--) {
        *p++ = (u8) c;
    }
    return s;
}

#ifndef LZ4LIB_VISIBILITY
#  if defined(__GNUC__) && (__GNUC__ >= 4)
#    define LZ4LIB_VISIBILITY __attribute__ ((visibility ("default")))
#  else
#    define LZ4LIB_VISIBILITY
#  endif
#endif
#if defined(LZ4_DLL_EXPORT) && (LZ4_DLL_EXPORT==1)
#  define LZ4LIB_API __declspec(dllexport) LZ4LIB_VISIBILITY
#elif defined(LZ4_DLL_IMPORT) && (LZ4_DLL_IMPORT==1)
#  define LZ4LIB_API __declspec(dllimport) LZ4LIB_VISIBILITY /* It isn't required but allows to generate better code, saving a function pointer load from the IAT and an indirect jump.*/
#else
#  define LZ4LIB_API LZ4LIB_VISIBILITY
#endif

LZ4LIB_API int LZ4_sizeofState(void);
LZ4LIB_API int LZ4_compress_fast_extState (void* state, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);

#ifndef LZ4_DISTANCE_MAX   /* history window size; can be user-defined at compile time */
#  define LZ4_DISTANCE_MAX 65535   /* set to maximum value by default */
#endif

typedef unsigned int* uptrval;

/*------   Version   ------*/
#define LZ4_VERSION_MAJOR    1    /* for breaking interface changes  */
#define LZ4_VERSION_MINOR    9    /* for new (non-breaking) interface capabilities */
#define LZ4_VERSION_RELEASE  4    /* for tweaks, bug-fixes, or development */

#define LZ4_VERSION_NUMBER (LZ4_VERSION_MAJOR *100*100 + LZ4_VERSION_MINOR *100 + LZ4_VERSION_RELEASE)

#define LZ4_LIB_VERSION LZ4_VERSION_MAJOR.LZ4_VERSION_MINOR.LZ4_VERSION_RELEASE
#define LZ4_QUOTE(str) #str
#define LZ4_EXPAND_AND_QUOTE(str) LZ4_QUOTE(str)
#define LZ4_VERSION_STRING LZ4_EXPAND_AND_QUOTE(LZ4_LIB_VERSION)  /* requires v1.7.3+ */

typedef enum {
    notLimited = 0,
    limitedOutput = 1,
    fillOutput = 2
} limitedOutput_directive;

/*-************************************
*  Common Constants
**************************************/
#define MINMATCH 4

#define WILDCOPYLENGTH 8
#define LASTLITERALS   5   /* see ../doc/lz4_Block_format.md#parsing-restrictions */
#define MFLIMIT       12   /* see ../doc/lz4_Block_format.md#parsing-restrictions */
#define MATCH_SAFEGUARD_DISTANCE  ((2*WILDCOPYLENGTH) - MINMATCH)   /* ensure it's possible to write 2 x wildcopyLength without overflowing output buffer */
#define FASTLOOP_SAFE_DISTANCE 64
static const int LZ4_minLength = (MFLIMIT+1);

/*-************************************
*  Local Constants
**************************************/
static const int LZ4_64Klimit = ((64 KB) + (MFLIMIT-1));
static const u32 LZ4_skipTrigger = 6;  /* Increase this value ==> compression run slower on incompressible data */


/*-************************************
*  Local Structures and types
**************************************/

/**
 * This enum distinguishes several different modes of accessing previous
 * content in the stream.
 *
 * - noDict        : There is no preceding content.
 * - withPrefix64k : Table entries up to ctx->dictSize before the current blob
 *                   blob being compressed are valid and refer to the preceding
 *                   content (of length ctx->dictSize), which is available
 *                   contiguously preceding in memory the content currently
 *                   being compressed.
 * - usingExtDict  : Like withPrefix64k, but the preceding content is somewhere
 *                   else in memory, starting at ctx->dictionary with length
 *                   ctx->dictSize.
 * - usingDictCtx  : Everything concerning the preceding content is
 *                   in a separate context, pointed to by ctx->dictCtx.
 *                   ctx->dictionary, ctx->dictSize, and table entries
 *                   in the current context that refer to positions
 *                   preceding the beginning of the current compression are
 *                   ignored. Instead, ctx->dictCtx->dictionary and ctx->dictCtx
 *                   ->dictSize describe the location and size of the preceding
 *                   content, and matches are found by looking in the ctx
 *                   ->dictCtx->hashTable.
 */
typedef enum { noDict = 0, withPrefix64k, usingExtDict, usingDictCtx } dict_directive;
typedef enum { noDictIssue = 0, dictSmall } dictIssue_directive;

/*-************************************
*  Tuning parameters
**************************************/
/*
 * LZ4_HEAPMODE :
 * Select how stateless compression functions like `LZ4_compress_default()`
 * allocate memory for their hash table,
 * in memory stack (0:default, fastest), or in memory heap (1:requires malloc()).
 */
#ifndef LZ4_HEAPMODE
#  define LZ4_HEAPMODE 0
#endif

/*
 * LZ4_ACCELERATION_DEFAULT :
 * Select "acceleration" for LZ4_compress_fast() when parameter value <= 0
 */
#define LZ4_ACCELERATION_DEFAULT 1
/*
 * LZ4_ACCELERATION_MAX :
 * Any "acceleration" value higher than this threshold
 * get treated as LZ4_ACCELERATION_MAX instead (fix #876)
 */
#define LZ4_ACCELERATION_MAX 65537

#define LZ4_MEMORY_USAGE_MIN 10
#define LZ4_MEMORY_USAGE_DEFAULT 14
#define LZ4_MEMORY_USAGE_MAX 20

/*!
 * LZ4_MEMORY_USAGE :
 * Memory usage formula : N->2^N Bytes (examples : 10 -> 1KB; 12 -> 4KB ; 16 -> 64KB; 20 -> 1MB; )
 * Increasing memory usage improves compression ratio, at the cost of speed.
 * Reduced memory usage may improve speed at the cost of ratio, thanks to better cache locality.
 * Default value is 14, for 16KB, which nicely fits into Intel x86 L1 cache
 */
#ifndef LZ4_MEMORY_USAGE
# define LZ4_MEMORY_USAGE LZ4_MEMORY_USAGE_DEFAULT
#endif

#if (LZ4_MEMORY_USAGE < LZ4_MEMORY_USAGE_MIN)
#  error "LZ4_MEMORY_USAGE is too small !"
#endif

#if (LZ4_MEMORY_USAGE > LZ4_MEMORY_USAGE_MAX)
#  error "LZ4_MEMORY_USAGE is too large !"
#endif

#define LZ4_HASHLOG   (LZ4_MEMORY_USAGE-2)
#define LZ4_HASHTABLESIZE (1 << LZ4_MEMORY_USAGE)
#define LZ4_HASH_SIZE_u32 (1 << LZ4_HASHLOG)       /* required as macro for static allocation */

typedef struct LZ4_stream_t_internal LZ4_stream_t_internal;
struct LZ4_stream_t_internal {
    u32 hashTable[LZ4_HASH_SIZE_u32];
    const u8* dictionary;
    const LZ4_stream_t_internal* dictCtx;
    u32 currentOffset;
    u32 tableType;
    u32 dictSize;
    /* Implicit padding to ensure structure is aligned */
};

#define LZ4_STREAM_MINSIZE  ((1UL << LZ4_MEMORY_USAGE) + 32)  /* static size, for inter-version compatibility */
union LZ4_stream_u {
    char minStateSize[LZ4_STREAM_MINSIZE];
    LZ4_stream_t_internal internal_donotuse;
} LZ4_stream_u; /* previously typedef'd to LZ4_stream_t */

typedef union LZ4_stream_t {
    char minStateSize[LZ4_STREAM_MINSIZE];
    LZ4_stream_t_internal internal_donotuse;
} LZ4_stream_t; /* previously typedef'd to LZ4_stream_t */

/*-************************************
*  Local Utils
**************************************/
int LZ4_versionNumber (void) { return LZ4_VERSION_NUMBER; }
const char* LZ4_versionString(void) { return LZ4_VERSION_STRING; }
int LZ4_compressBound(int isize)  { return LZ4_COMPRESSBOUND(isize); }
int LZ4_sizeofState(void) { return sizeof(LZ4_stream_t); }

static int LZ4_isAligned(const void* ptr, s32 alignment)
{
    return ((s32)ptr & (alignment -1)) == 0;
}

static s32 LZ4_stream_t_alignment(void)
{
#if LZ4_ALIGN_TEST
    typedef struct { char c; LZ4_stream_t t; } t_a;
    return sizeof(t_a) - sizeof(LZ4_stream_t);
#else
    return 1;  /* effectively disabled */
#endif
}

//LZ4_stream_t* LZ4_initStream (void* buffer, s32 size)
LZ4_stream_t* LZ4_initStream (void* buffer, s32 size)
{
    ////DEBUGLOG(5, "LZ4_initStream");
    if (buffer == NULL) { return NULL; }
    if (size < sizeof(LZ4_stream_t)) { return NULL; }
    if (!LZ4_isAligned(buffer, LZ4_stream_t_alignment())) return NULL;
    MEM_INIT(buffer, 0, sizeof(LZ4_stream_t_internal));
    return (LZ4_stream_t*)buffer;
}

/* __pack instructions are safer, but compiler specific, hence potentially problematic for some compilers */
/* currently only defined for gcc and icc */
typedef struct { u16 u16; } __attribute__((packed)) LZ4_unalign16;
typedef struct { u32 u32; } __attribute__((packed)) LZ4_unalign32;
typedef struct { reg_t uArch; } __attribute__((packed)) LZ4_unalignST;

static u16 LZ4_read16(const void* ptr) { return ((const LZ4_unalign16*)ptr)->u16; }
static u32 LZ4_read32(const void* ptr) { return ((const LZ4_unalign32*)ptr)->u32; }
static reg_t LZ4_read_ARCH(const void* ptr) { return ((const LZ4_unalignST*)ptr)->uArch; }

static void LZ4_write16(void* memPtr, u16 value) { ((LZ4_unalign16*)memPtr)->u16 = value; }
static void LZ4_write32(void* memPtr, u32 value) { ((LZ4_unalign32*)memPtr)->u32 = value; }

/*-******************************
*  Compression functions
********************************/
__attribute__((optimize("O2"))) u32 LZ4_hash4(u32 sequence, tableType_t const tableType)
{
    if (tableType == byu16)
        return ((sequence * 2654435761U) >> ((MINMATCH*8)-(LZ4_HASHLOG+1)));
    else
        return ((sequence * 2654435761U) >> ((MINMATCH*8)-LZ4_HASHLOG));
}

__attribute__((optimize("O2"))) u32 LZ4_hash5(u64 sequence, tableType_t const tableType)
{
    const u32 hashLog = (tableType == byu16) ? LZ4_HASHLOG+1 : LZ4_HASHLOG;
    const u64 prime8bytes = 11400714785074694791ULL;
    return (u32)(((sequence >> 24) * prime8bytes) >> (64 - hashLog));
    // if (LZ4_isLittleEndian()) {
    //     const u64 prime5bytes = 889523592379ULL;
    //     return (u32)(((sequence << 24) * prime5bytes) >> (64 - hashLog));
    // } else {
    //     const u64 prime8bytes = 11400714785074694791ULL;
    //     return (u32)(((sequence >> 24) * prime8bytes) >> (64 - hashLog));
    // }
}

__attribute__((optimize("O2"))) u32 LZ4_hashPosition(const void* const p, tableType_t const tableType)
{
    if ((sizeof(s32)==8) && (tableType != byu16)) return LZ4_hash5(LZ4_read_ARCH(p), tableType);
    return LZ4_hash4(LZ4_read32(p), tableType);
}

__attribute__((optimize("O2"))) void LZ4_clearHash(u32 h, void* tableBase, tableType_t const tableType)
{
    switch (tableType)
    {
    default: /* fallthrough */
    case clearedTable: { /* illegal! */ return; }
    case byPtr: { const u8** hashTable = (const u8**)tableBase; hashTable[h] = NULL; return; }
    case byu32: { u32* hashTable = (u32*) tableBase; hashTable[h] = 0; return; }
    case byu16: { u16* hashTable = (u16*) tableBase; hashTable[h] = 0; return; }
    }
}

__attribute__((optimize("O2"))) void LZ4_putIndexOnHash(u32 idx, u32 h, void* tableBase, tableType_t const tableType)
{
    switch (tableType)
    {
    default: /* fallthrough */
    case clearedTable: /* fallthrough */
    case byPtr: { /* illegal! */ return; }
    case byu32: { u32* hashTable = (u32*) tableBase; hashTable[h] = idx; return; }
    case byu16: { u16* hashTable = (u16*) tableBase; hashTable[h] = (u16)idx; return; }
    }
}

/* LZ4_putPosition*() : only used in byPtr mode */
__attribute__((optimize("O2"))) void LZ4_putPositionOnHash(const u8* p, u32 h,
                                  void* tableBase, tableType_t const tableType)
{
    const u8** const hashTable = (const u8**)tableBase;
    //assert(tableType == byPtr); (void)tableType;
    hashTable[h] = p;
}

__attribute__((optimize("O2"))) void LZ4_putPosition(const u8* p, void* tableBase, tableType_t tableType)
{
    u32 const h = LZ4_hashPosition(p, tableType);
    LZ4_putPositionOnHash(p, h, tableBase, tableType);
}

__attribute__((optimize("O2"))) u32 LZ4_getIndexOnHash(u32 h, const void* tableBase, tableType_t tableType)
{
    //LZ4_STATIC_ASSERT(LZ4_MEMORY_USAGE > 2);
    if (tableType == byu32) {
        const u32* const hashTable = (const u32*) tableBase;
        //assert(h < (1U << (LZ4_MEMORY_USAGE-2)));
        return hashTable[h];
    }
    if (tableType == byu16) {
        const u16* const hashTable = (const u16*) tableBase;
        //assert(h < (1U << (LZ4_MEMORY_USAGE-1)));
        return hashTable[h];
    }
    //assert(0); return 0;  /* forbidden case */
}

/* customized variant of memcpy, which can overwrite up to 8 bytes beyond dstEnd */
__attribute__((optimize("O2"))) void LZ4_wildCopy8(void* dstPtr, const void* srcPtr, void* dstEnd)
{
    u8* d = (u8*)dstPtr;
    const u8* s = (const u8*)srcPtr;
    u8* const e = (u8*)dstEnd;

    do { ct_memcpy(d,s,8); d+=8; s+=8; } while (d<e);
}

static void LZ4_writeLE16(void* memPtr, u16 value)
{
    u8* p = (u8*)memPtr;
    p[0] = (u8) value;
    p[1] = (u8)(value>>8);
}

/*-************************************
*  Common functions
**************************************/
static unsigned LZ4_NbCommonBytes (s32 val)
{
    //assert(val != 0);
    /* Big Endian CPU */ {
        if (sizeof(val)==8) {
#       if (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                        !defined(__TINYC__) && !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_clzll((u64)val) >> 3;
#       else
#if 1
            /* this method is probably faster,
             * but adds a 128 bytes lookup table */
            static const unsigned char ctz7_tab[128] = {
                7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            };
            u64 const mask = 0x0101010101010101ULL;
            u64 const t = (((val >> 8) - mask) | val) & mask;
            return ctz7_tab[(t * 0x0080402010080402ULL) >> 57];
#else
            /* this method doesn't consume memory space like the previous one,
             * but it contains several branches,
             * that may end up slowing execution */
            static const U32 by32 = sizeof(val)*4;  /* 32 on 64 bits (goal), 16 on 32 bits.
            Just to avoid some static analyzer complaining about shift by 32 on 32-bits target.
            Note that this code path is never triggered in 32-bits mode. */
            unsigned r;
            if (!(val>>by32)) { r=4; } else { r=0; val>>=by32; }
            if (!(val>>16)) { r+=2; val>>=8; } else { val>>=24; }
            r += (!val);
            return r;
#endif
#       endif
        } else /* 32 bits */ {
#       if (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                                        !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_clz((u32)val) >> 3;
#       else
            val >>= 8;
            val = ((((val + 0x00FFFF00) | 0x00FFFFFF) + val) |
              (val + 0x00FF0000)) >> 24;
            return (unsigned)val ^ 3;
#       endif
        }
    }
}

#define STEPSIZE sizeof(s32)
__attribute__((optimize("O2"))) 
unsigned LZ4_count(const u8* pIn, const u8* pMatch, const u8* pInLimit)
{
    const u8* const pStart = pIn;

    if (likely(pIn < pInLimit-(STEPSIZE-1))) {
        reg_t const diff = LZ4_read_ARCH(pMatch) ^ LZ4_read_ARCH(pIn);
        if (!diff) {
            pIn+=STEPSIZE; pMatch+=STEPSIZE;
        } else {
            return LZ4_NbCommonBytes(diff);
    }   }

    while (likely(pIn < pInLimit-(STEPSIZE-1))) {
        reg_t const diff = LZ4_read_ARCH(pMatch) ^ LZ4_read_ARCH(pIn);
        if (!diff) { pIn+=STEPSIZE; pMatch+=STEPSIZE; continue; }
        pIn += LZ4_NbCommonBytes(diff);
        return (unsigned)(pIn - pStart);
    }

    if ((STEPSIZE==8) && (pIn<(pInLimit-3)) && (LZ4_read32(pMatch) == LZ4_read32(pIn))) { pIn+=4; pMatch+=4; }
    if ((pIn<(pInLimit-1)) && (LZ4_read16(pMatch) == LZ4_read16(pIn))) { pIn+=2; pMatch+=2; }
    if ((pIn<pInLimit) && (*pMatch == *pIn)) pIn++;
    return (unsigned)(pIn - pStart);
}

// static void LZ4_writeLE16(void* memPtr, u16 value)
// {
//     if (LZ4_isLittleEndian()) {
//         LZ4_write16(memPtr, value);
//     } else {
//         u8* p = (u8*)memPtr;
//         p[0] = (u8) value;
//         p[1] = (u8)(value>>8);
//     }
// }

static const u8* LZ4_getPositionOnHash(u32 h, const void* tableBase, tableType_t tableType)
{
    //assert(tableType == byPtr); (void)tableType;
    { const u8* const* hashTable = (const u8* const*) tableBase; return hashTable[h]; }
}

__attribute__((optimize("O2"))) const u8*
LZ4_getPosition(const u8* p,
                const void* tableBase, tableType_t tableType)
{
    u32 const h = LZ4_hashPosition(p, tableType);
    return LZ4_getPositionOnHash(h, tableBase, tableType);
}

__attribute__((optimize("O2"))) int LZ4_compress_generic_validated(
                 LZ4_stream_t_internal* const cctx,
                 const char* const source,
                 char* const dest,
                 const int inputSize,
                 int*  inputConsumed, /* only written when outputDirective == fillOutput */
                 const int maxOutputSize,
                 const limitedOutput_directive outputDirective,
                 const tableType_t tableType,
                 const dict_directive dictDirective,
                 const dictIssue_directive dictIssue,
                 const int acceleration)
{
    int result;
    const u8* ip = (const u8*)source;

    u32 const startIndex = cctx->currentOffset;
    const u8* base = (const u8*)source - startIndex;
    const u8* lowLimit;

    const LZ4_stream_t_internal* dictCtx = (const LZ4_stream_t_internal*) cctx->dictCtx;
    const u8* const dictionary =
        dictDirective == usingDictCtx ? dictCtx->dictionary : cctx->dictionary;
    const u32 dictSize =
        dictDirective == usingDictCtx ? dictCtx->dictSize : cctx->dictSize;
    const u32 dictDelta =
        (dictDirective == usingDictCtx) ? startIndex - dictCtx->currentOffset : 0;   /* make indexes in dictCtx comparable with indexes in current context */

    int const maybe_extMem = (dictDirective == usingExtDict) || (dictDirective == usingDictCtx);
    u32 const prefixIdxLimit = startIndex - dictSize;   /* used when dictDirective == dictSmall */
    const u8* const dictEnd = dictionary ? dictionary + dictSize : dictionary;
    const u8* anchor = (const u8*) source;
    const u8* const iend = ip + inputSize;
    const u8* const mflimitPlusOne = iend - MFLIMIT + 1;
    const u8* const matchlimit = iend - LASTLITERALS;

    /* the dictCtx currentOffset is indexed on the start of the dictionary,
     * while a dictionary in the current context precedes the currentOffset */
    const u8* dictBase = (dictionary == NULL) ? NULL :
                           (dictDirective == usingDictCtx) ?
                            dictionary + dictSize - dictCtx->currentOffset :
                            dictionary + dictSize - startIndex;

    u8* op = (u8*) dest;
    u8* const olimit = op + maxOutputSize;

    u32 offset = 0;
    u32 forwardH;

    ////DEBUGLOG(5, "LZ4_compress_generic_validated: srcSize=%i, tableType=%u", inputSize, tableType);
    ////assert(ip != NULL);
    if (tableType == byu16) //assert(inputSize<LZ4_64Klimit);  /* Size too large (not within 64K limit) */
    if (tableType == byPtr) //assert(dictDirective==noDict);   /* only supported use case with byPtr */
    /* If init conditions are not met, we don't have to mark stream
     * as having dirty context, since no action was taken yet */
    if (outputDirective == fillOutput && maxOutputSize < 1) { return 0; } /* Impossible to store anything */
    ////assert(acceleration >= 1);

    lowLimit = (const u8*)source - (dictDirective == withPrefix64k ? dictSize : 0);

    /* Update context state */
    if (dictDirective == usingDictCtx) {
        /* Subsequent linked blocks can't use the dictionary. */
        /* Instead, they use the block we just compressed. */
        cctx->dictCtx = NULL;
        cctx->dictSize = (u32)inputSize;
    } else {
        cctx->dictSize += (u32)inputSize;
    }
    cctx->currentOffset += (u32)inputSize;
    cctx->tableType = (u32)tableType;

    if (inputSize<LZ4_minLength) goto _last_literals;        /* Input too small, no compression (all literals) */

    /* First Byte */
    {   u32 const h = LZ4_hashPosition(ip, tableType);
        if (tableType == byPtr) {
            LZ4_putPositionOnHash(ip, h, cctx->hashTable, byPtr);
        } else {
            LZ4_putIndexOnHash(startIndex, h, cctx->hashTable, tableType);
    }   }
    ip++; forwardH = LZ4_hashPosition(ip, tableType);

    /* Main Loop */
    for ( ; ; ) {
        const u8* match;
        u8* token;
        const u8* filledIp;

        /* Find a match */
        if (tableType == byPtr) {
            const u8* forwardIp = ip;
            int step = 1;
            int searchMatchNb = acceleration << LZ4_skipTrigger;
            do {
                u32 const h = forwardH;
                ip = forwardIp;
                forwardIp += step;
                step = (searchMatchNb++ >> LZ4_skipTrigger);

                if (unlikely(forwardIp > mflimitPlusOne)) goto _last_literals;
                //assert(ip < mflimitPlusOne);

                match = LZ4_getPositionOnHash(h, cctx->hashTable, tableType);
                forwardH = LZ4_hashPosition(forwardIp, tableType);
                LZ4_putPositionOnHash(ip, h, cctx->hashTable, tableType);

            } while ( (match+LZ4_DISTANCE_MAX < ip)
                   || (LZ4_read32(match) != LZ4_read32(ip)) );

        } else {   /* byu32, byu16 */

            const u8* forwardIp = ip;
            int step = 1;
            int searchMatchNb = acceleration << LZ4_skipTrigger;
            do {
                u32 const h = forwardH;
                u32 const current = (u32)(forwardIp - base);
                u32 matchIndex = LZ4_getIndexOnHash(h, cctx->hashTable, tableType);
                ////assert(matchIndex <= current);
                ////assert(forwardIp - base < (ptrdiff_t)(2 GB - 1));
                ip = forwardIp;
                forwardIp += step;
                step = (searchMatchNb++ >> LZ4_skipTrigger);

                if (unlikely(forwardIp > mflimitPlusOne)) goto _last_literals;
                ////assert(ip < mflimitPlusOne);

                if (dictDirective == usingDictCtx) {
                    if (matchIndex < startIndex) {
                        /* there was no match, try the dictionary */
                        ////assert(tableType == byu32);
                        matchIndex = LZ4_getIndexOnHash(h, dictCtx->hashTable, byu32);
                        match = dictBase + matchIndex;
                        matchIndex += dictDelta;   /* make dictCtx index comparable with current context */
                        lowLimit = dictionary;
                    } else {
                        match = base + matchIndex;
                        lowLimit = (const u8*)source;
                    }
                } else if (dictDirective == usingExtDict) {
                    if (matchIndex < startIndex) {
                        ////DEBUGLOG(7, "extDict candidate: matchIndex=%5u  <  startIndex=%5u", matchIndex, startIndex);
                        ////assert(startIndex - matchIndex >= MINMATCH);
                        ////assert(dictBase);
                        match = dictBase + matchIndex;
                        lowLimit = dictionary;
                    } else {
                        match = base + matchIndex;
                        lowLimit = (const u8*)source;
                    }
                } else {   /* single continuous memory segment */
                    match = base + matchIndex;
                }
                forwardH = LZ4_hashPosition(forwardIp, tableType);
                LZ4_putIndexOnHash(current, h, cctx->hashTable, tableType);

                ////DEBUGLOG(7, "candidate at pos=%u  (offset=%u \n", matchIndex, current - matchIndex);
                if ((dictIssue == dictSmall) && (matchIndex < prefixIdxLimit)) { continue; }    /* match outside of valid area */
                ////assert(matchIndex < current);
                if ( ((tableType != byu16) || (LZ4_DISTANCE_MAX < LZ4_DISTANCE_ABSOLUTE_MAX))
                  && (matchIndex+LZ4_DISTANCE_MAX < current)) {
                    continue;
                } /* too far */
                ////assert((current - matchIndex) <= LZ4_DISTANCE_MAX);  /* match now expected within distance */

                if (LZ4_read32(match) == LZ4_read32(ip)) {
                    if (maybe_extMem) offset = current - matchIndex;
                    break;   /* match found */
                }

            } while(1);
        }

        /* Catch up */
        filledIp = ip;
        while (((ip>anchor) & (match > lowLimit)) && (unlikely(ip[-1]==match[-1]))) { ip--; match--; }

        /* Encode Literals */
        {   unsigned const litLength = (unsigned)(ip - anchor);
            token = op++;
            if ((outputDirective == limitedOutput) &&  /* Check output buffer overflow */
                (unlikely(op + litLength + (2 + 1 + LASTLITERALS) + (litLength/255) > olimit)) ) {
                return 0;   /* cannot compress within `dst` budget. Stored indexes in hash table are nonetheless fine */
            }
            if ((outputDirective == fillOutput) &&
                (unlikely(op + (litLength+240)/255 /* litlen */ + litLength /* literals */ + 2 /* offset */ + 1 /* token */ + MFLIMIT - MINMATCH /* min last literals so last match is <= end - MFLIMIT */ > olimit))) {
                op--;
                goto _last_literals;
            }
            if (litLength >= RUN_MASK) {
                int len = (int)(litLength - RUN_MASK);
                *token = (RUN_MASK<<ML_BITS);
                for(; len >= 255 ; len-=255) *op++ = 255;
                *op++ = (u8)len;
            }
            else *token = (u8)(litLength<<ML_BITS);

            /* Copy Literals */
            LZ4_wildCopy8(op, anchor, op+litLength);
            op+=litLength;
            ////DEBUGLOG(6, "seq.start:%i, literals=%u, match.start:%i",
                        //(int)(anchor-(const u8*)source), litLength, (int)(ip-(const u8*)source));
        }

_next_match:
        /* at this stage, the following variables must be correctly set :
         * - ip : at start of LZ operation
         * - match : at start of previous pattern occurrence; can be within current prefix, or within extDict
         * - offset : if maybe_ext_memSegment==1 (constant)
         * - lowLimit : must be == dictionary to mean "match is within extDict"; must be == source otherwise
         * - token and *token : position to write 4-bits for match length; higher 4-bits for literal length supposed already written
         */

        if ((outputDirective == fillOutput) &&
            (op + 2 /* offset */ + 1 /* token */ + MFLIMIT - MINMATCH /* min last literals so last match is <= end - MFLIMIT */ > olimit)) {
            /* the match was too close to the end, rewind and go to last literals */
            op = token;
            goto _last_literals;
        }

        /* Encode Offset */
        if (maybe_extMem) {   /* static test */
            //DEBUGLOG(6, "             with offset=%u  (ext if > %i)", offset, (int)(ip - (const u8*)source));
            //assert(offset <= LZ4_DISTANCE_MAX && offset > 0);
            LZ4_writeLE16(op, (u16)offset); op+=2;
        } else  {
            //DEBUGLOG(6, "             with offset=%u  (same segment)", (u32)(ip - match));
            //assert(ip-match <= LZ4_DISTANCE_MAX);
            LZ4_writeLE16(op, (u16)(ip - match)); op+=2;
        }

        /* Encode MatchLength */
        {   unsigned matchCode;

            if ( (dictDirective==usingExtDict || dictDirective==usingDictCtx)
              && (lowLimit==dictionary) /* match within extDict */ ) {
                const u8* limit = ip + (dictEnd-match);
                //assert(dictEnd > match);
                if (limit > matchlimit) limit = matchlimit;
                matchCode = LZ4_count(ip+MINMATCH, match+MINMATCH, limit);
                ip += (s32)matchCode + MINMATCH;
                if (ip==limit) {
                    unsigned const more = LZ4_count(limit, (const u8*)source, matchlimit);
                    matchCode += more;
                    ip += more;
                }
                //DEBUGLOG(6, "             with matchLength=%u starting in extDict", matchCode+MINMATCH);
            } else {
                matchCode = LZ4_count(ip+MINMATCH, match+MINMATCH, matchlimit);
                ip += (s32)matchCode + MINMATCH;
                //DEBUGLOG(6, "             with matchLength=%u", matchCode+MINMATCH);
            }

            if ((outputDirective) &&    /* Check output buffer overflow */
                (unlikely(op + (1 + LASTLITERALS) + (matchCode+240)/255 > olimit)) ) {
                if (outputDirective == fillOutput) {
                    /* Match description too long : reduce it */
                    u32 newMatchCode = 15 /* in token */ - 1 /* to avoid needing a zero byte */ + ((u32)(olimit - op) - 1 - LASTLITERALS) * 255;
                    ip -= matchCode - newMatchCode;
                    //assert(newMatchCode < matchCode);
                    matchCode = newMatchCode;
                    if (unlikely(ip <= filledIp)) {
                        /* We have already filled up to filledIp so if ip ends up less than filledIp
                         * we have positions in the hash table beyond the current position. This is
                         * a problem if we reuse the hash table. So we have to remove these positions
                         * from the hash table.
                         */
                        const u8* ptr;
                        //DEBUGLOG(5, "Clearing %u positions", (u32)(filledIp - ip));
                        for (ptr = ip; ptr <= filledIp; ++ptr) {
                            u32 const h = LZ4_hashPosition(ptr, tableType);
                            LZ4_clearHash(h, cctx->hashTable, tableType);
                        }
                    }
                } else {
                    //assert(outputDirective == limitedOutput);
                    return 0;   /* cannot compress within `dst` budget. Stored indexes in hash table are nonetheless fine */
                }
            }
            if (matchCode >= ML_MASK) {
                *token += ML_MASK;
                matchCode -= ML_MASK;
                LZ4_write32(op, 0xFFFFFFFF);
                while (matchCode >= 4*255) {
                    op+=4;
                    LZ4_write32(op, 0xFFFFFFFF);
                    matchCode -= 4*255;
                }
                op += matchCode / 255;
                *op++ = (u8)(matchCode % 255);
            } else
                *token += (u8)(matchCode);
        }
        /* Ensure we have enough space for the last literals. */
        //assert(!(outputDirective == fillOutput && op + 1 + LASTLITERALS > olimit));

        anchor = ip;

        /* Test end of chunk */
        if (ip >= mflimitPlusOne) break;

        /* Fill table */
        {   u32 const h = LZ4_hashPosition(ip-2, tableType);
            if (tableType == byPtr) {
                LZ4_putPositionOnHash(ip-2, h, cctx->hashTable, byPtr);
            } else {
                u32 const idx = (u32)((ip-2) - base);
                LZ4_putIndexOnHash(idx, h, cctx->hashTable, tableType);
        }   }

        /* Test next position */
        if (tableType == byPtr) {

            match = LZ4_getPosition(ip, cctx->hashTable, tableType);
            LZ4_putPosition(ip, cctx->hashTable, tableType);
            if ( (match+LZ4_DISTANCE_MAX >= ip)
              && (LZ4_read32(match) == LZ4_read32(ip)) )
            { token=op++; *token=0; goto _next_match; }

        } else {   /* byu32, byu16 */

            u32 const h = LZ4_hashPosition(ip, tableType);
            u32 const current = (u32)(ip-base);
            u32 matchIndex = LZ4_getIndexOnHash(h, cctx->hashTable, tableType);
            //assert(matchIndex < current);
            if (dictDirective == usingDictCtx) {
                if (matchIndex < startIndex) {
                    /* there was no match, try the dictionary */
                    //assert(tableType == byu32);
                    matchIndex = LZ4_getIndexOnHash(h, dictCtx->hashTable, byu32);
                    match = dictBase + matchIndex;
                    lowLimit = dictionary;   /* required for match length counter */
                    matchIndex += dictDelta;
                } else {
                    match = base + matchIndex;
                    lowLimit = (const u8*)source;  /* required for match length counter */
                }
            } else if (dictDirective==usingExtDict) {
                if (matchIndex < startIndex) {
                    //assert(dictBase);
                    match = dictBase + matchIndex;
                    lowLimit = dictionary;   /* required for match length counter */
                } else {
                    match = base + matchIndex;
                    lowLimit = (const u8*)source;   /* required for match length counter */
                }
            } else {   /* single memory segment */
                match = base + matchIndex;
            }
            LZ4_putIndexOnHash(current, h, cctx->hashTable, tableType);
            //assert(matchIndex < current);
            if ( ((dictIssue==dictSmall) ? (matchIndex >= prefixIdxLimit) : 1)
              && (((tableType==byu16) && (LZ4_DISTANCE_MAX == LZ4_DISTANCE_ABSOLUTE_MAX)) ? 1 : (matchIndex+LZ4_DISTANCE_MAX >= current))
              && (LZ4_read32(match) == LZ4_read32(ip)) ) {
                token=op++;
                *token=0;
                if (maybe_extMem) offset = current - matchIndex;
                //DEBUGLOG(6, "seq.start:%i, literals=%u, match.start:%i",
                            //(int)(anchor-(const u8*)source), 0, (int)(ip-(const u8*)source));
                goto _next_match;
            }
        }

        /* Prepare next loop */
        forwardH = LZ4_hashPosition(++ip, tableType);

    }

_last_literals:
    /* Encode Last Literals */
    {   s32 lastRun = (s32)(iend - anchor);
        if ( (outputDirective) &&  /* Check output buffer overflow */
            (op + lastRun + 1 + ((lastRun+255-RUN_MASK)/255) > olimit)) {
            if (outputDirective == fillOutput) {
                /* adapt lastRun to fill 'dst' */
                //assert(olimit >= op);
                lastRun  = (s32)(olimit-op) - 1/*token*/;
                lastRun -= (lastRun + 256 - RUN_MASK) / 256;  /*additional length tokens*/
            } else {
                //assert(outputDirective == limitedOutput);
                return 0;   /* cannot compress within `dst` budget. Stored indexes in hash table are nonetheless fine */
            }
        }
        //DEBUGLOG(6, "Final literal run : %i literals", (int)lastRun);
        if (lastRun >= RUN_MASK) {
            s32 accumulator = lastRun - RUN_MASK;
            *op++ = RUN_MASK << ML_BITS;
            for(; accumulator >= 255 ; accumulator-=255) *op++ = 255;
            *op++ = (u8) accumulator;
        } else {
            *op++ = (u8)(lastRun<<ML_BITS);
        }
        ct_memcpy(op, anchor, lastRun);
        ip = anchor + lastRun;
        op += lastRun;
    }

    if (outputDirective == fillOutput) {
        *inputConsumed = (int) (((const char*)ip)-source);
    }
    result = (int)(((char*)op) - dest);
    //assert(result > 0);
    //DEBUGLOG(5, "LZ4_compress_generic: compressed %i bytes into %i bytes", inputSize, result);
    return result;
}

__attribute__((optimize("O2"))) int LZ4_compress_generic(
                 LZ4_stream_t_internal* const cctx,
                 const char* const src,
                 char* const dst,
                 const int srcSize,
                 int *inputConsumed, /* only written when outputDirective == fillOutput */
                 const int dstCapacity,
                 const limitedOutput_directive outputDirective,
                 const tableType_t tableType,
                 const dict_directive dictDirective,
                 const dictIssue_directive dictIssue,
                 const int acceleration)
{
    //DEBUGLOG(5, "LZ4_compress_generic: srcSize=%i, dstCapacity=%i",
                //srcSize, dstCapacity);

    if ((u32)srcSize > (u32)LZ4_MAX_INPUT_SIZE) { return 0; }  /* Unsupported srcSize, too large (or negative) */
    if (srcSize == 0) {   /* src == NULL supported if srcSize == 0 */
        if (outputDirective != notLimited && dstCapacity <= 0) return 0;  /* no output, can't write anything */
        ////DEBUGLOG(5, "Generating an empty block");
        ////assert(outputDirective == notLimited || dstCapacity >= 1);
        ////assert(dst != NULL);
        dst[0] = 0;
        if (outputDirective == fillOutput) {
            ////assert (inputConsumed != NULL);
            *inputConsumed = 0;
        }
        return 1;
    }
    ////assert(src != NULL);

    return LZ4_compress_generic_validated(cctx, src, dst, srcSize,
                inputConsumed, /* only written into if outputDirective == fillOutput */
                dstCapacity, outputDirective,
                tableType, dictDirective, dictIssue, acceleration);
}

int LZ4_compress_fast_extState(void* state, const char* source, char* dest, int inputSize, int maxOutputSize, int acceleration)
{
    LZ4_stream_t_internal* const ctx = & LZ4_initStream(state, sizeof(LZ4_stream_t)) -> internal_donotuse;
    ////assert(ctx != NULL);
    if (acceleration < 1) acceleration = LZ4_ACCELERATION_DEFAULT;
    if (acceleration > LZ4_ACCELERATION_MAX) acceleration = LZ4_ACCELERATION_MAX;
    if (maxOutputSize >= LZ4_compressBound(inputSize)) {
        if (inputSize < LZ4_64Klimit) {
            return LZ4_compress_generic(ctx, source, dest, inputSize, NULL, 0, notLimited, byu16, noDict, noDictIssue, acceleration);
        } else {
            const tableType_t tableType = ((sizeof(void*)==4) && ((uptrval)source > LZ4_DISTANCE_MAX)) ? byPtr : byu32;
            return LZ4_compress_generic(ctx, source, dest, inputSize, NULL, 0, notLimited, tableType, noDict, noDictIssue, acceleration);
        }
    } else {
        if (inputSize < LZ4_64Klimit) {
            return LZ4_compress_generic(ctx, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, byu16, noDict, noDictIssue, acceleration);
        } else {
            const tableType_t tableType = ((sizeof(void*)==4) && ((uptrval)source > LZ4_DISTANCE_MAX)) ? byPtr : byu32;
            return LZ4_compress_generic(ctx, source, dest, inputSize, NULL, maxOutputSize, limitedOutput, tableType, noDict, noDictIssue, acceleration);
        }
    }
}

int LZ4_compress_fast(const char* src, char* dest, int srcSize, int dstCapacity, int acceleration)
{
    int result;
#if (LZ4_HEAPMODE)
    LZ4_stream_t* const ctxPtr = (LZ4_stream_t*)ALLOC(sizeof(LZ4_stream_t));   /* malloc-calloc always properly aligned */
    if (ctxPtr == NULL) return 0;
#else
    LZ4_stream_t ctx;
    LZ4_stream_t* const ctxPtr = &ctx;
#endif
    result = LZ4_compress_fast_extState(ctxPtr, src, dest, srcSize, dstCapacity, acceleration);

#if (LZ4_HEAPMODE)
    FREEMEM(ctxPtr);
#endif
    return result;
}

int LZ4_compress_default(const char* src, char* dst, int srcSize, int dstCapacity)
{
    return LZ4_compress_fast(src, dst, srcSize, dstCapacity, 1);
}




/* Decompression functions */





static const unsigned inc32table[8] = {0, 1, 2,  1,  0,  4, 4, 4};
static const int      dec64table[8] = {0, 0, 0, -1, -4,  1, 2, 3};



/*! LZ4_streamDecode_t :
 *  Never ever use below internal definitions directly !
 *  These definitions are not API/ABI safe, and may change in future versions.
 *  If you need static allocation, declare or allocate an LZ4_streamDecode_t object.
**/
typedef struct {
    const u8* externalDict;
    const u8* prefixEnd;
    s32 extDictSize;
    s32 prefixSize;
} LZ4_streamDecode_t_internal;

#define LZ4_STREAMDECODE_MINSIZE 32
union LZ4_streamDecode_u {
    char minStateSize[LZ4_STREAMDECODE_MINSIZE];
    LZ4_streamDecode_t_internal internal_donotuse;
} ;   /* previously typedef'd to LZ4_streamDecode_t */

typedef union LZ4_streamDecode_u LZ4_streamDecode_t;   /* tracking context */







/*-*******************************
 *  Decompression functions
 ********************************/

typedef enum { decode_full_block = 0, partial_decode = 1 } earlyEnd_directive;
#define LZ4_FORCE_INLINE static inline __attribute__((always_inline))

#undef MIN
#define MIN(a,b)    ( (a) < (b) ? (a) : (b) )

void DEBUGLOG(void* temp, ...) {
    return;
}

void assert(s32 temp) {
    return;
}

/* Read the variable-length literal or match length.
 *
 * @ip : input pointer
 * @ilimit : position after which if length is not decoded, the input is necessarily corrupted.
 * @initial_check - check ip >= ipmax before start of loop.  Returns initial_error if so.
 * @error (output) - error code.  Must be set to 0 before call.
**/
typedef s32 Rvl_t;
static const Rvl_t rvl_error = (Rvl_t)(-1);
LZ4_FORCE_INLINE Rvl_t
read_variable_length(const u8** ip, const u8* ilimit,
                     int initial_check)
{
    Rvl_t s, length = 0;
    assert(ip != NULL);
    assert(*ip !=  NULL);
    assert(ilimit != NULL);
    if (initial_check && unlikely((*ip) >= ilimit)) {    /* read limit reached */
        return rvl_error;
    }
    do {
        s = **ip;
        (*ip)++;
        length += s;
        if (unlikely((*ip) > ilimit)) {    /* read limit reached */
            return rvl_error;
        }
        /* accumulator overflow detection (32-bit mode only) */
        if ((sizeof(length)<8) && unlikely(length > ((Rvl_t)(-1)/2)) ) {
            return rvl_error;
        }
    } while (s==255);

    return length;
}

/*! LZ4_decompress_generic() :
 *  This generic decompression function covers all use cases.
 *  It shall be instantiated several times, using different sets of directives.
 *  Note that it is important for performance that this function really get inlined,
 *  in order to remove useless branches during compilation optimization.
 */

static u16 LZ4_readLE16(const void* memPtr)
{
    const u8* p = (const u8*)memPtr;
    return (u16)((u16)p[0] + (p[1]<<8));
}

LZ4_FORCE_INLINE int
LZ4_decompress_generic(
                 const char* const src,
                 char* const dst,
                 int srcSize,
                 int outputSize,         /* If endOnInput==endOnInputSize, this value is `dstCapacity` */

                 earlyEnd_directive partialDecoding,  /* full, partial */
                 dict_directive dict,                 /* noDict, withPrefix64k, usingExtDict */
                 const u8* const lowPrefix,  /* always <= dst, == dst when no prefix */
                 const u8* const dictStart,  /* only if dict==usingExtDict */
                 const s32 dictSize         /* note : = 0 if noDict */
                 )
{
    if ((src == NULL) || (outputSize < 0)) { return -1; }

    {   const u8* ip = (const u8*) src;
        const u8* const iend = ip + srcSize;

        u8* op = (u8*) dst;
        u8* const oend = op + outputSize;
        u8* cpy;

        const u8* const dictEnd = (dictStart == NULL) ? NULL : dictStart + dictSize;

        const int checkOffset = (dictSize < (int)(64 KB));


        /* Set up the "end" pointers for the shortcut. */
        const u8* const shortiend = iend - 14 /*maxLL*/ - 2 /*offset*/;
        const u8* const shortoend = oend - 14 /*maxLL*/ - 18 /*maxML*/;

        const u8* match;
        s32 offset;
        unsigned token;
        s32 length;


        DEBUGLOG(5, "LZ4_decompress_generic (srcSize:%i, dstSize:%i)", srcSize, outputSize);

        /* Special cases */
        assert(lowPrefix <= op);
        if (unlikely(outputSize==0)) {
            /* Empty output buffer */
            if (partialDecoding) return 0;
            return ((srcSize==1) && (*ip==0)) ? 0 : -1;
        }
        if (unlikely(srcSize==0)) { return -1; }

    /* LZ4_FAST_DEC_LOOP:
     * designed for modern OoO performance cpus,
     * where copying reliably 32-bytes is preferable to an unpredictable branch.
     * note : fast loop may show a regression for some client arm chips. */
#if LZ4_FAST_DEC_LOOP
        if ((oend - op) < FASTLOOP_SAFE_DISTANCE) {
            DEBUGLOG(6, "skip fast decode loop");
            goto safe_decode;
        }

        /* Fast loop : decode sequences as long as output < oend-FASTLOOP_SAFE_DISTANCE */
        DEBUGLOG(6, "using fast decode loop");
        while (1) {
            /* Main fastloop assertion: We can always wildcopy FASTLOOP_SAFE_DISTANCE */
            assert(oend - op >= FASTLOOP_SAFE_DISTANCE);
            assert(ip < iend);
            token = *ip++;
            length = token >> ML_BITS;  /* literal length */

            /* decode literal length */
            if (length == RUN_MASK) {
                s32 const addl = read_variable_length(&ip, iend-RUN_MASK, 1);
                if (addl == rvl_error) {
                    DEBUGLOG(6, "error reading long literal length");
                    goto _output_error;
                }
                length += addl;
                if (unlikely((uptrval)(op)+length<(uptrval)(op))) { goto _output_error; } /* overflow detection */
                if (unlikely((uptrval)(ip)+length<(uptrval)(ip))) { goto _output_error; } /* overflow detection */

                /* copy literals */
                cpy = op+length;
                //LZ4_STATIC_ASSERT(MFLIMIT >= WILDCOPYLENGTH);
                if ((cpy>oend-32) || (ip+length>iend-32)) { goto safe_literal_copy; }
                LZ4_wildCopy32(op, ip, cpy);
                ip += length; op = cpy;
            } else {
                cpy = op+length;
                DEBUGLOG(7, "copy %u bytes in a 16-bytes stripe", (unsigned)length);
                /* We don't need to check oend, since we check it once for each loop below */
                if (ip > iend-(16 + 1/*max lit + offset + nextToken*/)) { goto safe_literal_copy; }
                /* Literals can only be <= 14, but hope compilers optimize better when copy by a register size */
                ct_memcpy(op, ip, 16);
                ip += length; op = cpy;
            }

            /* get offset */
            offset = LZ4_readLE16(ip); ip+=2;
            DEBUGLOG(6, " offset = %zu", offset);
            match = op - offset;
            assert(match <= op);  /* overflow check */

            /* get matchlength */
            length = token & ML_MASK;

            if (length == ML_MASK) {
                s32 const addl = read_variable_length(&ip, iend - LASTLITERALS + 1, 0);
                if (addl == rvl_error) {
                    DEBUGLOG(6, "error reading long match length");
                    goto _output_error;
                }
                length += addl;
                length += MINMATCH;
                if (unlikely((uptrval)(op)+length<(uptrval)op)) { goto _output_error; } /* overflow detection */
                if ((checkOffset) && (unlikely(match + dictSize < lowPrefix))) {
                    DEBUGLOG(6, "Error : offset outside buffers");
                    goto _output_error;
                }
                if (op + length >= oend - FASTLOOP_SAFE_DISTANCE) {
                    goto safe_match_copy;
                }
            } else {
                length += MINMATCH;
                if (op + length >= oend - FASTLOOP_SAFE_DISTANCE) {
                    goto safe_match_copy;
                }

                /* Fastpath check: skip LZ4_wildCopy32 when true */
                if ((dict == withPrefix64k) || (match >= lowPrefix)) {
                    if (offset >= 8) {
                        assert(match >= lowPrefix);
                        assert(match <= op);
                        assert(op + 18 <= oend);

                        ct_memcpy(op, match, 8);
                        ct_memcpy(op+8, match+8, 8);
                        ct_memcpy(op+16, match+16, 2);
                        op += length;
                        continue;
            }   }   }

            if ( checkOffset && (unlikely(match + dictSize < lowPrefix)) ) {
                DEBUGLOG(6, "Error : pos=%zi, offset=%zi => outside buffers", op-lowPrefix, op-match);
                goto _output_error;
            }
            /* match starting within external dictionary */
            if ((dict==usingExtDict) && (match < lowPrefix)) {
                assert(dictEnd != NULL);
                if (unlikely(op+length > oend-LASTLITERALS)) {
                    if (partialDecoding) {
                        DEBUGLOG(7, "partialDecoding: dictionary match, close to dstEnd");
                        length = MIN(length, (s32)(oend-op));
                    } else {
                        DEBUGLOG(6, "end-of-block condition violated");
                        goto _output_error;
                }   }

                if (length <= (s32)(lowPrefix-match)) {
                    /* match fits entirely within external dictionary : just copy */
                    LZ4_memmove(op, dictEnd - (lowPrefix-match), length);
                    op += length;
                } else {
                    /* match stretches into both external dictionary and current block */
                    s32 const copySize = (s32)(lowPrefix - match);
                    s32 const restSize = length - copySize;
                    ct_memcpy(op, dictEnd - copySize, copySize);
                    op += copySize;
                    if (restSize > (s32)(op - lowPrefix)) {  /* overlap copy */
                        u8* const endOfMatch = op + restSize;
                        const u8* copyFrom = lowPrefix;
                        while (op < endOfMatch) { *op++ = *copyFrom++; }
                    } else {
                        ct_memcpy(op, lowPrefix, restSize);
                        op += restSize;
                }   }
                continue;
            }

            /* copy match within block */
            cpy = op + length;

            assert((op <= oend) && (oend-op >= 32));
            if (unlikely(offset<16)) {
                ct_memcpy_using_offset(op, match, cpy, offset);
            } else {
                LZ4_wildCopy32(op, match, cpy);
            }

            op = cpy;   /* wildcopy correction */
        }
    safe_decode:
#endif

        /* Main Loop : decode remaining sequences where output < FASTLOOP_SAFE_DISTANCE */
        DEBUGLOG(6, "using safe decode loop");
        while (1) {
            assert(ip < iend);
            token = *ip++;
            length = token >> ML_BITS;  /* literal length */

            /* A two-stage shortcut for the most common case:
             * 1) If the literal length is 0..14, and there is enough space,
             * enter the shortcut and copy 16 bytes on behalf of the literals
             * (in the fast mode, only 8 bytes can be safely copied this way).
             * 2) Further if the match length is 4..18, copy 18 bytes in a similar
             * manner; but we ensure that there's enough space in the output for
             * those 18 bytes earlier, upon entering the shortcut (in other words,
             * there is a combined check for both stages).
             */
            if ( (length != RUN_MASK)
                /* strictly "less than" on input, to re-enter the loop with at least one byte */
              && likely((ip < shortiend) & (op <= shortoend)) ) {
                /* Copy the literals */
                ct_memcpy(op, ip, 16);
                op += length; ip += length;

                /* The second stage: prepare for match copying, decode full info.
                 * If it doesn't work out, the info won't be wasted. */
                length = token & ML_MASK; /* match length */
                offset = LZ4_readLE16(ip); ip += 2;
                match = op - offset;
                assert(match <= op); /* check overflow */

                /* Do not deal with overlapping matches. */
                if ( (length != ML_MASK)
                  && (offset >= 8)
                  && (dict==withPrefix64k || match >= lowPrefix) ) {
                    /* Copy the match. */
                    ct_memcpy(op + 0, match + 0, 8);
                    ct_memcpy(op + 8, match + 8, 8);
                    ct_memcpy(op +16, match +16, 2);
                    op += length + MINMATCH;
                    /* Both stages worked, load the next token. */
                    continue;
                }

                /* The second stage didn't work out, but the info is ready.
                 * Propel it right to the point of match copying. */
                goto _copy_match;
            }

            /* decode literal length */
            if (length == RUN_MASK) {
                s32 const addl = read_variable_length(&ip, iend-RUN_MASK, 1);
                if (addl == rvl_error) { goto _output_error; }
                length += addl;
                if (unlikely((uptrval)(op)+length<(uptrval)(op))) { goto _output_error; } /* overflow detection */
                if (unlikely((uptrval)(ip)+length<(uptrval)(ip))) { goto _output_error; } /* overflow detection */
            }

            /* copy literals */
            cpy = op+length;
#if LZ4_FAST_DEC_LOOP
        safe_literal_copy:
#endif
            //LZ4_STATIC_ASSERT(MFLIMIT >= WILDCOPYLENGTH);
            if ((cpy>oend-MFLIMIT) || (ip+length>iend-(2+1+LASTLITERALS))) {
                /* We've either hit the input parsing restriction or the output parsing restriction.
                 * In the normal scenario, decoding a full block, it must be the last sequence,
                 * otherwise it's an error (invalid input or dimensions).
                 * In partialDecoding scenario, it's necessary to ensure there is no buffer overflow.
                 */
                if (partialDecoding) {
                    /* Since we are partial decoding we may be in this block because of the output parsing
                     * restriction, which is not valid since the output buffer is allowed to be undersized.
                     */
                    DEBUGLOG(7, "partialDecoding: copying literals, close to input or output end");
                    DEBUGLOG(7, "partialDecoding: literal length = %u", (unsigned)length);
                    DEBUGLOG(7, "partialDecoding: remaining space in dstBuffer : %i", (int)(oend - op));
                    DEBUGLOG(7, "partialDecoding: remaining space in srcBuffer : %i", (int)(iend - ip));
                    /* Finishing in the middle of a literals segment,
                     * due to lack of input.
                     */
                    if (ip+length > iend) {
                        length = (s32)(iend-ip);
                        cpy = op + length;
                    }
                    /* Finishing in the middle of a literals segment,
                     * due to lack of output space.
                     */
                    if (cpy > oend) {
                        cpy = oend;
                        assert(op<=oend);
                        length = (s32)(oend-op);
                    }
                } else {
                     /* We must be on the last sequence (or invalid) because of the parsing limitations
                      * so check that we exactly consume the input and don't overrun the output buffer.
                      */
                    if ((ip+length != iend) || (cpy > oend)) {
                        DEBUGLOG(6, "should have been last run of literals");
                        DEBUGLOG(6, "ip(%p) + length(%i) = %p != iend (%p)", ip, (int)length, ip+length, iend);
                        DEBUGLOG(6, "or cpy(%p) > oend(%p)", cpy, oend);
                        goto _output_error;
                    }
                }
                //LZ4_memmove(op, ip, length);  /* supports overlapping memory regions, for in-place decompression scenarios */
                ct_memcpy(op, ip, length);  /* supports overlapping memory regions, for in-place decompression scenarios */
                ip += length;
                op += length;
                /* Necessarily EOF when !partialDecoding.
                 * When partialDecoding, it is EOF if we've either
                 * filled the output buffer or
                 * can't proceed with reading an offset for following match.
                 */
                if (!partialDecoding || (cpy == oend) || (ip >= (iend-2))) {
                    break;
                }
            } else {
                LZ4_wildCopy8(op, ip, cpy);   /* can overwrite up to 8 bytes beyond cpy */
                ip += length; op = cpy;
            }

            /* get offset */
            offset = LZ4_readLE16(ip); ip+=2;
            match = op - offset;

            /* get matchlength */
            length = token & ML_MASK;

    _copy_match:
            if (length == ML_MASK) {
                s32 const addl = read_variable_length(&ip, iend - LASTLITERALS + 1, 0);
                if (addl == rvl_error) { goto _output_error; }
                length += addl;
                if (unlikely((uptrval)(op)+length<(uptrval)op)) goto _output_error;   /* overflow detection */
            }
            length += MINMATCH;

#if LZ4_FAST_DEC_LOOP
        safe_match_copy:
#endif
            if ((checkOffset) && (unlikely(match + dictSize < lowPrefix))) goto _output_error;   /* Error : offset outside buffers */
            /* match starting within external dictionary */
            if ((dict==usingExtDict) && (match < lowPrefix)) {
                assert(dictEnd != NULL);
                if (unlikely(op+length > oend-LASTLITERALS)) {
                    if (partialDecoding) length = MIN(length, (s32)(oend-op));
                    else goto _output_error;   /* doesn't respect parsing restriction */
                }

                if (length <= (s32)(lowPrefix-match)) {
                    /* match fits entirely within external dictionary : just copy */
                    LZ4_memmove(op, dictEnd - (lowPrefix-match), length);
                    op += length;
                } else {
                    /* match stretches into both external dictionary and current block */
                    s32 const copySize = (s32)(lowPrefix - match);
                    s32 const restSize = length - copySize;
                    ct_memcpy(op, dictEnd - copySize, copySize);
                    op += copySize;
                    if (restSize > (s32)(op - lowPrefix)) {  /* overlap copy */
                        u8* const endOfMatch = op + restSize;
                        const u8* copyFrom = lowPrefix;
                        while (op < endOfMatch) *op++ = *copyFrom++;
                    } else {
                        ct_memcpy(op, lowPrefix, restSize);
                        op += restSize;
                }   }
                continue;
            }
            assert(match >= lowPrefix);

            /* copy match within block */
            cpy = op + length;

            /* partialDecoding : may end anywhere within the block */
            assert(op<=oend);
            if (partialDecoding && (cpy > oend-MATCH_SAFEGUARD_DISTANCE)) {
                s32 const mlen = MIN(length, (s32)(oend-op));
                const u8* const matchEnd = match + mlen;
                u8* const copyEnd = op + mlen;
                if (matchEnd > op) {   /* overlap copy */
                    while (op < copyEnd) { *op++ = *match++; }
                } else {
                    ct_memcpy(op, match, mlen);
                }
                op = copyEnd;
                if (op == oend) { break; }
                continue;
            }

            if (unlikely(offset<8)) {
                LZ4_write32(op, 0);   /* silence msan warning when offset==0 */
                op[0] = match[0];
                op[1] = match[1];
                op[2] = match[2];
                op[3] = match[3];
                match += inc32table[offset];
                ct_memcpy(op+4, match, 4);
                match -= dec64table[offset];
            } else {
                ct_memcpy(op, match, 8);
                match += 8;
            }
            op += 8;

            if (unlikely(cpy > oend-MATCH_SAFEGUARD_DISTANCE)) {
                u8* const oCopyLimit = oend - (WILDCOPYLENGTH-1);
                if (cpy > oend-LASTLITERALS) { goto _output_error; } /* Error : last LASTLITERALS bytes must be literals (uncompressed) */
                if (op < oCopyLimit) {
                    LZ4_wildCopy8(op, match, oCopyLimit);
                    match += oCopyLimit - op;
                    op = oCopyLimit;
                }
                while (op < cpy) { *op++ = *match++; }
            } else {
                ct_memcpy(op, match, 8);
                if (length > 16)  { LZ4_wildCopy8(op+8, match+8, cpy); }
            }
            op = cpy;   /* wildcopy correction */
        }

        /* end of decoding */
        DEBUGLOG(5, "decoded %i bytes", (int) (((char*)op)-dst));
        return (int) (((char*)op)-dst);     /* Nb of output bytes decoded */

        /* Overflow error detected */
    _output_error:
        return (int) (-(((const char*)ip)-src))-1;
    }
}

__attribute__((optimize("O2"))) int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize)
{
    return LZ4_decompress_generic(source, dest, compressedSize, maxDecompressedSize,
                                  decode_full_block, noDict,
                                  (u8*)dest, NULL, 0);
}

enum {
    BLOCK_BYTES = 1024 * 8,
//  BLOCK_BYTES = 1024 * 64,
};

s32 write_bin(void* fp, const void* array, s32 arrayBytes) {
    //return fwrite(array, 1, arrayBytes, fp);
    ct_memcpy(fp, array, arrayBytes);
}

int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize)
{
    LZ4_streamDecode_t_internal* lz4sd = &LZ4_streamDecode->internal_donotuse;
    lz4sd->prefixSize = (s32)dictSize;
    if (dictSize) {
        assert(dictionary != NULL);
        lz4sd->prefixEnd = (const u8*) dictionary + dictSize;
    } else {
        lz4sd->prefixEnd = (const u8*) dictionary;
    }
    lz4sd->externalDict = NULL;
    lz4sd->extDictSize  = 0;
    return 1;
}

typedef struct {
  char const* buf;
  s32 size;
  s32 pos;
} const_cursor_t;

typedef struct {
  char* buf;
  s32 size;
  s32 pos;
} cursor_t;

typedef union LZ4_streamHC_u LZ4_streamHC_t;   /* incomplete type (defined later) */

typedef struct {
  LZ4_stream_t* cstream;
  LZ4_streamHC_t* cstreamHC;
  LZ4_streamDecode_t* dstream;
  const_cursor_t data;
  cursor_t compressed;
  cursor_t roundTrip;
  u32 seed;
  int level;
} state_t;