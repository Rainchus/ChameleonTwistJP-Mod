#ifndef _CT1_H
#define _CT1_H
#include "types.h"
#include "OS.h"
#include "structs.h"

#define ramStartAddr 0x800EE1C0
#define ramEndAddr 0x803B5000

extern s32 savestateCompressedSize;

void compress_lz4_test2(const u8* srcData, int srcSize);
void decompress_lz4_test2(int srcSize);
extern void test_lz4(const u8* srcData, int srcSize);
extern void* my_memcpy(void* dst, const void* src, s32 n);
extern void* my_memmove(void* dest, const void* src, s32 n);

#define KB *(1 <<10)
#define MB *(1 <<20)
#define GB *(1U<<30)
extern int LZ4_compress_fast(const char* src, char* dest, int srcSize, int dstCapacity, int acceleration);

#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))

typedef char *outfun(char*,const char*,unsigned int);
int _Printf(outfun prout, char *arg, const char *fmt, va_list args);
void crash_screen_init(void);
void __osDisableInt(void);
void __osRestoreInt(void);

extern playerActor* P1Instance;
extern Tongue* TongueInstance;
extern playerActor p1;
extern Tongue tongue;

void printText(f32 xPos, f32 yPos, f32 zero, f32 scale, f32 zero2, f32 zero3, void* text, s32);
void _bzero(void* buffer, s32 size);
extern s32 debugBool;
void ct_memcpy(void* destination, void* src, s32 size);
void customMemCpy(void* destination, void* src, s32 size);
extern char compressBuffer[1024 * 1024];
extern char decompressBuffer[ramEndAddr - ramStartAddr];

extern u8 currentFileLevelUnlocks;
extern s16 p1ButtonsHeld;
extern s16 p1ButtonsPressed;
extern u64 __osCurrentTime;
extern u32 osMemSize;

typedef u64	OSTime;
#define	OS_CLOCK_RATE		62500000LL
#define	OS_CPU_COUNTER		(OS_CLOCK_RATE*3/4) // 46875000
#define OS_CYCLES_TO_USEC(c)	(((u64)(c)*(1000000LL/15625LL))/(OS_CPU_COUNTER/15625LL))
extern u32		osGetCount(void);

void osInvalICache(void*, s32);
void osInvalDCache(void*, s32);
int __osSpDeviceBusy();
int __osSiDeviceBusy();

extern volatile s32 stateCooldown;
extern volatile s32 stateFinishedBool;

typedef u8 uint8_t;


#endif