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


# List of source files
SOURCES = $(wildcard src/*.c)

# List of object files, generated from the source files
OBJECTS = $(SOURCES:src/%.c=obj/%.o)

CC := mips64-elf-gcc
STANDARDFLAGS := -O2 -Wall -mtune=vr4300 -march=vr4300 -mabi=32 -fomit-frame-pointer -mno-abicalls -fno-pic -G0
SPEEDFLAGS := -Os -Wall -mtune=vr4300 -march=vr4300 -mabi=32 -fomit-frame-pointer -mno-abicalls -fno-pic -G0

# Default target
all: $(OBJECTS) assemble

# Rule for building object files from source files
obj/%.o: src/%.c | obj
	@$(PRINT)$(GREEN)Compiling C file: $(ENDGREEN)$(BLUE)$<$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@$(CC) $(STANDARDFLAGS) -c $< -o $@

assemble: $(OBJECTS)
	@$(PRINT)$(GREEN)Assembling with armips: $(ENDGREEN)$(BLUE)asm/main.asm$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@armips asm/main.asm
	@$(PRINT)$(GREEN)n64crc $(ENDGREEN)$(BLUE)"rom/ct1JP.mod.z64"$(ENDBLUE)$(ENDCOLOR)$(ENDLINE)
	@n64crc "rom/ct1JP.mod.z64"

# Rule for creating the obj folder
obj:
	@mkdir -p obj

# Rule for cleaning up the project
clean:
	@rm -f $(OBJECTS)
