#include "../include/ct1.h"

extern void* crash_screen_copy_to_buf(void* dest, const char* src, u32 size);

#define DPAD_UP 0x0800
#define DPAD_DOWN 0x0400
#define DPAD_LEFT 0x0200
#define DPAD_RIGHT 0x0100
#define L_BUTTON 0x0020

extern char textBuffer[0x200];
extern char textBuffer2[0x200];

typedef struct CustomThread {
    /* 0x000 */ OSThread thread;
    /* 0x1B0 */ char stack[0x4100];
    /* 0x9B0 */ OSMesgQueue queue;
    /* 0x9C8 */ OSMesg mesg;
    /* 0x9CC */ u16* frameBuf;
    /* 0x9D0 */ u16 width;
    /* 0x9D2 */ u16 height;
} CustomThread; // size = 0x9D4


//char buffer1[(0x80025C00 - 0x80000000)];
//char buffer2[(0x80300000 - 0x800EE1C0)];

void _sprintf(void* destination, void* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _Printf(crash_screen_copy_to_buf, &textBuffer, fmt, args);
    va_end(args);
}

void convertAsciiToText(void* buffer, char* source) {
    u16* buf = (u16*)buffer;
    s32 strlength = ct_strlen(source);

    for (s32 i = 0; i < strlength; i++) {
        if ( (source[i] >= '0' && source[i] <= '9') ||
            (source[i] >= 'A' && source[i] <= 'Z')) { //is 0 - 9 or A - Z
            buf[i] = source[i] + 0xA380; //0x30 = 0 in ascii, 0xA3B0 = 0 in chameleon text
        } else if ( (source[i] > '0' && source[i] <= '9') ||
            (source[i] >= 'a' && source[i] <= 'z')) { //is 0 - 9 or A - Z
            buf[i] = source[i] + 0xA360; //0x30 = 0 in ascii, 0xA3B0 = 0 in chameleon text
        } else if(source[i] == '-') {
            buf[i] = 0xA1DD; // '-' in chameleon text
        } else if (source[i] == '.') {
            buf[i] = 0xA1A5; // '.' in chameleon text
        } else if (source[i] == ':') {
            buf[i] = 0xA1A7; // ':' in chameleon text
        } else if (source[i] == ' ') {
            buf[i] = 0xA1A1; // ' ' in chameleon text
        }
    }
}

// ------------- FPS COUNTER ---------------
// To use it, call print_fps(x,y); every frame.
#define FRAMETIME_COUNT 30

OSTime frameTimes[FRAMETIME_COUNT];
u8 curFrameTimeIndex = 0;

// Call once per frame
f32 calculate_and_update_fps(void) {
    u32 newTime = osGetCount();
    u32 oldTime = frameTimes[curFrameTimeIndex];
    frameTimes[curFrameTimeIndex] = newTime;

    curFrameTimeIndex++;
    if (curFrameTimeIndex >= FRAMETIME_COUNT) {
        curFrameTimeIndex = 0;
    }

    return (1000000.0f * FRAMETIME_COUNT * (OS_CPU_COUNTER/15625.0f) / (1000000.0f/15625.0f)) / (f32)(newTime - oldTime);
}

void print_fps(s32 x, s32 y) {
    f32 xPos = 20.0f;
    f32 yPos = 220.0f;
    s32 arga2 = 0.0f;
    f32 scale = 0.5f;
    f32 arga4 = 0.0f;
    f32 arga5 = 0.0f;
    s32 style = 3;

    f32 fps = calculate_and_update_fps();
    char text[10];

	_bzero(&textBuffer, sizeof(textBuffer)); //clear buffer
	_sprintf(textBuffer, "FPS: %2.2f", fps);
	_bzero(&textBuffer2, sizeof(textBuffer2)); //clear buffer
	convertAsciiToText(&textBuffer2, (char*)&textBuffer);
	printText(xPos, yPos, arga2, scale, arga4, arga5, &textBuffer2, style);
}

int cBootMain(void) {
	crash_screen_init();
    set_gp();
    stateFinishedBool = 0;
    stateCooldown = 0;
	return 1;
}

s32 printTextBool = 0;

//800DCA90 osInvalICache
//800DCB10 osInvalDCache

void pauseUntilDMANotBusy(void) {
	volatile s32* dmaBusyAddr = (s32*)0xA4600010;

	if (*dmaBusyAddr & 3) {
		while (*dmaBusyAddr & 3) {}
	}
}

// s32 __osAiDeviceBusy(void) {
//     register s32 status = IO_READ(AI_STATUS_REG);

//     if (status & AI_STATUS_FIFO_FULL) {
//         return TRUE;
//     }

//     return FALSE;
// }

/*
*************************************************************************
 * Peripheral Interface (PI) Registers
*/
#define PI_BASE_REG		0x04600000

/* PI DRAM address (R/W): [23:0] starting RDRAM address */
#define PI_DRAM_ADDR_REG	(PI_BASE_REG+0x00)	/* DRAM address */

/* PI pbus (cartridge) address (R/W): [31:0] starting AD16 address */
#define PI_CART_ADDR_REG	(PI_BASE_REG+0x04)

/* PI read length (R/W): [23:0] read data length */
#define PI_RD_LEN_REG		(PI_BASE_REG+0x08)

/* PI write length (R/W): [23:0] write data length */
#define PI_WR_LEN_REG		(PI_BASE_REG+0x0C)

/*
 * PI status (R): [0] DMA busy, [1] IO busy, [2], error
 *           (W): [0] reset controller (and abort current op), [1] clear intr
 */
#define PI_STATUS_REG		(PI_BASE_REG+0x10)

#define PI_STATUS_DMA_BUSY  (1 << 0)
#define PI_STATUS_IO_BUSY   (1 << 1)
#define PI_STATUS_ERROR     (1 << 2)

#define WAIT_ON_IOBUSY(stat)                                                                \
    while (stat = IO_READ(PI_STATUS_REG), stat & (PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY))  \
        ;                                                                                   \
    (void)0

int __osPiDeviceBusy() {
    register u32 stat = IO_READ(PI_STATUS_REG);
    if (stat & (PI_STATUS_DMA_BUSY | PI_STATUS_IO_BUSY))
        return 1;
    return 0;
}

//80500000 (try 80480000 first)

//skip 80000000 - 800EE1C0
//800EE1C0 - 803B5000

#define ramStartAddr 0x800EE1C0
#define ramEndAddr 0x803B5000
//#define ramEndAddr 0x80400000

//GPR regs 80400000 - 80400080
//FPR regs 80400080 - 80400100

// void copyGPRRegs(void) {
//     s32* GPRRegsAddr = (s32*)0x80400000;
//     u32 baseInstruction = 0xACA00000; //SW r0, 0x0000 (a1)
//     for (int i = 0; i < 32; i++) {
//         executeASM( baseInstruction | (i << 16), &GPRRegsAddr[i]);
//     }
// }

// void copyFPRRegs(void) {
//     s32* FPRRegsAddr = (s32*)0x80400000;
//     u32 baseInstruction = 0xE4A00000; //SWC1 f0, 0x0000 (a1)
//     for (int i = 0; i < 32; i++) {
//         executeASM( baseInstruction | (i << 16), &FPRRegsAddr[i]);
//     }
// }

void loadstateMain(void) {
    stateFinishedBool = 0;
	pauseUntilDMANotBusy();
    
    //wait on rsp
    while (__osSpDeviceBusy() == 1) {}

    //wait on rdp
    while ( __osDpDeviceBusy() == 1) {}

    //wait on SI
    while (__osSiDeviceBusy() == 1) {}

    //wait on PI
    while (__osPiDeviceBusy() == 1) {}

    //invalidate caches
    osInvalICache((void*)0x80000000, 0x2000);
	osInvalDCache((void*)0x80000000, 0x2000);
    __osDisableInt();
    customMemCpy(ramStartAddr, 0x80480000, ramEndAddr - ramStartAddr);
    __osRestoreInt();
    //loadRegsManual();
    stateFinishedBool = 1;
}
    
void savestateMain(void) {
    stateFinishedBool = 0;
    //copyRegsManual();
	pauseUntilDMANotBusy();
    
    //wait on rsp
    while (__osSpDeviceBusy() == 1) {}

    //wait on rdp
    while ( __osDpDeviceBusy() == 1) {}

    //wait on SI
    while (__osSiDeviceBusy() == 1) {}

    //wait on PI
    while (__osPiDeviceBusy() == 1) {}

    //invalidate caches
    osInvalICache((void*)0x80000000, 0x2000);
	osInvalDCache((void*)0x80000000, 0x2000);

    __osDisableInt();
    customMemCpy(0x80480000, ramStartAddr, ramEndAddr - ramStartAddr);
    //LZ4_compress_fast(ramStartAddr, 0x80480000, ramEndAddr - ramStartAddr, 1 << 20, 1);
    __osRestoreInt();
    stateFinishedBool = 1;
}

extern CustomThread gCustomThread;

void mainCFunction(void) {
    f32 xPos = 20.0f;
    f32 yPos = 35.0f;
    s32 arga2 = 0.0f;
    f32 scale = 0.5f;
    f32 arga4 = 0.0f;
    f32 arga5 = 0.0f;
    s32 style = 3;
    volatile s32 threadBool = 0;
	
	//debugBool = 1;
	currentFileLevelUnlocks = 0x13; //unlock all levels

	if (p1ButtonsPressed == DPAD_LEFT) {
        if (stateCooldown == 0) {
            osCreateThread(&gCustomThread.thread, 255, savestateMain, NULL,
                    gCustomThread.stack + sizeof(gCustomThread.stack), 255);
            osStartThread(&gCustomThread.thread);
            stateCooldown = 5;
        }
	}

	if (p1ButtonsPressed == DPAD_RIGHT) {
        if (stateCooldown == 0) {
            osCreateThread(&gCustomThread.thread, 255, loadstateMain, NULL,
                    gCustomThread.stack + sizeof(gCustomThread.stack), 255);
            osStartThread(&gCustomThread.thread);
            stateCooldown = 5;
        }
	}

    if ((p1ButtonsHeld & L_BUTTON) && (p1ButtonsPressed & DPAD_UP)) {
        if (debugBool == 1) {
            debugBool = 0;
        } else if (debugBool == 0) {
            debugBool = 1;
        } else {
            //if 0xFFFFFFFF
            debugBool = 0;
        }
    }

	if (printTextBool == 1) {
		if (P1Instance != NULL) {
			_sprintf(textBuffer, "XPos: %.4f\n", p1.xPos);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, arga2, scale, arga4, arga5, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "YPos: %.4f\n", p1.yPos);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, arga2, scale, arga4, arga5, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "ZPos: %.4f\n", p1.zPos);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, arga2, scale, arga4, arga5, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "ANGL: %.4f\n", p1.yAngle);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, arga2, scale, arga4, arga5, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "VAUL: %02d\n", tongue.vaultTime);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, arga2, scale, arga4, arga5, &textBuffer2, style);

			yPos += 10.0f;
		}
	}

    // yPos = 220.0f;

    // _sprintf(textBuffer, "ANGL: %.4f\n", p1.yAngle);
    // _bzero(&textBuffer2, 50); //clear 50 bytes of buffer
    // convertAsciiToText(&textBuffer2, (char*)&textBuffer);
    // printText(xPos, yPos, arga2, scale, arga4, arga5, &textBuffer2, style);

    if (stateCooldown > 0) {
        stateCooldown--;
    }

	//print_fps(0, 0);
}