#include "../include/ct1.h"
#include "../include/print.h"

enum Pages {
    PAGE_NONE = 0,
    PAGE_MAIN = 1
};

#define FUNCS_PER_PAGE 8


//testing func ptr
s32 printCustomDebugText(void);

typedef s32 (*menuProc) (void);
s32 pageNumber = 1;

u8 menuOptionBuffer[50] = {0};  // Buffer for menu options text

void pageMainDisplay(void) {
    menuOptionBuffer[0] = 0;
}

s32 menuDisplay(void) {
    switch (pageNumber) {
        case PAGE_NONE:
            break;
        case PAGE_MAIN:
            pageMainDisplay();
            break;
    }
    return 1;
}

s32 menuProcFunc(void) {
    switch (pageNumber) {
        case PAGE_NONE:
            break;
        case PAGE_MAIN:

            break;
    }
    return 1;
}

void testFuncPointer(void) {
    menuProc temp = &printCustomDebugText;
    (*temp)();
    pageMainDisplay();
}

// typedef struct menuOptionProc {
//     /* 0x00 */ s32 
// } menuOptionProc;

// Boolean Menu Options (Toggles)
#define TOGGLE_COUNT 1  // Needs to be updated when new toggle is added
u8 toggles[TOGGLE_COUNT];

enum Toggles {
    TOGGLE_HIDE_TEXT = 0
};


