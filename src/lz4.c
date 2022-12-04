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