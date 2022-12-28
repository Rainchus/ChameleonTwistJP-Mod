#include "../include/menu.h"

enum Pages {
    PAGE_MAIN = 0,
    PAGE_JL = 1
};

#define FUNCS_PER_PAGE 8
#define FUNCS_PER_LAST_PAGE 3

typedef s32 (*menuProc) (void);
#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))

u8 toggles[] = {
    0, // TOGGLE_HIDE_TEXT
    0  // TOGGLE_INFINITE_HEALTH
};

s32 toggleHideText(void) {
    toggles[TOGGLE_HIDE_TEXT] ^= 1;
    stateModeDisplay ^= 1;
    return 1;
}

s32 toggleInfiniteHealth(void) {
    toggles[TOGGLE_INFINITE_HEALTH] ^= 1;
    return 1;
}

typedef struct menuPage {
    /* 0x08 */ s32 optionCount;
    /* 0x0C */ s32 pageIndex;
    /* 0x10 */ char* options[FUNCS_PER_PAGE];
    /* 0x30 */ s32 (*menuProc[FUNCS_PER_PAGE]) (void);
} menuPage;

menuPage page1 = {
    .optionCount = 2,
    .pageIndex = PAGE_JL,
    .options = {
        "Toggle Hide Text\n",
        "Infinite Health\n"
    },
    .menuProc = {
        &toggleHideText,
        &toggleInfiniteHealth
    }
};

menuPage page0 = {
    .optionCount = 3,
    .pageIndex = PAGE_MAIN,
    .options = {
        "Load Boss\n",
        "Page0 Option1\n",
        "Page0 Option2\n",
    },
    .menuProc = {
        &teleportToStageBoss,
        &teleportToStageBoss,
        &teleportToStageBoss
    }
};

menuPage* pageList[] = {
    &page0,
    &page1
};

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
    0x0A, 0xFF, 0x00, 0xFF, // top
    0xFF, 0xFF, 0xFF, 0xFF, // bottom
    0x0A, 0xFF, 0x00, 0xFF, // top
    0xFF, 0xFF, 0x00, 0x0F  // bottom
};

s32 currPageNo = 1;
s32 currOptionNo = 0;

char menuOptionBuffer[50] = { 0 };  // Buffer for menu options text
char menuOptionBufferConverted[100] = { 0 };  // Buffer for menu options text converted to chameleon twist format

s32 isMenuActive = 0;

void updateMenuInput(void){
    if (currentlyPressedButtons & DPAD_UP) {
        if (currOptionNo > 0) {
            currOptionNo--;
        }
    }
    else if (currentlyPressedButtons & DPAD_DOWN) {
        if (currOptionNo < pageList[currPageNo]->optionCount - 1) {
            currOptionNo++;
        }
    }
    else if (currentlyPressedButtons & DPAD_LEFT) {
        if (currPageNo > 0) {
            currPageNo--;
            currOptionNo = 0;
        }
    }
    else if (currentlyPressedButtons & DPAD_RIGHT) {
        if (currPageNo < ARRAY_COUNT(pageList) - 1) {
            currPageNo++;
            currOptionNo = 0;
        }
    }
    else if (currentlyPressedButtons & B_BUTTON) {
        isMenuActive = 0;
    }
    else if (currentlyPressedButtons & A_BUTTON) {
        pageList[currPageNo]->menuProc[currOptionNo]();
    }
}

void pageMainDisplay(s32 currPageNo, s32 currOptionNo) {
    menuPage* currPage = pageList[currPageNo];
    f32 xPos = 20.0f;
    f32 yPos = 35.0f;
    
    for (int i = 0; i < currPage->optionCount; i++) {
        _bzero(&menuOptionBuffer, sizeof(menuOptionBuffer)); //clear buffer
        _sprintf(menuOptionBuffer, "%s", currPage->options[i]);
        _bzero(&menuOptionBufferConverted, sizeof(menuOptionBufferConverted)); //clear buffer 2
        convertAsciiToText(&menuOptionBufferConverted, (char*)&menuOptionBuffer);

        if (i == currOptionNo) {
            colorTextWrapper(colorTest);
        }

        textPrint(xPos, (yPos + (i * 15.0f)), 0.5f, &menuOptionBufferConverted, 1);
    }
}

// Boolean Menu Options (Toggles)



