#ifndef _CT1_H
#define _CT1_H
#include "types.h"
#include "OS.h"
#include "structs.h"

#include "print.h"  // Custom Printing Functions
#include "menu.h"   // Custom Menu Functions

extern s32 currPageNo;
extern s32 currOptionNo;
extern u8 menuIsActive;

#define DPAD_UP 0x08000000
#define DPAD_DOWN 0x04000000
#define DPAD_LEFT 0x02000000
#define DPAD_RIGHT 0x01000000
#define L_BUTTON 0x00200000
#define R_BUTTON 0x00100000
#define B_BUTTON 0x40000000
#define A_BUTTON 0x80000000

extern u8 toggles[];
extern s32 textWhiteColor[];

enum Toggles {
    TOGGLE_HIDE_SAVESTATE_TEXT,
    TOGGLE_HIDE_IGT,
    TOGGLE_INFINITE_HEALTH,
    TOGGLE_CUSTOM_DEBUG_TEXT,
};

// Menu //
extern void updateMenuInput(void);
s32 givePlayerMaxCrowns(void);
extern void drawTimer(void);
extern void loadBoss(void);
// End Menu //



#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))
#define ramStartAddr (void*)0x800EE1C0
#define ramEndAddr (void*)0x803B5000
#define KB *(1 <<10)
#define MB *(1 <<20)
#define GB *(1U<<30)

// Controller and Inputs //
typedef struct OSContPad {
    unsigned short 	button;     /* Controller button data */
    char  			stick_x;    /* Control stick's X coordinate position*/
    char  			stick_y;    /* Control stick's Y coordinate position*/
    unsigned char  	errno;      /* Error values returned from the Controller */
} OSContPad;

extern s32 heldButtonsMain;
extern s32 currentlyPressedButtons;
extern s32 previouslyPressedButtons;
extern s32 previouslyHeldButtons;
// End Controller and Inputs //

// Save States //
extern volatile s32 saveOrLoadStateMode;
extern volatile s32 savestateCurrentSlot;
extern s32 savestate1Size;
extern s32 savestate2Size;
extern s32 savestate3Size;
extern s32 savestate4Size;
extern s32 stateModeDisplay;
// End Save States //

// Player 1 //
extern playerActor p1;
extern s8 p1Health; 
extern playerActor* P1Instance;
extern Tongue tongue;
extern Tongue* TongueInstance;
extern s16 p1ButtonsHeld;
extern s16 p1ButtonsPressed;
// End Player 1 //

extern u64 __osCurrentTime;
extern u32 osMemSize;
extern s32 freeCamActive; //0 fixed cam, 1 free cam

extern s32 isPaused;
extern s32 pauseFrameCountMode;
extern s8 currLevel;

// All Unlocks //
extern u8 currentFileLevelUnlocks;  // Flag
extern u8 currentFileLevelUnlocks2; // Flag
extern s8 jlCrowns;
extern s8 alCrowns;
extern s8 blCrowns;
extern s8 klCrowns;
extern s8 dcCrowns;
extern s8 gcCrowns;
extern u8 blackWhiteUnlock;         // Flag (0x0C)


// End All Unlocks //

// SFX // 
void playSound(s32, void*, s32);
// End SFX //



extern int getStatusRegister(void); //returns status reg
extern int setStatusRegister(s32); //returns status reg

extern void test_lz4(const u8* srcData, int srcSize);
extern void* my_memcpy(void* dst, const void* src, s32 n);
extern void* my_memmove(void* dest, const void* src, s32 n);

void decompress_lz4_ct_default(int srcSize, int savestateCompressedSize, u8* compressedBuffer);
s32 compress_lz4_ct_default(const u8* srcData, int srcSize, u8* bufferAddr);

extern int LZ4_decompress_safe_withPrefix64k(const char* source, char* dest, int compressedSize, int maxOutputSize);
extern int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize);
extern int LZ4_compress_fast(const char* src, char* dest, int srcSize, int dstCapacity, int acceleration);
extern int LZ4_compress_default(const char* src, char* dst, int srcSize, int dstCapacity);

extern void colorText(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
typedef char *outfun(char*,const char*,unsigned int);
int _Printf(outfun prout, char *arg, const char *fmt, va_list args);
void crash_screen_init(void);
void __osDisableInt(void);
void __osRestoreInt(void);
void printText(f32 xPos, f32 yPos, f32 zero, f32 scale, f32 zero2, f32 zero3, void* text, s32);
void _bzero(void* buffer, s32 size);
extern s32 debugBool;
void ct_memcpy(void* destination, void* src, s32 size);
void customMemCpy(void* destination, void* src, s32 size);
void infiniteLoop(void);
s32 ct_strlen(char* string);
void set_gp(void);
// extern char compressBuffer[1024 * 1024];
extern char decompressBuffer[0x803B5000 - 0x800EE1C0];
extern s32 isTakingLoadingZone;
extern s32 pauseFrameCount;

extern s32 isTakingLoadingZone;
extern s32 pauseFrameCount;

typedef u64	OSTime;
#define	OS_CLOCK_RATE		62500000LL
#define	OS_CPU_COUNTER		(OS_CLOCK_RATE*3/4) // 46875000
#define OS_CYCLES_TO_USEC(c)	(((u64)(c)*(1000000LL/15625LL))/(OS_CPU_COUNTER/15625LL))
extern OSTime		osGetCount(void);

void osInvalICache(void*, s32);
void osInvalDCache(void*, s32);
int __osDpDeviceBusy();
int __osSpDeviceBusy();
int __osSiDeviceBusy();
extern s32 rngSeed;

extern s32 gameMode;

enum GameModes {
 GAME_MODE_OVERWORLD = 0,
 GAME_MODE_JUNGLE_LAND_MENU = 1,
 GAME_MODE_SAVE_MENU = 2,
 GAME_MODE_LOAD_GAME_MENU = 3,
 GAME_MODE_DEMO = 4,
 GAME_MODE_DEMO_2 = 5,
 GAME_MODE_TITLE_SCREEN = 6,
 GAME_MODE_BATTLE_MENU = 7,
 GAME_MODE_OPTIONS_MENU = 8,
 GAME_MODE_GAME_OVER = 9,
 GAME_MODE_SUPPLY_SYSTEM_LOGO = 10,
 GAME_MODE_PRE_CREDITS = 11,
 GAME_MODE_NEW_GAME_MENU = 12,
 GAME_MODE_JUNGLE_LAND = 13,
 GAME_MODE_STAGE_SELECT = 14,
 GAME_MODE_UNK_15 = 15,
 GAME_MODE_RANKING = 16,
 GAME_MODE_BOOT = 18,
 GAME_MODE_SUNSOFT_LOGO = 19,
 GAME_MODE_CREDITS = 20,
 GAME_MODE_OPENING_CUTSCENE = 21
};

extern volatile s32 stateCooldown;

typedef u8 uint8_t;


#endif