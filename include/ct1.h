#ifndef _CT1_H
#define _CT1_H
#include "types.h"
#include "OS.h"
#include "structs.h"

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


#endif