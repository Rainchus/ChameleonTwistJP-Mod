//JP addresses are ~ +0x3990  of US addresses

.definelabel gameMode, 0x800FFEB4
.definelabel subGameMode, 0x800FFEB8
.definelabel dma_copy, 0x80085C20



//Payload related information
.definelabel PAYLOAD_START_ROM, 0xB315D0
.definelabel PAYLOAD_START_RAM, 0x80400000
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

.org 0x8009071C //per frame hook (at beginning of gamemode step loop)
JAL mainCFunctionWrapper
NOP

.headersize (PAYLOAD_START_RAM - PAYLOAD_START_ROM)
.org 0x80400000 //ct1.asm max size of 0x1000, otherwise will overwrite ct1.o
originalCode:
JAL cBootMain
NOP
LUI v0, 0x800F
LW v0, 0x06E8 (v0)
ADDIU at, r0, 0xFFFF
LUI a0, 0x8012
BEQ v0, at, label0
ADDIU a0, a0, 0x92E8
LUI at, 0x8010
SW v0, 0xFEB4 (at)
label0:
LUI at, 0x8010
J 0x800906B8
SW r0, 0xFEB8 (at)

mainCFunctionWrapper:
JAL mainCFunction
NOP
LUI a1, 0x8010
J 0x80090724
LW a1, 0xFEB4 (a1)

.importobj "obj/ct1.o"

PAYLOAD_END: