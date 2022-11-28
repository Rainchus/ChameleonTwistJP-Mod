#include "types.h"

typedef struct TonguePos{
    /* 0x00 */ f32 pos1;
    /* 0x04 */ f32 pos2;
    /* 0x08 */ f32 pos3;
    /* 0x0C */ f32 pos4;
    /* 0x10 */ f32 pos5;
    /* 0x14 */ f32 pos6;
    /* 0x18 */ f32 pos7;
    /* 0x1C */ f32 pos8;
    /* 0x20 */ f32 pos9;
    /* 0x24 */ f32 pos10;
    /* 0x28 */ f32 pos11;
    /* 0x2C */ f32 pos12;
    /* 0x30 */ f32 pos13;
    /* 0x34 */ f32 pos14;
    /* 0x38 */ f32 pos15;
    /* 0x3C */ f32 pos16;
    /* 0x40 */ f32 pos17;
    /* 0x44 */ f32 pos18;
    /* 0x48 */ f32 pos19;
    /* 0x4C */ f32 pos20;
    /* 0x50 */ f32 pos21;
    /* 0x54 */ f32 pos22;
    /* 0x58 */ f32 pos23;
    /* 0x5C */ f32 pos24;
    /* 0x60 */ f32 pos25;
    /* 0x64 */ f32 pos26;
    /* 0x68 */ f32 pos27;
    /* 0x6C */ f32 pos28;
    /* 0x70 */ f32 pos29;
    /* 0x74 */ f32 pos30;
    /* 0x78 */ f32 pos31;
    /* 0x7C */ f32 pos32;
} TonguePos;

typedef struct tongueSlot {
    /* 0x00 */ u32 slot1;
    /* 0x04 */ u32 slot2;
    /* 0x08 */ u32 slot3;
    /* 0x0C */ u32 slot4;
    /* 0x10 */ u32 slot5;
    /* 0x14 */ u32 slot6;
    /* 0x18 */ u32 slot7;
    /* 0x1C */ u32 slot8;
    /* 0x20 */ u32 slot9;
    /* 0x24 */ u32 slot10;
    /* 0x28 */ u32 slot11;
    /* 0x2C */ u32 slot12;
    /* 0x30 */ u32 slot13;
    /* 0x34 */ u32 slot14;
    /* 0x38 */ u32 slot15;
    /* 0x3C */ u32 slot16;
    /* 0x40 */ u32 slot17;
    /* 0x44 */ u32 slot18;
    /* 0x48 */ u32 slot19;
    /* 0x4C */ u32 slot20;
    /* 0x50 */ u32 slot21;
    /* 0x54 */ u32 slot22;
    /* 0x58 */ u32 slot23;
    /* 0x5C */ u32 slot24;
    /* 0x60 */ u32 slot25;
    /* 0x64 */ u32 slot26;
    /* 0x68 */ u32 slot27;
    /* 0x6C */ u32 slot28;
    /* 0x70 */ u32 slot29;
    /* 0x74 */ u32 slot30;
    /* 0x78 */ u32 slot31;
    /* 0x7C */ u32 slot32;
    /* 0x80 */ u32 slot33;
    /* 0x84 */ u32 slot34;
    /* 0x88 */ u32 slot35;
    /* 0x8C */ u32 slot36;
    /* 0x90 */ u32 slot37;
    /* 0x94 */ u32 slot38;
    /* 0x98 */ u32 slot39;
    /* 0x9C */ u32 slot40;
    /* 0xA0*/ u32 slot41;
    /* 0xA8*/ u32 slot42;
    /* 0xAC*/ u32 slot43;
    /* 0xB0*/ u32 slot44;
    /* 0xB4*/ u32 slot45;
    /* 0xB8*/ u32 slot46;
    /* 0xBC*/ u32 slot47;
    /* 0xC0*/ u32 slot48;
    /* 0xC4*/ u32 slot49;
    /* 0xC8*/ u32 slot50;
    /* 0xCC*/ u32 slot51;
    /* 0xD0*/ u32 slot52;
    /* 0xD4*/ u32 slot53;
    /* 0xD8*/ u32 slot54;
    /* 0xDC*/ u32 slot55;
    /* 0xE0*/ u32 slot56;
    /* 0xE4*/ u32 slot57;
    /* 0xE8*/ u32 slot58;
    /* 0xEC*/ u32 slot59;
    /* 0xF0*/ u32 slot60;
    /* 0xF4*/ u32 slot61;
    /* 0xF8*/ u32 slot62;
    /* 0xFC*/ u32 slot63;
    /* 0x100*/ u32 slot64;
} tongueSlot;

typedef struct Tongue { // at 80169268 (for p1 at least lol)
    /* 0x00 */ u32 vaulting; //0x00 = no, 0x01 = yes
    /* 0x04 */ u32 tongueMode; //enum of tongue actions
    /* 0x08 */ s32 segments;
    /* 0x0C */ s32 poleSegmentAt;
    /* 0x10 */ u32 timer;
    /* 0x14 */ s32 cameraSegmentAt;//where to point the camera
    /* 0x18 */ TonguePos tongueXs;//all (including the rope model) positions
    // + 80
    /* 0x98 */ f32 xpos33;//the last piece is special to the positions and angle
    /* 0x9C */ TonguePos tongueYs;
    // + 80
    /* 0x11C*/ f32 ypos33;
    /* 0x120*/ TonguePos tongueZs;
    // + 80
    /* 0x1A0*/ f32 zpos33;
    /* 0x1A4*/ TonguePos tongueAngles;
    // + 80
    /* 0x224*/ f32 anglepos33;
    /* 0x228*/ TonguePos tongueForwards;//????? why
    // + 80
    /* 0x2A8*/ TonguePos tongueHalfX;// idk what this or the other one is doing but they are axiis so
    // + 80
    /* 0x328*/ TonguePos tongueHalfZ;
    // + 80
    /* 0x3A8*/ f32 controlAngle;
    /* 0x3AC*/ f32 length;
    /* 0x3B0*/ f32 trueAngle;
    /* 0x3B4*/ tongueSlot onTongue;
    // + 100
    /* 0x4B4*/ u32 amountTnTongue;
    /* 0x4B8*/ tongueSlot inMouth;
    // + 100
    /* 0x5B8*/ u32 amountInMouth;
    //all of this has to do with vaulting
    /* 0x5BC*/ f32 reset1;
    /* 0x5C0*/ f32 reset2;
    /* 0x5C4*/ f32 reset3;
    /* 0x5C8*/ f32 reset4;
    /* 0x5CC*/ f32 reset5;
    /* 0x5D0*/ f32 reset6;
    /* 0x5D4*/ f32 reset7;
    /* 0x5D8*/ f32 reset8;
    /* 0x5DC*/ f32 reset9;
    /* 0x5E0*/ f32 reset10;//used, but always set to 0 at the peak of vault
    /* 0x5E4*/ f32 angleOfVault;
    /* 0x5E8*/ f32 powerOfVault;
    /* 0x5EC*/ f32 xOffset;//offsets for when you release
    /* 0x5F0*/ f32 yOffset;
    /* 0x5F4*/ f32 zOffset;
    /* 0x5F8*/ u32 vaultTime;
    //all of this has to do with poles
    /* 0x5FC*/ f32 lastTongueOffset;//offset for matching the twisted model to the last segment
    /* 0x600*/ u32 poleID;//please tell me im right
    /* 0x604*/ u32 tongueDir;//0x01 for counter clockwise. 0xFFFFFFFF for clockwise.
    //wall
    /* 0x608*/ u32 wallTime;//timer for tongue-touching a wall
} Tongue;

typedef struct playerActor {
    /* 0x00 */ u32 playerID; //trust me thats what it is
    /* 0x04 */ f32 xPos;
    /* 0x08 */ f32 yPos;
    /* 0x0C */ f32 zPos;
    /* 0x10 */ f32 xPosAgain;
    /* 0x14 */ f32 yPosAgain;
    /* 0x18 */ f32 zPosAgain;
    /* 0x1C */ f32 yCounter; //counts around where the y is but not at. weird.
    /* 0x20 */ f32 waterGIMP; //how much to gimp you in water? idk but its correlated
    /* 0x24 */ f32 xVel;
    /* 0x28 */ f32 yVel;
    /* 0x2C */ f32 zVel;
    /* 0x30 */ f32 xVaultlocity;
    /* 0x34 */ f32 yVaultlocity;
    /* 0x38 */ f32 zVaultlocity;
    /* 0x3C */ f32 yAngle;
    /* 0x40 */ f32 forwardVel; //between 0 and 20.8. gets broken on slope jumps
    /* 0x44 */ f32 forwardImpulse;
    /* 0x48 */ f32 waterFactor; //gets effected in water again
    /* 0x4C */ f32 hitboxSize; //30 default
    /* 0x50 */ f32 hitboxYStretch; //unconfirmed. 150 default.
    //there is no reason to make these s32's
    /* 0x54 */ u32 canJump;    //0x00 = yes, 0x01 = no
    /* 0x58 */ u32 jumpReleasedInAir;    // 0x00 = no, 0x01 = yes
    /* 0x5C */ u32 jumpAnimFrame;
    /* 0x60 */ u32 hasTumbled;    //0x00 = no, 0x01 = yes. resets on jump.
    /* 0x64 */ u32 unk64;
    /* 0x68 */ u32 inWater;//0x00 = no, 0x01 = yes.
    /* 0x6C */ u32 squishTimer;
    /* 0x70 */ f32 yScale;
    /* 0x74 */ u32 locked; //0x00 = no, 0x16 = yes. when using lock to stand in place.
    /* 0x78 */ s32 amountToShoot; //number for machine gun shoot
    /* 0x7C */ s32 surface; //-1 when off ground, diff number when on diff surface. if 0 you slow to a crawl
    /* 0x80 */ s32 wSurface; //-1 when not in water, diff number when in diff water
    /* 0x84 */ s32 surfaceSlide; //1 if you slide on a slope while standing. 0 if else (not walkable slopes)
    /* 0x88 */ s32 surfaceFine; //more accurate
    /* 0x8C */ s32 vaulting; // 0 if not, 1 if
    /* 0x90 */ f32 xFromCenter; //from center of room (when on ground)
    /* 0x94 */ f32 yFromCenter;
    /* 0x98 */ f32 zFromCenter;
    /* 0xA8 */ f32 xShift; //override(?) when on moving objects (falling bridges, etc)
    /* 0xAC */ f32 yShift;
    /* 0xB0 */ f32 zShift;
    /* 0xA8 */ f32 xMove; //override when sliding on slopes or on poles
    /* 0xAC */ f32 yMove;
    /* 0xB0 */ f32 zMove;
    /* 0xB4 */ u32 groundMovement; //0x00 = standing, 0x01 = walking, 0x02 = running
    /* 0xB8 */ f32 speed;
    /* 0xBC */ u32 unkBC;
    /* 0xC0 */ u32 shootLeft;
    /* 0xC4 */ u32 vaultFall;//timer for falling after vault
    /* 0xC8 */ u32 hp;
    /* 0xCC */ u32 playerHURTSTATE;
    /* 0xD0 */ u32 playerHURTTIMER;
    /* 0xD4 */ u32 playerHURTANIM;
    /* 0xD8 */ u32 playerHURTBY;
    /* 0xDC */ f32 useless1; //vec3's i have to assume
    /* 0xE0 */ f32 useless2;
    /* 0xE4 */ f32 useless3;
    /* 0xE8 */ f32 useless4;
    /* 0xEC */ f32 useless5;
    /* 0xF0 */ f32 useless6;
    /* 0xF4 */ f32 useless7;
    /* 0xF8 */ f32 useless8;
    /* 0xFC */ f32 useless9;
    /* 0x100*/ f32 useless10;
    /* 0x104*/ f32 useless11;
    /* 0x108*/ f32 useless12;
    /* 0x10C*/ f32 timerDown;
    /* 0x110*/ f32 reticleSize;
    /* 0x114*/ u32 active; //0x00 = no, 0x01 = yes
    /* 0x118*/ u32 exists; //0x00 = no, 0x01 = yes
    /* 0x11C*/ u32 power; //enum of power it has
    /* 0x120*/ u32 powerTimer; 
    /* 0x124*/ u32 powerTimerTill; 
    /* 0x128*/ f32 tongueYOffset; 
    /* 0x12C*/ f32 tongueSeperation; 
} playerActor;