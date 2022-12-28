//JP addresses are ~ +0x3990  of US addresses

// Vars //
.definelabel p1Health, 0x80168E73

.definelabel jlCrowns, 0x80200B6A
.definelabel alCrowns, 0x80200B6B
.definelabel blCrowns, 0x80200B6C
.definelabel dcCrowns, 0x80200B6D
.definelabel klCrowns, 0x80200B6E
.definelabel gcCrowns, 0x80200B6F

.definelabel blackWhiteUnlock, 0x80200C01

// Funcs //

.definelabel gameMode, 0x800FFEB4
.definelabel subGameMode, 0x800FFEB8
.definelabel dma_copy, 0x80085C20
.definelabel printText, 0x80080430

.definelabel ct_memcpy, 0x800EBF10
.definelabel strlen, 0x800EBF3C
.definelabel strchr, 0x800EBF64

.definelabel currLevel, 0x80174877
.definelabel loadBoss, 0x800C0CFC

.definelabel p1ButtonsHeld, 0x80175680
.definelabel p1ButtonsPressed, 0x80175684
.definelabel freeCamActive, 0x8016AA98
.definelabel P1Instance, 0x80174858
.definelabel TongueInstance, 0x8017485C
.definelabel _bzero, 0x800E0420
.definelabel rngSeed, 0x800F6884

.definelabel p1, 0x80168DA8
.definelabel tongue, 0x80169268

.definelabel osGetCount, 0x800E0250 

.definelabel osInvalICache, 0x800DCA90
.definelabel osInvalDCache, 0x800DCB10
.definelabel __osSpDeviceBusy, 0x800E6800
.definelabel __osSiDeviceBusy, 0x800EBBE0

.definelabel stateCooldown, 0x8047FFF8
//.definelabel stateFinishedBool, 0x8047FFFC
.definelabel gCrashScreen, 0x80400100
.definelabel gCustomThread, 0x80400200
.definelabel osMemSize, 0x80000318
.definelabel memcpy, 0x800EBF10
//    .definelabel _Printf
.definelabel osWritebackDCacheAll, 0x800DC550
.definelabel osViBlack, 0x800DA040
.definelabel osViSwapBuffer, 0x800DB060
.definelabel __osActiveQueue, 0x80109E0C
.definelabel osSetEventMesg, 0x800DB230
.definelabel osRecvMesg, 0x800DA620
.definelabel osStopThread, 0x800EBC10
.definelabel osCreateMesgQueue, 0x800DA4E0
.definelabel osCreateThread, 0x800D99D0
.definelabel osStartThread, 0x800D9B20
.definelabel osGetTime, 0x800E01C0
.definelabel __osCurrentTime, 0x8024C260
.definelabel osContStartReadData, 0x800DB820
.definelabel osContGetReadData, 0x800DB8E4
.definelabel ct_strchr, 0x800EBF64
.definelabel ct_strlen, 0x800EBF3C
.definelabel __osDisableInt, 0x800E2BE0
.definelabel __osRestoreInt, 0x800E2C00
.definelabel debugBool, 0x800F06B0
.definelabel currentFileLevelUnlocks, 0x80200B66
.definelabel currentFileLevelUnlocks2, 0x80200B68
.definelabel isPaused, 0x801749B4
.definelabel isTakingLoadingZone, 0x80176F58
.definelabel playSound, 0x80087ED0

.definelabel textBuffer, 0x807F0000
.definelabel textBuffer2, 0x807F0200

.definelabel heldButtonsMain, 0x80175650
.definelabel pauseFrameCountMode, 0x807FFFCC
.definelabel pauseFrameCount, 0x807FFFD0
.definelabel stateModeDisplay, 0x807FFFD4
.definelabel saveOrLoadStateMode, 0x807FFFD8
.definelabel savestateCurrentSlot, 0x807FFFDC
.definelabel previouslyHeldButtons, 0x807FFFE0
.definelabel previouslyPressedButtons, 0x807FFFE4
.definelabel currentlyHeldButtons, 0x807FFFE8
.definelabel currentlyPressedButtons, 0x807FFFEC
.definelabel savestate1size, 0x807FFFF0
.definelabel savestate2size, 0x807FFFF4
.definelabel savestate3size, 0x807FFFF8

.definelabel p1ControllerOS, 0x800F6888

.definelabel compressBuffer, 0x80500000
.definelabel decompressBuffer, 0x800EE1C0

.definelabel drawTimer, 0x80089BA8



//Payload related information
.definelabel PAYLOAD_START_ROM, 0xC00000
.definelabel PAYLOAD_START_RAM, 0x80410000
//.definelabel CT_MOD_ROM, PAYLOAD_START_ROM + 0x1000
//.definelabel CT_MOD_RAM, PAYLOAD_START_RAM + 0x1000
.definelabel PAYLOAD_SIZE, 0x0006E000

PAYLOAD_START:
.headersize 0x80024C00 //ran once on boot
.org 0x80090694
LUI a0, hi(PAYLOAD_START_ROM)
ADDIU a0, a0, lo(PAYLOAD_START_ROM)
LUI a1, hi(PAYLOAD_START_RAM)
ADDIU a1, a1, lo(PAYLOAD_START_RAM)
LUI a2, hi(PAYLOAD_SIZE)
JAL dma_copy
ADDIU a2, a2, lo(PAYLOAD_SIZE)
J originalCode
NOP

.org 0x800A1090
NOP

//.org 8002D660
//ADDIU a0, r0, 0x0 // No enemies or objects appear

.org 0x80089BB4 //in game time hook
NOP

.org 0x8009071C //per frame hook (at beginning of gamemode step loop)
JAL mainCFunctionWrapper
NOP


PAYLOAD_END: