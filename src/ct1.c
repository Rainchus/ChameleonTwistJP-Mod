#include "../include/ct1.h"

extern void* crash_screen_copy_to_buf(void* dest, const char* src, u32 size);

#define SAVE_MODE 0
#define LOAD_MODE 1

s32 controllerData = 0x80175650;
u8 menuIsActive = 0;

volatile s32 isSaveOrLoadActive = 0;

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

s32 printTextBool = 1;

int __osPiDeviceBusy() {
    register u32 stat = IO_READ(PI_STATUS_REG);
    if (stat & (PI_STATUS_DMA_BUSY | PI_STATUS_IO_BUSY))
        return 1;
    return 0;
}

void _sprintf(void* destination, void* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _Printf((void*)crash_screen_copy_to_buf, destination, fmt, args);
    va_end(args);
}

void convertAsciiToText(void* buffer, char* source) {
    u16* buf = (u16*)buffer;
    s32 strlength = ct_strlen(source);
    s32 i;

    for (i = 0; i < strlength; i++) {
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
    buf[i] = 0; //terminate buffer
}

void convertAsciiToText2(void* buffer, char* source) {
    u16* buf = (u16*)buffer;
    s32 strlength = ct_strlen(source);
    s32 i;

    for (i = 0; i < strlength; i++) {
        if ( (source[i] >= '0' && source[i] <= '9') ||
            (source[i] >= 'A' && source[i] <= 'Z')) { //is 0 - 9 or A - Z
            buf[i] = source[i] + 0xA380; //0x30 = 0 in ascii, 0xA3B0 = 0 in chameleon text
        } else if ( (source[i] > '0' && source[i] <= '9') ||
            (source[i] >= 'a' && source[i] <= 'z')) { //is 0 - 9 or A - Z
            buf[i] = source[i] + 0xA360; //0x30 = 0 in ascii, 0xA3B0 = 0 in chameleon text
        } else if(source[i] == '-') {
            buf[i] = 0xA1DD; // '-' in chameleon text
        } else if (source[i] == '.') {
            buf[i] = 0xA1C7; // ' ' ' in chameleon text (apostrophe)
        } else if (source[i] == ':') {
            buf[i] = 0xA1A7; // ':' in chameleon text
        } else if (source[i] == ' ') {
            buf[i] = 0xA1A1; // ' ' in chameleon text
        }
    }
    buf[i] = 0; //terminate buffer
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

void print_fps(void) {
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
    isSaveOrLoadActive = 0;
    savestate1Size = 0;
    savestate2Size = 0;
    savestate3Size = 0;
	return 1;
}

void pauseUntilDMANotBusy(void) {
	volatile s32* dmaBusyAddr = (s32*)0xA4600010;

	if (*dmaBusyAddr & 3) {
		while (*dmaBusyAddr & 3) {}
	}
}

#define ramAddrSavestateDataSlot1 (void*)0x804C0000
#define ramAddrSavestateDataSlot2 (void*)0x805D0000
#define ramAddrSavestateDataSlot3 (void*)0x806E0000 //hopefully doesn't overflow into 0x807FFFDC (though if it does we were screwed anyway)
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
            if (savestate1Size != 0 && savestate1Size != -1) {
                decompress_lz4_ct_default(ramEndAddr - ramStartAddr, savestate1Size, ramAddrSavestateDataSlot1); //always decompresses to `ramStartAddr`
            }  
        break;
        case DPAD_UP_CASE:
            if (savestate2Size != 0 && savestate2Size != -1) {
                decompress_lz4_ct_default(ramEndAddr - ramStartAddr, savestate2Size, ramAddrSavestateDataSlot2); //always decompresses to `ramStartAddr`
            }
        break;
        case DPAD_RIGHT_CASE:
            if (savestate3Size != 0 && savestate3Size != -1) {
                decompress_lz4_ct_default(ramEndAddr - ramStartAddr, savestate3Size, ramAddrSavestateDataSlot3); //always decompresses to `ramStartAddr`
            }  
        break;
    }
    setStatusRegister(status);
    __osRestoreInt();
    isSaveOrLoadActive = 0; //allow thread 3 to continue
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
    isSaveOrLoadActive = 0; //allow thread 3 to continue
}

void updateCustomInputTracking(void) {
    s32 temp;
    temp = previouslyHeldButtons ^ heldButtonsMain; //if holding this frame and last frame, button wasn't pressed
    temp ^= previouslyPressedButtons;
    temp &= heldButtonsMain;
    currentlyPressedButtons = temp;

    previouslyHeldButtons = heldButtonsMain;
}

extern s32 textGreenMatColor[];
extern s32 textCyanColor[];
extern s32 textWhiteColor[];

void KLTogglePrinting(void) {
    char messageBuffer[20];
    char convertedMessageBuffer[sizeof(messageBuffer) * 2];

    colorTextWrapper(textGreenMatColor);
    _bzero(messageBuffer, sizeof(messageBuffer)); //clear buffer
    _sprintf(messageBuffer, "SPD: %.2f", p1.forwardVel);
    _bzero(convertedMessageBuffer, sizeof(convertedMessageBuffer)); //clear buffer
    convertAsciiToText2(&convertedMessageBuffer, (char*)&messageBuffer);
    textPrint(13.0f, 170.0f, 0.5f, &convertedMessageBuffer, 1);

    colorTextWrapper(textCyanColor);
    _bzero(messageBuffer, sizeof(messageBuffer)); //clear buffer
    _sprintf(messageBuffer, "ANG: %.2f", p1.yAngle);
    _bzero(convertedMessageBuffer, sizeof(convertedMessageBuffer)); //clear buffer
    convertAsciiToText2(&convertedMessageBuffer, (char*)&messageBuffer);
    textPrint(13.0f, 182.0f, 0.5f, &convertedMessageBuffer, 1);

    colorTextWrapper(textWhiteColor);
    _bzero(messageBuffer, sizeof(messageBuffer)); //clear buffer
    _sprintf(messageBuffer, "X: %.2f", p1.xPos);
    _bzero(convertedMessageBuffer, sizeof(convertedMessageBuffer)); //clear buffer
    convertAsciiToText2(&convertedMessageBuffer, (char*)&messageBuffer);
    textPrint(13.0f, 194.0f, 0.5f, &convertedMessageBuffer, 1);

    colorTextWrapper(textWhiteColor);
    _bzero(messageBuffer, sizeof(messageBuffer)); //clear buffer
    _sprintf(messageBuffer, "Z: %.2f", p1.zPos);
    _bzero(convertedMessageBuffer, sizeof(convertedMessageBuffer)); //clear buffer
    convertAsciiToText2(&convertedMessageBuffer, (char*)&messageBuffer);
    textPrint(13.0f, 206.0f, 0.5f, &convertedMessageBuffer, 1);
}

void checkInputsForSavestates(void) {
    savestateCurrentSlot = -1;//set to invalid

    if (currentlyPressedButtons & DPAD_LEFT) {
        savestateCurrentSlot = 0;
    }

    if (currentlyPressedButtons & DPAD_UP) {
        savestateCurrentSlot = 1;
    }

    // if (currentlyPressedButtons & DPAD_RIGHT) {
    //     savestateCurrentSlot = 2;
    // }
    if (currentlyPressedButtons & DPAD_RIGHT) {
        //KL specific code
        KLTogglePrintBool ^= 1;
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
            isSaveOrLoadActive = 1;
            osCreateThread(&gCustomThread.thread, 255, (void*)savestateMain, NULL,
                    gCustomThread.stack + sizeof(gCustomThread.stack), 255);
            osStartThread(&gCustomThread.thread);
            stateCooldown = 5;
        } else {
            isSaveOrLoadActive = 1;
            osCreateThread(&gCustomThread.thread, 255, (void*)loadstateMain, NULL,
                    gCustomThread.stack + sizeof(gCustomThread.stack), 255);
            osStartThread(&gCustomThread.thread);
            stateCooldown = 5;            
        }
    }
}

u8 timesvaulted = 0;

s32 spinRoomTwoDoorClipPractice(void) {
    if ((gameMode == GAME_MODE_OVERWORLD) && (currLevel == 4) && (currRoom == 11)) {
        f32 xPos = 20.0f;
        f32 yPos = 35.0f;
        f32 scale = 0.5f;
        s32 style = 3;

        _sprintf(textBuffer, "VAUL: %02d\n", timesvaulted);
		_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
		convertAsciiToText(&textBuffer2, (char*)&textBuffer);
		printText(xPos, yPos, 0, scale, 0, 0, &textBuffer2, style);
    }
    return 1;
}

// Only Runs in the cave skip room
// TODO: Pause data after second vault; Add Speed Data

s32 caveSkipPractice(void) {
    if ((gameMode == GAME_MODE_OVERWORLD) && (currLevel == 0) && (currRoom == 0) && (toggles[TOGGLE_CAVE_SKIP_PRACTICE] == 1)) {
        // Printout Location
        f32 xPos = 20.0f;
        f32 yPos = 35.0f;
        f32 scale = 0.5f;
        s32 style = 3;

        // Data Needed
        if (tongue.vaultTime == 1) {
            timesvaulted++;
        }

        if (timesvaulted > 0) {
            f32 caveAngleDiff = (p1.yAngle - 90.0f);
            f32 caveAngleDiffAbs = ((caveAngleDiff < 0) ? -caveAngleDiff : caveAngleDiff);
            if (caveAngleDiffAbs < 5.0f) {
                colorTextWrapper(textGreenColor);
            }
            else {
                colorTextWrapper(textRedColor);
            }
            _bzero(&textBuffer, 50); //clear 50 bytes of buffer
            _sprintf(textBuffer, "ANGLE OFF BY: %.4f\n", caveAngleDiffAbs);
		    _bzero(&textBuffer2, 50); //clear 50 bytes of buffer
		    convertAsciiToText(&textBuffer2, (char*)&textBuffer);
		    printText(xPos, yPos, 0, scale, 0, 0, &textBuffer2, style);

            f32 caveZDiff = (2925.0f - p1.zPos);
            f32 caveZDiffAbs = ((caveZDiff < 0) ? -caveZDiff : caveZDiff);
            if ((p1.zPos < 2930.0f) && (p1.zPos > 2920.0f)) {
                colorTextWrapper(textGreenColor);
            }
            else {
                colorTextWrapper(textRedColor);
            }
            _sprintf(textBuffer, "Z OFF BY: %.4f\n", caveZDiffAbs);
			_bzero(&textBuffer2, 100); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, (yPos += 10.0f), 0, scale, 0, 0, &textBuffer2, style);

            //_sprintf(textBuffer, "Backwards vault so you hit the door", p1.forwardVel);
			//_bzero(&textBuffer2, 100); //clear 50 bytes of buffer
			//convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			//printText(xPos, 155.0f, 0, scale, 0, 0, &textBuffer2, style);

            //_sprintf(textBuffer, "Run keeping the angle at around 90", p1.forwardVel);
			//_bzero(&textBuffer2, 100); //clear 50 bytes of buffer
			//convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			//printText(xPos, 175.0f, 0, scale, 0, 0, &textBuffer2, style);

            //_sprintf(textBuffer, "Use a second vault on green z coord", p1.forwardVel);
			//_bzero(&textBuffer2, 100); //clear 50 bytes of buffer
			//convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			//printText(xPos, 195.0f, 0, scale, 0, 0, &textBuffer2, style);
        }

        //_sprintf(textBuffer, "VAUL: %02d\n", timesvaulted);
		//_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
		//convertAsciiToText(&textBuffer2, (char*)&textBuffer);
		//printText(xPos, yPos, 0, scale, 0, 0, &textBuffer2, style);
    }
    return 1;
}

s32 printCustomDebugText(void) {
    f32 xPos = 20.0f;
    f32 yPos = 35.0f;
    f32 scale = 0.5f;
    s32 style = 3;

	if (printTextBool == 1) {
		if (P1Instance != NULL) {
			yPos += 10.0f;
			_sprintf(textBuffer, "XPos: %.4f\n", p1.xPos);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, 0, scale, 0, 0, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "YPos: %.4f\n", p1.yPos);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, 0, scale, 0, 0, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "ZPos: %.4f\n", p1.zPos);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, 0, scale, 0, 0, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "ANGL: %.4f\n", p1.yAngle);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, 0, scale, 0, 0, &textBuffer2, style);

			yPos += 10.0f;

			_sprintf(textBuffer, "VAUL: %02d\n", tongue.vaultTime);
			_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
			convertAsciiToText(&textBuffer2, (char*)&textBuffer);
			printText(xPos, yPos, 0, scale, 0, 0, &textBuffer2, style);

			yPos += 10.0f;
		}
        
        _sprintf(textBuffer, "RNG: %08X\n", rngSeed);
		_bzero(&textBuffer2, 50); //clear 50 bytes of buffer
		convertAsciiToText(&textBuffer2, (char*)&textBuffer);
		printText(xPos, yPos, 0, scale, 0, 0, &textBuffer2, style);
	}
    return 0;
}

extern s32		osContStartReadData(OSMesgQueue *);
extern void osContGetReadData(OSContPad *);

s32 teleportToStageBoss(void) {
    // Teleports Player to Current World's Boss Stage
    if ((gameMode == GAME_MODE_OVERWORLD) && (currLevel < 0x06)) {
        loadBoss();
    }
    return 1;
}

// Not Useful Now we have Game State Flag
s32 givePlayerMaxCrowns(void) {
    // Gives Player Max Crowns
    if (gameMode == GAME_MODE_OVERWORLD) {
        jlCrowns = 25;
        alCrowns = 25;
        blCrowns = 21;
        klCrowns = 23;
        dcCrowns = 24;
        gcCrowns = 23;
    }
    return 1;
}

void printPausePractice(void) {
    f32 xPos = 20.0f;
    f32 yPos = 35.0f;
    s32 arga2 = 0.0f;
    f32 scale = 0.5f;
    f32 arga4 = 0.0f;
    f32 arga5 = 0.0f;
    s32 style = 3;

    if (isTakingLoadingZone == TRUE) {
        if (pauseFrameCountMode != 1) {
            //count frames
            pauseFrameCount++;
            if (pauseFrameCount == 8) { //is pause frame
                playSound(0x2A, (void*)0x80168DA8, 0);
                return;
            } else if (pauseFrameCount > 8 && pauseFrameCount < 30) {
                _sprintf(textBuffer, "Pause\n");
                _bzero(&textBuffer2, 50); //clear 50 bytes of buffer
                convertAsciiToText(&textBuffer2, (char*)&textBuffer);
                printText(xPos, yPos, arga2, scale, arga4, arga5, &textBuffer2, style);
            }
        }
    } else {
        //on the 11th frame when entering a boss fight, isTakingLoadingZone is FALSE for a single frame
        if (pauseFrameCount != 11) {
            pauseFrameCount = 0;
        } else {
            pauseFrameCount++;
        }
    }
}

void pageMainDisplay(s32, s32);
extern s32 isMenuActive;

void mainCFunction(void) {
    if (debugBool == -1) {
        debugBool = 0;
    }

    // Max out player 1 health
    if (toggles[TOGGLE_INFINITE_HEALTH] == 1) {p1Health = 0x0A;}
    updateCustomInputTracking();
    blackWhiteUnlock = 0x0C;
	currentFileLevelUnlocks = 0xFF; //unlock all levels
    currentFileLevelUnlocks2 = 0xFF; //unlock all levels

    caveSkipPractice();
    //spinRoomTwoDoorClipPractice();

    // Some Button
    if (isMenuActive == 1) {
        pageMainDisplay(currPageNo, currOptionNo);
        updateMenuInput();
    }
    else {
        //printPausePractice();

        if (stateCooldown == 0) {
            if ((heldButtonsMain & R_BUTTON) && (currentlyPressedButtons & DPAD_UP)) {
                debugBool ^= 1;
            } else if ((heldButtonsMain & R_BUTTON) && (currentlyPressedButtons & DPAD_DOWN)) {
                isMenuActive ^= 1;
            } else if (currentlyPressedButtons & DPAD_DOWN) {
                saveOrLoadStateMode ^= 1;
            } else {
                checkInputsForSavestates();
            }
        }

        if (stateCooldown > 0) {
            stateCooldown--;
        }

        if (toggles[TOGGLE_HIDE_SAVESTATE_TEXT] == 1) {
            if (saveOrLoadStateMode == SAVE_MODE) {
                textBuffer2[0] =  0xA3;
                textBuffer2[1] = 0x60 + 's';
            } else {
                textBuffer2[0] = 0xA3;
                textBuffer2[1] = 0x60 + 'l';
            }
            textBuffer2[2] = 0;
            textPrint(13.0f, 218.0f, 0.65f, &textBuffer2, 3);
        }

        if (toggles[TOGGLE_HIDE_IGT] == 1) {
            if (gameMode == GAME_MODE_OVERWORLD){
                if (isPaused == 0) {
                    drawTimer();
                }
            }
        }

        if (toggles[TOGGLE_CUSTOM_DEBUG_TEXT] == 1) {
            colorTextWrapper(textWhiteColor);
            printCustomDebugText();
        }
        
    }

    if (KLTogglePrintBool == 1) {
        KLTogglePrinting();
    }

    //if a savestate is being saved/loaded, stall thread
    while (isSaveOrLoadActive != 0) {}

}