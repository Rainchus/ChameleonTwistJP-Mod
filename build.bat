mkdir obj
mips64-elf-gcc -Wall -O1 -mtune=vr4300 -march=vr4300 -mabi=32 -fomit-frame-pointer -G0 -c src/ct1.c
move ct1.o obj/
mips64-elf-gcc -Wall -O1 -mtune=vr4300 -march=vr4300 -mabi=32 -fomit-frame-pointer -G0 -c src/crash.c
move crash.o obj/
armips asm/main.asm
n64crc "rom/ct1JP.mod.z64"