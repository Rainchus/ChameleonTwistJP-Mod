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

customVar:
.word 0x00000000

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