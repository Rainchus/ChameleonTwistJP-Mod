#include "../include/menu.h"

enum Pages {
    PAGE_MAIN = 0,
    PAGE_JL = 1
};

#define FUNCS_PER_PAGE 8
#define FUNCS_PER_LAST_PAGE 3

typedef struct menuPage {
    /* 0x08 */ s32 optionCount;
    /* 0x0C */ s32 pageIndex;
    /* 0x10 */ char* options[FUNCS_PER_PAGE];
    /* 0x30 */ void* functions[FUNCS_PER_PAGE];
} menuPage;

menuPage page1 = {
    .optionCount = 2,
    .pageIndex = PAGE_JL,
    .options = {
        "p10\n",
        "p11\n"
    }
};

menuPage page0 = {
    .optionCount = 3,
    .pageIndex = PAGE_MAIN,
    .options = {
        "Load Boss\n",
        "Page0 Option1\n",
        "Page0 Option2\n",
    }
};

menuPage* pageList[] = {
    &page0,
    &page1
};

#define PAGE_TOTAL sizeof(pageList) / sizeof(pageList[0]);

enum MenuOptions {
    MENU_OPTION_LOAD_BOSS = 0,
    MENU_OPTION_TEST = 1,
    MENU_OPTION_TEST2 = 2,
    MENU_OPTION_TEST3 = 3,
    MENU_OPTION_TEST4 = 4,
    MENU_OPTION_TEST5 = 5,
    MENU_OPTION_TEST6 = 6,
    MENU_OPTION_TEST7 = 7,
    MENU_OPTION_TEST8 = 8
};

//testing func ptr
s32 printCustomDebugText(void);

void colorTextWrapper(s32* color) {
    colorText(color[0], color[1], color[2], color[3], color[4], color[5], color[6], color[7],
             color[8], color[9], color[10], color[11], color[12], color[13], color[14], color[15]);
}

s32 colorTest[] = {
    0xFF, 0xFF, 0xFF, 0xFF, // top
    0x00, 0x00, 0x00, 0x00, // bottom
    0xFF, 0xFF, 0xFF, 0xFF, // top
    0x00, 0x00, 0x00, 0x00  // bottom
};

typedef s32 (*menuProc) (void);
s32 currPageNo = 1;
s32 currOptionNo = 0;

char menuOptionBuffer[50] = { 0 };  // Buffer for menu options text
char menuOptionBufferConverted[100] = { 0 };  // Buffer for menu options text converted to chameleon twist format

//void updateMenuInput(void){
//    if (currentlyPressedButtons & BUTTON_DPAD_UP) {
//        if (currOptionNo > 0) {
//            currOptionNo--;
//        }
//    }
//    else if (currentlyPressedButtons & BUTTON_DPAD_DOWN) {
//        if (currOptionNo < pageList[currPageNo]->optionCount) {
//            currOptionNo++;
//        }
//    }
//    else if (currentlyPressedButtons & BUTTON_DPAD_LEFT) {
//        if (currPageNo > 0) {
//            currPageNo--;
//        }
//    }
//    else if (currentlyPressedButtons & BUTTON_DPAD_RIGHT) {
//        if (currPageNo < PAGE_TOTAL) {
//            currPageNo++;
//        }
//    }
//    else if (currentlyPressedButtons & BUTTON_B) {
//        menuIsActive = 0;
//    }
//    else if (currentlyPressedButtons & BUTTON_A) {
//        menuIsActive = 0;
//    }
//}

void pageMainDisplay(s32 currPageNo, s32 currOptionNo) {
    f32 xPos = 20.0f;
    f32 yPos = 10.0f;
    menuPage* currPage = pageList[currPageNo];

    for (int i = 0; i < currPage->optionCount; i++) {
        _bzero(&menuOptionBuffer, sizeof(menuOptionBuffer)); //clear buffer
        _sprintf(menuOptionBuffer, "%s", currPage->options[i]);
        _bzero(&menuOptionBufferConverted, sizeof(menuOptionBufferConverted)); //clear buffer 2
        convertAsciiToText(&menuOptionBufferConverted, (char*)&menuOptionBuffer);
        colorTextWrapper(colorTest);
        textPrint(xPos, (yPos + (i * 30.0f)), 0.5f, &menuOptionBufferConverted, 1);
    }
}

// s32 menuDisplay(void) {
//     switch (currPageNo) {
//         case PAGE_NONE:
//             break;
//         case PAGE_MAIN:
//             //pageMainDisplay();
//             break;
//     }
//     return 1;
// }

// s32 menuProcFunc(void) {
//     switch (currPageNo) {
//         case PAGE_NONE:
//             break;
//         case PAGE_MAIN:

//             break;
//     }
//     return 1;
// }

void testFuncPointer(void) {
    // menuProc temp = &printCustomDebugText;
    // (*temp)();
    //pageMainDisplay(0, 0);
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


