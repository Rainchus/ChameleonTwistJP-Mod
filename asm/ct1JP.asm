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

.definelabel textBuffer, 0x807F0000
.definelabel textBuffer2, 0x807F0200

.definelabel heldButtonsMain, 0x80175650
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
.definelabel PAYLOAD_START_ROM, 0xB315D0
.definelabel PAYLOAD_START_RAM, 0x80410000
.definelabel CT_MOD_ROM, PAYLOAD_START_ROM + 0x1000
.definelabel CT_MOD_RAM, PAYLOAD_START_RAM + 0x1000
.definelabel PAYLOAD_SIZE, 0x0006E000 //end of rom - 0xB92000 to get this size

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

.headersize (PAYLOAD_START_RAM - PAYLOAD_START_ROM)
.org 0x80410000 //ct1.asm max size of 0x1000, otherwise will overwrite ct1.o
originalCode:
JAL cBootMain
NOP
LUI v0, 0x800F
LW v0, 0x06E8 (v0)
ADDIU at, r0, 0xFFFF
LUI a0, 0x8012
BEQ v0, at, label0_main
ADDIU a0, a0, 0x92E8
LUI at, 0x8010
SW v0, 0xFEB4 (at)
label0_main:
LUI at, 0x8010
J 0x800906B8
SW r0, 0xFEB8 (at)

mainCFunctionWrapper:
JAL mainCFunction
NOP
LUI a1, 0x8010
J 0x80090724
LW a1, 0xFEB4 (a1)

customMemCpy: //requires 0x08 alignment
BLEZ a2, exitMemCpy
LD t0, 0x0000 (a1)
SD t0, 0x0000 (a0)
ADDIU a1, a1, 8
ADDIU a0, a0, 8
BEQ r0, r0, customMemCpy
ADDIU a2, a2, -8
exitMemCpy:
JR RA
NOP

set_gp:
LUI gp, 0x8040
JR RA
ORI gp, gp, 0x0000


executeASM:
//a0 holds instruction
//a1 address to store register value to
ADDIU sp, sp, -0x20
SW ra, 0x0018 (sp)
JAL getPC
NOP
getPC:
SW a0, 0x0010 (ra)
NOP //buffer just in case
NOP //buffer just in case
NOP //buffer just in case
NOP //(custom instruction goes here)
LW ra, 0x0018 (sp)
JR RA
ADDIU sp, sp, 0x20


copyRAM:
JR RA
NOP

osViRepeatLine:
ADDIU          SP, SP, -0x18
SW             S0, 0x0010 (SP)
SW             RA, 0x0014 (SP)
JAL            0x800E2BE0 //__osDisableInt
ADDU           S0, A0, R0
ANDI           S0, S0, 0x00FF
BEQZ           S0, label1
ADDU           A0, V0, R0
LUI            V1, 0x8011
LW             V1, 0x9E94 (V1)
LHU            V0, 0x0000 (V1)
J              label0
ORI            V0, V0, 0x0040
label1:
LUI            V1, 0x8011
LW             V1, 0x9E94(V1)
LHU            V0, 0x0000 (V1)
ANDI           V0, V0, 0xFFBF
label0:
JAL            0x800E2C00 //__osRestoreInt
SH             V0, 0x0000 (V1)
LW             RA, 0x0014 (SP)
LW             S0, 0x0010 (SP)
JR             RA
ADDIU          SP, SP, 0x18

// osSetTime:
// LUI t0, 0x8025
// SW r0, 0xC260 (t0)
// JR RA
// SW r0, 0xC264 (t0)

crash_screen_sleep: //takes arg a0, ms to sleep
ADDIU sp, sp, -0x20
ORI v0, r0, 0xB71B
MULT a0, v0
DADDU a0, r0, r0
DADDU a1, r0, r0
SW ra, 0x0018 (sp)
SW s1, 0x0014 (sp)
SW s0, 0x0010 (sp)
MFHI s0
MFLO s1
JAL osSetTime
NOP
crashScreenLoop:
JAL osGetTime
NOP
DADDU a0, v0, r0
DADDU a1, v1, r0
SLTU v0, a0, s0
BNEZ v0, crashScreenLoop
NOP
BNE s0, a0, exitCrashFunc
SLTU v0, a1, s1
BNEZ v0, crashScreenLoop
NOP
exitCrashFunc:
LW ra, 0x0018 (sp)
LW s1, 0x0014 (sp)
LW s0, 0x0010 (sp)
JR RA
ADDIU sp, sp, 0x20

//so emulator wont close the game upon infinite loop
infiniteLoop:
NOP
J infiniteLoop
NOP

__osDpDeviceBusy:
LUI t6, 0xA410
LW a0, 0x000C (t6)
ADDIU sp, sp, -0x08
ANDI t7, a0, 0x0100
BEQZ t7, dpRet
NOP
B dpExit
ADDIU v0, r0, 1
dpRet:
OR v0, r0, r0
dpExit:
JR RA
ADDIU sp, sp, 0x08

asmCrashGame:
LUI t0, 0x8000
ORI t0, t0, 0x0001
LW t1, 0x0000 (t0)
JR RA
NOP

getStatusRegister:
MFC0 v0, $12
NOP
NOP
JR RA
NOP

setStatusRegister:
MTC0 a0, $12
NOP
NOP
JR RA
NOP


.include "asm/registers.asm"
.include "asm/printf.asm"

.importobj "obj/crash.o"
.importobj "obj/print.o"
.importobj "obj/ct1.o"
.importobj "obj/lz4.o"
.importobj "obj/lib.o"


PAYLOAD_END: