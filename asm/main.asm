.n64 // Let armips know we're coding for the N64 architecture
.open "rom/ct1JP.z64", "rom/ct1JP.mod.z64", 0 // Open the ROM file
.include "asm/ct1JP.asm" // Include ct1.asm to tell armips' linker where to find the game's function(s)

.headersize 0x7F810000
.org 0x80420000 //ct1.asm max size of 0x1000, otherwise will overwrite ct1.o
.include "asm/boot.asm"
.importobj "obj/crash.o"
.importobj "obj/print.o"
.importobj "obj/ct1.o"
.importobj "obj/lz4.o"
.importobj "obj/lib.o"
.importobj "obj/menu.o"
.close // Close the ROM file