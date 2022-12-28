#ifndef _MENU_H
#define _MENU_H

#include "../include/ct1.h"
#include "../include/print.h"

extern void _sprintf(void* destination, void* fmt, ...);
extern void convertAsciiToText(void* buffer, char* source);

void pageMainDisplay(s32 currPageNo, s32 currOptionNo);
s32 menuDisplay(void);
s32 menuProcFunc(void);

#endif