#include "../include/ct1.h"

extern void* crash_screen_copy_to_buf(void* dest, const char* src, u32 size);

#define DPAD_UP 0x08000000
#define DPAD_DOWN 0x04000000
#define DPAD_LEFT 0x02000000
#define DPAD_RIGHT 0x01000000
#define L_BUTTON 0x00200000

#define SAVE_MODE 0
#define LOAD_MODE 1

s32 controllerData = 0x80175650;

typedef struct CustomThread {
    /* 0x000 */ OSThread thread;
    /* 0x1B0 */ char stack[0xC000];
    /* 0x9B0 */ OSMesgQueue queue;
    /* 0x9C8 */ OSMesg mesg;
    /* 0x9CC */ u16* frameBuf;
    /* 0x9D0 */ u16 width;
    /* 0x9D2 */ u16 height;
} CustomThread; // size = 0x9D4

extern CustomThread gCustomThread;
extern char textBuffer[0x200];
extern char textBuffer2[0x200];

s32 printTextBool = 0;

int __osPiDeviceBusy() {
    register u32 stat = IO_READ(PI_STATUS_REG);
    if (stat & (PI_STATUS_DMA_BUSY | PI_STATUS_IO_BUSY))
        return 1;
    return 0;
}

void _sprintf(void* destination, void* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _Printf((void*)crash_screen_copy_to_buf, (void*)&textBuffer, fmt, args);
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
    f32 scale = 0.5f;
    s32 style = 3;

    f32 fps = calculate_and_update_fps();

	_bzero(&textBuffer, sizeof(textBuffer)); //clear buffer
	_sprintf(textBuffer, "FPS: %2.2f", fps);
	_bzero(&textBuffer2, sizeof(textBuffer2)); //clear buffer
	convertAsciiToText(&textBuffer2, (char*)&textBuffer);
    textPrint(xPos, yPos, scale, &textBuffer2, style);
}

int cBootMain(void) {
	crash_screen_init();
    set_gp();
    saveOrLoadStateMode = SAVE_MODE;
    savestateCurrentSlot = 0;
    stateModeDisplay = 1;
    debugBool = 0;
    stateCooldown = 0;
	return 1;
}

void pauseUntilDMANotBusy(void) {
	volatile s32* dmaBusyAddr = (s32*)0xA4600010;

	if (*dmaBusyAddr & 3) {
		while (*dmaBusyAddr & 3) {}
	}
}

#define ramAddrSavestateDataSlot1 (void*)0x80500000
#define ramAddrSavestateDataSlot2 (void*)0x80600000
#define ramAddrSavestateDataSlot3 (void*)0x80700000 //hopefully doesn't overflow into 0x807FFFDC (though if it does we were screwed anyway)
#define DPAD_LEFT_CASE 0
#define DPAD_UP_CASE 1
#define DPAD_RIGHT_CASE 2

void loadstateMain(void) {
    s32 register status = getStatusRegister();
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
    //decompress_lz4_ct_default(ramEndAddr - ramStartAddr, savestate1Size, ramAddrSavestateDataSlot1); //always decompresses to `ramStartAddr`
    switch (savestateCurrentSlot) {
        case DPAD_LEFT_CASE:
            if (savestate1Size != 0) {
                decompress_lz4_ct_default(ramEndAddr - ramStartAddr, savestate1Size, ramAddrSavestateDataSlot1); //always decompresses to `ramStartAddr`
            }  
        break;
        case DPAD_UP_CASE:
            if (savestate2Size != 0) {
                decompress_lz4_ct_default(ramEndAddr - ramStartAddr, savestate2Size, ramAddrSavestateDataSlot2); //always decompresses to `ramStartAddr`
            }
        break;
        case DPAD_RIGHT_CASE:
            if (savestate3Size != 0) {
                decompress_lz4_ct_default(ramEndAddr - ramStartAddr, savestate3Size, ramAddrSavestateDataSlot3); //always decompresses to `ramStartAddr`
            }  
        break;
    }
    setStatusRegister(status);
    __osRestoreInt();
    //force crash for testing
    //asmCrashGame();
}
    
void savestateMain(void) {
    //push status
    s32 register status = getStatusRegister();
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
    switch (savestateCurrentSlot) {
        case DPAD_LEFT_CASE:
            savestate1Size = compress_lz4_ct_default(ramStartAddr, ramEndAddr - ramStartAddr, ramAddrSavestateDataSlot1);
        break;
        case DPAD_UP_CASE:
            savestate2Size = compress_lz4_ct_default(ramStartAddr, ramEndAddr - ramStartAddr, ramAddrSavestateDataSlot2);
        break;
        case DPAD_RIGHT_CASE:
            savestate3Size = compress_lz4_ct_default(ramStartAddr, ramEndAddr - ramStartAddr, ramAddrSavestateDataSlot3);
        break;
    }
    setStatusRegister(status);
    __osRestoreInt();
}

void updateCustomInputTracking(void) {
    s32 temp;
    temp = previouslyHeldButtons ^ heldButtonsMain; //if holding this frame and last frame, button wasn't pressed
    temp ^= previouslyPressedButtons;
    temp &= heldButtonsMain;
    currentlyPressedButtons = temp;

    previouslyHeldButtons = heldButtonsMain;
}

void checkInputsForSavestates(void) {
    savestateCurrentSlot = -1;//set to invalid

    if (currentlyPressedButtons & DPAD_LEFT) {
        savestateCurrentSlot = 0;
    }

    if (currentlyPressedButtons & DPAD_UP) {
        savestateCurrentSlot = 1;
    }

    if (currentlyPressedButtons & DPAD_RIGHT) {
        savestateCurrentSlot = 2;
    }

    if (savestateCurrentSlot == -1 || stateCooldown != 0){
        return;
    }

    if (gameMode != GAME_MODE_STAGE_SELECT &&
    gameMode != GAME_MODE_NEW_GAME_MENU &&
    gameMode != GAME_MODE_TITLE_SCREEN &&
    gameMode != GAME_MODE_JUNGLE_LAND_MENU &&
    isPaused == 0) {
        if (saveOrLoadStateMode == SAVE_MODE) {
            osCreateThread(&gCustomThread.thread, 255, (void*)savestateMain, NULL,
                    gCustomThread.stack + sizeof(gCustomThread.stack), 255);
            osStartThread(&gCustomThread.thread);
            stateCooldown = 5;
        } else {
            osCreateThread(&gCustomThread.thread, 255, (void*)loadstateMain, NULL,
                    gCustomThread.stack + sizeof(gCustomThread.stack), 255);
            osStartThread(&gCustomThread.thread);
            stateCooldown = 5;            
        }
    }
}

void printCustomDebugText(void) {
    f32 xPos = 20.0f;
    f32 yPos = 35.0f;
    f32 scale = 0.5f;
    s32 style = 3;

	if (printTextBool == 1) {
		if (P1Instance != NULL) {
			_sprintf(textBuffer, "XPos: %.4f\n", p1.xPos);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
            textPrint(xPos, yPos, scale, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "YPos: %.4f\n", p1.yPos);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			textPrint(xPos, yPos, scale, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "ZPos: %.4f\n", p1.zPos);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			textPrint(xPos, yPos, scale, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "ANGL: %.4f\n", p1.yAngle);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			textPrint(xPos, yPos, scale, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "VAUL: %02d\n", tongue.vaultTime);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			textPrint(xPos, yPos, scale, &textBuffer2, style);

			yPos += 10.0f;
		}
	}
}

extern s32		osContStartReadData(OSMesgQueue *);
extern void osContGetReadData(OSContPad *);

// s32 readInputsWrapper(void) {
//     OSContPad unkPtr;
//     osContStartReadData(&unkPtr);
//     osRecvMesg(&unkPtr, 0, 1);
//     osContGetReadData(&unkPtr);
//     osRecvMesg(&unkPtr, 0, 1);
//     OSContPad* p1Pad = (u8*)0x80175650;
//     p1Pad->button = unkPtr.button;
//     p1Pad->stick_x = unkPtr.stick_x;
//     p1Pad->stick_y = unkPtr.stick_y;
//     return &unkPtr;
// }

void mainCFunction(void) {

    //readInputsWrapper();
    updateCustomInputTracking();

	//debugBool = 1;
	currentFileLevelUnlocks = 0x13; //unlock all levels

    if (stateCooldown == 0) {
        if ((heldButtonsMain & L_BUTTON) && (currentlyPressedButtons & DPAD_UP)) {
            if (debugBool == 1) {
                debugBool = 0;
            } else if (debugBool == 0) {
                debugBool = 1;
            } else {
                //if -1
                debugBool = 0;
            }
        } else if ((heldButtonsMain & L_BUTTON) && (currentlyPressedButtons & DPAD_DOWN)) {
            stateModeDisplay ^= 1;
        } else if (currentlyPressedButtons & DPAD_DOWN) {
            saveOrLoadStateMode ^= 1;
        } else {
            checkInputsForSavestates();
        }
    }

    if (stateCooldown > 0) {
        stateCooldown--;
    }

    if (stateModeDisplay == 1) {
        if (saveOrLoadStateMode == SAVE_MODE) {
            textBuffer2[0] =  0xA3;
            textBuffer2[1] = 0x60 + 's';
        } else {
            textBuffer2[0] = 0xA3;
            textBuffer2[1] = 0x60 + 'l';
        }
        textBuffer2[2] = 0;

        textPrint(13.0f, 208.0f, 0.65f, &textBuffer2, 3);
        if (gameMode == GAME_MODE_OVERWORLD){
        if (isPaused == 0) {
            drawTimer();
        }
    }
    }
}