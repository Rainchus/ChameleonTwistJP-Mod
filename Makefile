PRINT := printf '
 ENDCOLOR := \033[0m
 WHITE     := \033[0m
 ENDWHITE  := $(ENDCOLOR)
 GREEN     := \033[0;32m
 ENDGREEN  := $(ENDCOLOR)
 BLUE      := \033[0;34m
 ENDBLUE   := $(ENDCOLOR)
 YELLOW    := \033[0;33m
 ENDYELLOW := $(ENDCOLOR)
 PURPLE    := \033[0;35m
 ENDPURPLE := $(ENDCOLOR)
ENDLINE := \n'

OPTFLAGS := -O2

all:
	@mkdir -p obj

	@$(PRINT)$(GREEN)Compiling C file: $(ENDGREEN)$(BLUE)crash.c$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@mips64-elf-gcc -Wall $(OPTFLAGS) -mtune=vr4300 -march=vr4300 -mabi=32 -fomit-frame-pointer -mno-abicalls -fno-pic -G0 -c src/crash.c
	@mv crash.o obj/

	@$(PRINT)$(GREEN)Compiling C file: $(ENDGREEN)$(BLUE)print.c$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@mips64-elf-gcc -Wall $(OPTFLAGS) -mtune=vr4300 -march=vr4300 -mabi=32 -fomit-frame-pointer -mno-abicalls -fno-pic -G0 -c src/print.c
	@mv print.o obj/

	@$(PRINT)$(GREEN)Compiling C file: $(ENDGREEN)$(BLUE)ct1.c$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@mips64-elf-gcc -Wall $(OPTFLAGS) -mtune=vr4300 -march=vr4300 -mabi=32 -fomit-frame-pointer -mno-abicalls -fno-pic -G0 -c src/ct1.c
	@mv ct1.o obj/

	@$(PRINT)$(GREEN)Compiling C file: $(ENDGREEN)$(BLUE)lz4.c$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@mips64-elf-gcc -Wall -Os -mtune=vr4300 -march=vr4300 -mabi=32 -fomit-frame-pointer -mno-abicalls -fno-pic -G0 -c src/lz4.c
	@mv lz4.o obj/

	@$(PRINT)$(GREEN)Compiling C file: $(ENDGREEN)$(BLUE)lib.c$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@mips64-elf-gcc -Wall $(OPTFLAGS) -mtune=vr4300 -march=vr4300 -mabi=32 -fomit-frame-pointer -mno-abicalls -fno-pic -G0 -c src/lib.c
	@mv lib.o obj/

	@$(PRINT)$(GREEN)armips $(ENDGREEN)$(BLUE)asm/main.asm$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@armips asm/main.asm

	@$(PRINT)$(GREEN)n64crc $(ENDGREEN)$(BLUE)"rom/ct1JP.mod.z64"$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@n64crc "rom/ct1JP.mod.z64"

clean:
	@rm -rf obj