/* --------------------------------------------------------------------------------------------- */
/* FarmSquad (main.c)                                                                            */
/*                                                                                               */
/* Atari 8-bit XL/XE arcade game, built using the cc65-2.19 toolchain.                           */
/*                                                                                               */
/* BSD-3-Clause License                                                                          */
/* Copyright 2024 Supurloop Software LLC                                                         */
/*                                                                                               */
/* Redistribution and use in source and binary forms, with or without modification, are          */
/* permitted provided that the following conditions are met:                                     */
/*                                                                                               */
/* 1. Redistributions of source code must retain the above copyright notice, this list of        */
/* conditions and the following disclaimer.                                                      */
/* 2. Redistributions in binary form must reproduce the above copyright notice, this list of     */
/* conditions and the following disclaimer in the documentation and/or other materials provided  */
/* with the distribution.                                                                        */
/* 3. Neither the name of the copyright holder nor the names of its contributors may be used to  */
/* endorse or promote products derived from this software without specific prior written         */
/* permission.                                                                                   */
/*                                                                                               */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   */
/* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    */
/* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL      */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE */
/* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    */
/* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  */
/* OF THE POSSIBILITY OF SUCH DAMAGE.                                                            */
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/* Build                                                                                         */
/* cl65 --target atari -C .\atari-xex.cfg -O -o Vector-Doubles.xex main.c                        */
/* To make different tune use RMT to export stripped rmt and then run, incorporate into main.c   */
/* ..\..\xasm-3.1.0-windows\xxd -i .\newmusicstripped.rmt > song.h                               */
/* Altirra - XE ATOS/800 NTSC/64K/BASIC */
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/* Includes                                                                                      */
/* --------------------------------------------------------------------------------------------- */
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atari.h>
#include <peekpoke.h>
#include <stdint.h>
#include "rmt.h"
#include "dlist.h"
#include "main.h"
#include "rand.h"
//#include "terrain.h"

/* --------------------------------------------------------------------------------------------- */
/* Macros                                                                                        */
/* --------------------------------------------------------------------------------------------- */
#define RMT_RUN (1u)
#define BGCOLOR (0xD01A)
#define CS_ADDR (0x8000)
#define CS_ADDR_HI (CS_ADDR >> 8)
#define DL_ADDR OS.sdlst //(0x0600) // 0x9C20
#define CHAR_SET_SIZE (8u)

#define PADDLE_MIN (0u)
#define PADDLE_MAX (228u)
#define PADDLE_STEER_LEFT (133u)
#define PADDLE_STEER_RIGHT (95u)
#define PADDLE_STEER_DELAY (32u)
#define PADDLE_STEER_STEP (4u)
#define PLAYER_WIDTH (8u)
#define PLAYER_MIN_HPOS (56u)
#define PLAYER_MAX_HPOS (192u)

#define PLAYER0_DFL_HPOS (PLAYER_MIN_HPOS + (PADDLE_STEER_STEP * 4))
#define PLAYER1_DFL_HPOS (PLAYER_MIN_HPOS + (PADDLE_STEER_STEP * 12))
#define PLAYER2_DFL_HPOS (PLAYER_MIN_HPOS + (PADDLE_STEER_STEP * 22))
#define PLAYER3_DFL_HPOS (PLAYER_MIN_HPOS + (PADDLE_STEER_STEP * 30))

#define PADDL0 (0x0270)
#define PADDL1 (0x0271)
#define PADDL2 (0x0272)
#define PADDL3 (0x0273)

#define PTRIG0 (0x027C)
#define PTRIG1 (0x027D)
#define PTRIG2 (0x027E)
#define PTRIG3 (0x027F)

#define PF0_COLOR 0xe6 /* Green Crops */
#define PF1_COLOR 0x08 /* Gray Rock */
#define PF2_COLOR 0x66 /* Purple Repairs */
#define PF3_COLOR 0x86 /* Blue EMPs */

/* --------------------------------------------------------------------------------------------- */
/* Waits for vblank.                                                                             */
/* --------------------------------------------------------------------------------------------- */
#define waitForVBLANK() { \
    currClockFrame = OS.rtclok[2]; \
    while (OS.rtclok[2] == currClockFrame); \
}

/* --------------------------------------------------------------------------------------------- */
/* Terrain Cache                                                                                 */
/* --------------------------------------------------------------------------------------------- */
uint8_t wsyncCount;
uint8_t blowUp;
uint8_t blown;

/* --------------------------------------------------------------------------------------------- */
/* Macros to convert single byte chars into double byte chars                                    */
/* --------------------------------------------------------------------------------------------- */
#define CLN2B(n) (((n & 0x01) | ((n & 0x02) << 1) | ((n & 0x04) << 2) | ((n & 0x08) << 3)) << 1)
#define CHN2B(n) CLN2B((n >> 4))
#define CHAR_DOUBLE(b0, b1, b2, b3, b4, b5, b6, b7) \
    { CHN2B(b0), CHN2B(b1), CHN2B(b2), CHN2B(b3), CHN2B(b4), CHN2B(b5), CHN2B(b6), CHN2B(b7) }, \
    { CLN2B(b0), CLN2B(b1), CLN2B(b2), CLN2B(b3), CLN2B(b4), CLN2B(b5), CLN2B(b6), CLN2B(b7) },

/* --------------------------------------------------------------------------------------------- */
/* Custom Character Set                                                                          */
/* --------------------------------------------------------------------------------------------- */
#pragma data-name (push, "CHARDATA")
uint8_t customCharSet[CHAR_MAX][8] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* 0 - Blank  */
    { 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11 }, /* 1 - Crop */
    { 0x00, 0x00, 0x08, 0x0A, 0x2A, 0x2A, 0xAA, 0xAA }, /* 2 - Rock */
    { 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C }, /* 3 - Repairs (Animation 0) */
    { 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00 }, /* 4 - EMP (Animation 0) */
    { 0xa6, 0xa2, 0xa6, 0xa2, 0xa6, 0xa2, 0x44, 0x44 }, /* 5 - Wall 1 */
    { 0x2a, 0x6a, 0x6a, 0x6a, 0x2a, 0x6a, 0x11, 0x11 }, /* 6 - Wall 2 */
    { 0xa9, 0xa8, 0xa9, 0xa8, 0xa9, 0xa8, 0x44, 0x11 }, /* 7 - Wall 3 */
    { 0x8a, 0x9a, 0x8a, 0x9a, 0x9a, 0x9a, 0x11, 0x44 }, /* 8 - Wall 4 */
    { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 }, /* 9 - Full Block */
    { 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50 }, /* 10 - Left Half */
    { 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05 }, /* 11 - Right Half */
    { 0x55, 0x55, 0x54, 0x54, 0x50, 0x50, 0x40, 0x40 }, /* 12 - Corner Top Left */
    { 0x40, 0x40, 0x50, 0x50, 0x54, 0x54, 0x55, 0x55 }, /* 13 - Corner Bottom Left */
    { 0x55, 0x55, 0x15, 0x15, 0x05, 0x05, 0x01, 0x01 }, /* 14 - Corner Top Right */
    { 0x01, 0x01, 0x05, 0x05, 0x15, 0x15, 0x55, 0x55 }, /* 15 - Corner Bottom Right */
    { 0x05, 0x05, 0x05, 0x05, 0x55, 0x55, 0x55, 0x55 }, /* 16 - 3/4 Top Left */
    { 0x55, 0x55, 0x55, 0x55, 0x05, 0x05, 0x05, 0x05 }, /* 17 - 3/4 Bottom Left */
    { 0x50, 0x50, 0x50, 0x50, 0x55, 0x55, 0x55, 0x55 }, /* 18 - 3/4 Top Right */
    { 0x55, 0x55, 0x55, 0x55, 0x50, 0x50, 0x50, 0x50 }, /* 19 - 3/4 Bottom Right */

#if 1 
    CHAR_DOUBLE(0, 60,  102, 110, 118, 102, 60,  0)     /* 20/21 - 0 */
    CHAR_DOUBLE(0, 24,  56,  24,  24,  24,  126, 0)     /* 22/23 - 1 */
    CHAR_DOUBLE(0, 60,  102, 12,  24,  48,  126, 0)     /* 24/25 - 2 */
    CHAR_DOUBLE(0, 126, 12,  24,  12,  102, 60,  0)     /* 26/27 - 3 */
    CHAR_DOUBLE(0, 12,  28,  60,  108, 126, 12,  0)     /* 28/29 - 4 */
    CHAR_DOUBLE(0, 126, 96,  124, 6,   102, 60,  0)     /* 30/31 - 5 */
    CHAR_DOUBLE(0, 60,  96,  124, 102, 102, 60,  0)     /* 32/33 - 6 */
    CHAR_DOUBLE(0, 126, 6,   12,  24,  48,  48,  0)     /* 34/35 - 7 */
    CHAR_DOUBLE(0, 60,  102, 60,  102, 102, 60,  0)     /* 36/37 - 8 */
    CHAR_DOUBLE(0, 60,  102, 62,  6,   12,  56,  0)     /* 38/39 - 9 */
    CHAR_DOUBLE(0, 24,  60,  102, 102, 126, 102, 0)     /* 40/41 - A */
    CHAR_DOUBLE(0, 124, 102, 124, 102, 102, 124, 0)     /* 42/43 - B */
    CHAR_DOUBLE(0, 60,  102, 96,  96,  102, 60,  0)     /* 44/45 - C */
    CHAR_DOUBLE(0, 120, 108, 102, 102, 108, 120, 0)     /* 46/47 - D */
    CHAR_DOUBLE(0, 126, 96,  124, 96,  96,  126, 0)     /* 48/49 - E */
    CHAR_DOUBLE(0, 126, 96,  124, 96,  96,  96,  0)     /* 50/51 - F */
    CHAR_DOUBLE(0, 62,  96,  96,  110, 102, 62,  0)     /* 52/53 - G */
    CHAR_DOUBLE(0, 102, 102, 126, 102, 102, 102, 0)     /* 54/55 - H */
    CHAR_DOUBLE(0, 126, 24,  24,  24,  24,  126, 0)     /* 56/57 - I */
    CHAR_DOUBLE(0, 6,   6,   6,   6,   102, 60,  0)     /* 58/59 - J */
    CHAR_DOUBLE(0, 102, 108, 120, 120, 108, 102, 0)     /* 60/61 - K */
    CHAR_DOUBLE(0, 96,  96,  96,  96,  96,  126, 0)     /* 62/63 - L */
    CHAR_DOUBLE(0, 99,  119, 127, 107, 99,  99,  0)     /* 64/65 - M */
    CHAR_DOUBLE(0, 102, 118, 126, 126, 110, 102, 0)     /* 66/67 - N */
    CHAR_DOUBLE(0, 60,  102, 102, 102, 102, 60,  0)     /* 68/69 - O */
    CHAR_DOUBLE(0, 124, 102, 102, 124, 96,  96,  0)     /* 70/71 - P */
    CHAR_DOUBLE(0, 60,  102, 102, 102, 108, 54,  0)     /* 72/73 - Q */
    CHAR_DOUBLE(0, 124, 102, 102, 124, 108, 102, 0)     /* 74/75 - R */
    CHAR_DOUBLE(0, 60,  96,  60,  6,   6,   60,  0)     /* 76/77 - S */
    CHAR_DOUBLE(0, 126, 24,  24,  24,  24,  24,  0)     /* 78/79 - T */
    CHAR_DOUBLE(0, 102, 102, 102, 102, 102, 126, 0)     /* 80/81 - U */
    CHAR_DOUBLE(0, 102, 102, 102, 102, 60,  24,  0)     /* 82/83 - V */
    CHAR_DOUBLE(0, 99,  99,  107, 127, 119, 99,  0)     /* 84/85 - W */
    CHAR_DOUBLE(0, 102, 102, 60,  60,  102, 102, 0)     /* 86/87 - X */
    CHAR_DOUBLE(0, 102, 102, 60,  24,  24,  24,  0)     /* 88/89 - Y */
    CHAR_DOUBLE(0, 126, 12,  24,  48,  96,  126, 0)     /* 90/91 - Z */
    CHAR_DOUBLE(0, 0,   0,  252, 252,  0,   0,   0)     /* 92/93 - Dash */
#endif    
};
#pragma data-name (pop)

#define WRITE_TEXT(row, off, s, sl) \
  memcpy(&rows[row][off], s, sl)

#define AZC(c) (((c - 'A') << 1) + CHAR_A), (((c - 'A') << 1) + CHAR_A + 1)
#define Z9C(c) (((c - '0') << 1) + CHAR_0), (((c - '0') << 1) + CHAR_0 + 1)

//char byJimHigginsStr[] = { AZC('B'), AZC('Y'), CHAR_BLANK, AZC('J'), AZC('I'), AZC('M'), CHAR_BLANK, AZC('H'), AZC('I'), AZC('G'), AZC('G'), AZC('I'), AZC('N'), AZC('S')};
char cropRockStr[] = { CHAR_CROP, CHAR_BLANK, AZC('C'), AZC('R'), AZC('O'), AZC('P'), CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_ROCK, CHAR_BLANK, AZC('R'), AZC('O'), AZC('C'), AZC('K') };
char repairEmpStr[] = { CHAR_REPAIR, CHAR_BLANK, AZC('R'), AZC('E'), AZC('P'), AZC('A'), AZC('I'), AZC('R'), CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_EMP | 0x80, CHAR_BLANK, AZC('E'), AZC('M'), AZC('P') };
char paddleStr[] = { AZC('P'), AZC('A'), AZC('D'), AZC('D'), AZC('L'), AZC('E'), CHAR_BLANK, CHAR_BLANK, Z9C('1'), CHAR_DASH, Z9C('4'), CHAR_BLANK, AZC('P'), AZC('L'), AZC('A'), AZC('Y'), AZC('E'), AZC('R'), AZC('S') };
char togglePlayerStr[] = { AZC('F'), AZC('I'), AZC('R'), AZC('E'), CHAR_BLANK, AZC('T'), AZC('O'), AZC('G'), AZC('G'), AZC('L'), AZC('E'), CHAR_BLANK, AZC('P'), AZC('L'), AZC('A'), AZC('Y'), AZC('E'), AZC('R') };
char fireStartStr[] = { AZC('H'), AZC('O'), AZC('L'), AZC('D'), CHAR_BLANK, AZC('F'), AZC('I'), AZC('R'), AZC('E'), CHAR_BLANK, CHAR_BLANK, AZC('T'), AZC('O'), CHAR_BLANK, AZC('S'), AZC('T'), AZC('A'), AZC('R'), AZC('T') };
char scoreStr[] = { CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, AZC('H'), AZC('I'), CHAR_BLANK, Z9C('0'), Z9C('0'), Z9C('0'), Z9C('0'), Z9C('0') };
char gameOverStr[] = { CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, CHAR_BLANK, AZC('G'), AZC('A'), AZC('M'), AZC('E'), CHAR_BLANK, CHAR_BLANK, AZC('O'), AZC('V'), AZC('E'), AZC('R') };

const char repairsCharAnimation[4][8] =
{
    { 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C },
    { 0x00, 0x3C, 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C },
    { 0x00, 0x3C, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C },
    { 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C },
};

const char empCharAnimation[4][8] =
{
    { 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x3C, 0xFF, 0xFF, 0x3C, 0x00, 0x00 },
    { 0x00, 0x3C, 0xC3, 0xC3, 0xC3, 0xC3, 0x3C, 0x00 },
    { 0x3C, 0x00, 0xC3, 0xC3, 0x00, 0xC3, 0xC3, 0x3C },
};

/* FarmSquad Title Character Graphic */
#define FARM_SQUAD_LINES (7u)
const char framsquad[FARM_SQUAD_LINES][NUM_PLAY_COLUMNS] =
{
/*    0  1  2   3   4  5   6  7  8  9  10 11 12  13  14 15  16  17  18  19  20  21  22 23  24 25  26 27  28 29 30 31  32 33 */
    { 9, 9, 9,  9,  9, 9,  9, 9, 9, 9,  9, 9, 9,  9,  9, 9,  9,  9,  9,  9,  9,  9,  9, 9,  9, 9,  9, 9,  9, 9, 9, 9,  9, 9 },
    { 9, 0, 0, 11, 10, 0, 11, 9, 0, 0, 11, 9, 0, 17, 19, 0,  9, 10,  0, 11, 10,  0, 11, 9,  0, 9, 10, 0, 11, 9, 0, 0, 11, 9 },
    { 9, 0, 9,  9,  0, 9,  0, 9, 0, 9,  0, 9, 0,  0,  0, 0,  9,  0, 11,  9,  0,  9,  0, 9,  0, 9,  0, 9,  0, 9, 0, 9,  0, 9 },
    { 9, 0, 0,  9,  0, 0,  0, 9, 0, 0, 11, 9, 0, 18, 16, 0,  9, 10,  0, 17,  0,  9,  0, 9,  0, 9,  0, 0,  0, 9, 0, 9,  0, 9 },
    { 9, 0, 9,  9,  0, 9,  0, 9, 0, 9,  0, 9, 0,  9,  9, 0,  9,  9, 10, 11, 10,  0, 11, 0, 11, 9,  0, 9,  0, 9, 0, 0, 11, 9 },
    { 9, 0, 9,  9,  9, 9,  9, 9, 9, 9,  9, 9, 9,  9,  9, 10, 0,  0,  0, 16,  9, 10, 17, 9,  9, 9,  9, 9,  9, 9, 9, 9,  9, 9 },
    { 9, 9, 9,  9,  9, 9,  9, 9, 9, 9,  9, 9, 9,  9,  9, 9,  9,  9,  9,  9,  9,  9,  9, 9,  9, 9,  9, 9,  9, 9, 9, 9,  9, 9 }
};

/* 1KB Aligned for Antic Display List */
#pragma bss-name (push, "ROWBSS")
uint8_t rows[NUM_ROWS][NUM_COLUMNS];
uint8_t notice[NUM_COLUMNS];
#pragma bss-name (pop)

/* Frequently used variables - Put in Zero Page */
#pragma data-name (push, "ZEROPAGE")
uint8_t gstate;
unsigned char ifs;
unsigned char fs;
unsigned char line;
unsigned char dline;
unsigned char cline;
uint8_t rmtplayCount;
unsigned char animation;
uint8_t paddle;
uint8_t trigger;
uint16_t score;
uint16_t hiscore;
uint8_t thposp0;
uint8_t thposp1;
uint8_t thposp2;
uint8_t thposp3;
uint8_t ufoVolume;
#pragma data-name (pop)

//#define MAIN_COLOR_START 64  // deep magenta
#define MAIN_COLOR_START 47
#define MAIN_COLOR_END 47

unsigned char mainbgcolor = MAIN_COLOR_START;
unsigned char currClockFrame = 0;
uint8_t *rp;
char *p = (char *)CS_ADDR;
char *dl;// = (char *)DL_ADDR;
unsigned char *dlp = &dlist;

uint8_t hposDrone;
uint8_t hposShadow;
uint8_t hposShadowDelta;
uint8_t hposShadowCounts;
uint8_t day = 1;

#define DEFINE_PLAYER(pn) \
uint8_t hposp##pn; \
uint8_t hposm##pn; \
uint8_t colpm##pn; \
uint8_t delayp##pn; \
uint8_t lastTrigp##pn; \
uint16_t trigCountp##pn; \
uint8_t lastDirp##pn; \
uint8_t newDirp##pn; \
uint8_t blowUp##pn;

DEFINE_PLAYER(0)
DEFINE_PLAYER(1)
DEFINE_PLAYER(2)
DEFINE_PLAYER(3)

uint8_t rev;
uint8_t volume;
uint8_t tone;
uint8_t ramp;

uint8_t droneTarget;
uint8_t targeted;
uint16_t droneTargetCount;
uint8_t vblanks;
uint8_t idleInit;
uint8_t runInit;
uint8_t dayInit;

/* Terrain making variables */
uint8_t column;
uint8_t r;
uint8_t *pterrain2;
uint8_t tline2;

#define DO_LYRICS (1u)
#if DO_LYRICS == 1
const char *ly1 = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x26" "arm" "\x33" "quad\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const char *ly2 = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x22" "est\x00of\x00the\x00" "best\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const char *ly3 = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x37ill\x00pass\x00the\x00test\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
//                 1234567890123456789012345678901234567890
const char *ly4 = "\x34hey\x00mutilate\x00thirst\x00\x0d\x00" "by\x00manly\x00" "drinks\x0e";
const char *ly5 = "\x00\x00\x00\x2duscles\x00" "bulge\x00\x0d\x00they\x00" "cannot\x00think\x0e\x00\x00\x00";
const char *ly6 = "\x00\x37" "e\x00" "built\x00the\x00walls\x00\x0d\x00to\x00keep\x00them\x00out\x0e\x00";
//                 1234567890123456789012345678901234567890
const char *ly7 = "\x21nd\x00still\x00we\x00hear\x00them\x00\x0d\x00rant\x00" "and\x00shout\x1a";
const char *ly8 = "\x00\x02\x27ive\x00the\x00" "crops\x00\x0d\x00" "a\x00surge\x00of\x00height\x0e\x02\x00";
const char *ly9 = "\x00\x00\x00\x00\x02\x26" "eed\x00them\x00with\x00\x0d\x00" "electrolyes\x01\x02\x00\x00\x00\x00";
//                  1234567890123456789012345678901234567890
const char *ly10 = "\x00\x34hey\x00" "fly\x00" "drones\x00\x0d\x00to\x00" "douse\x00our\x00" "crop\x0e\x00\x00";
const char *ly11 = "\x00\x00\x00\x2fur\x00sworn\x00" "duty\x1f\x00\x0d\x00" "make\x00them\x00stop\x0e\x00\x00\x00";

const char *ly12 = "\x00\x00\x25" "ach\x00" "day\x00tractors\x00\x0d\x00quickly\x00harvest\x0e\x00\x00";
const char *ly13 = "\x00\x00\x00\x00\x24odging\x00rocks\x00\x0d\x00" "being\x00smartest\x0e\x00\x00\x00\x00";
const char *ly14 = "\x00\x00\x00\x37ith\x00good\x00luck\x00\x0d\x00we\x00" "find\x00repairs\x0e\x00\x00\x00";
const char *ly15 = "\x00\x00\x22" "ecause\x00two\x00hits\x00\x0d\x00is\x00" "all\x00they\x00" "bear\x0e\x00\x00";
const char *ly16 = "\x00\x00\x00\x00\x21\x00" "determined\x00search\x00\x0d\x00" "for\x00" "\x25\x2d\x30s\x0e\x00\x00\x00\x00";
//                  1234567890123456789012345678901234567890
const char *ly17 = "\x34ogether\x00we\x00" "fire\x00\x0d\x00" "and\x00stop\x00their\x00spree\x0e";
const char *ly18 = "\x00\x00\x00\x00\x3a" "ap\x00the\x00" "drone\x00\x0d\x00" "ere\x00" "day\x00is\x00out\x0e\x00\x00\x00\x00";
const char *ly19 = "\x00\x00\x00\x21nd\x00live\x00to\x00" "fight\x00\x0d\x00" "another\x00" "bout\x01\x00\x00\x00";
#endif

#if 0
#define BCD_ADD(va, plus, line) \
        __asm__("ldx #$00"); \
        __asm__("lda %v,x", va); \
        __asm__("adc %v", plus); \
        __asm__("sta %v,x", va); \
        __asm__("bcc %g", va##line); \
        __asm__("ldx #$01"); \
        __asm__("lda %v,x", va); \
        __asm__("adc #$00"); \
        __asm__("sta %v,x", va); \
va##line: \
        __asm__("cld");
#endif

#define UFO_SPRITE_SET_ROW(p0d, p1d, p2d, p3d, r) \
  __asm__("lda #%s", p0d); \
  __asm__("ldx #%s", p1d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00d"); \
  __asm__("stx $d00e"); \
  __asm__("ldx #%s", p2d); \
  __asm__("stx $d00f"); \
  __asm__("ldx #%s", p3d); \
  __asm__("stx $d010");

#define UFO_SPRITE_SET_ROW_END(d) \
  __asm__("lda #%s", d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00d"); \
  __asm__("sta $d010"); \
  __asm__("sta $d00e"); \
  __asm__("sta $d00f");

#define REVERSE(b) ( \
  (((b) & 0x80) >> 7) | (((b) & 0x40) >> 5) | (((b) & 0x20) >> 3) | (((b) & 0x10) >> 1) | (((b) & 0x01) << 7) | (((b) & 0x02) << 5) | (((b) & 0x04) << 3) | (((b) & 0x08) << 1) \
)

#define UFO_SPRITE_SET_ALL_F(f0, f1, f2, f3, f4, f5, f6, t0, t1, t2, t3, t4, t5, t6) \
  ANTIC.wsync = 0; \
  UFO_SPRITE_SET_ROW(3,                     REVERSE(3),                      0, 0,   18); \
  UFO_SPRITE_SET_ROW(12      | 2 | t0,      REVERSE(12       | 2 | t0),      0, 0,   58); \
  UFO_SPRITE_SET_ROW(24      | 6 | t1,      REVERSE(24       | 6 | t1),      0, 0,   18); \
  UFO_SPRITE_SET_ROW(16      | 12 | t2,     REVERSE(16       | 12 | t2),     0, 0,   58); \
  UFO_SPRITE_SET_ROW(32      | 20 | t3,     REVERSE(32       | 20 | t3),     0, 38,  18); \
  UFO_SPRITE_SET_ROW(32      | 4 | t4,      REVERSE(32       | 4 | t4),      0, 102, 58); \
  UFO_SPRITE_SET_ROW(64      | 38 | t5,     REVERSE(64       | 38 | t5),     0, 102, 18); \
  UFO_SPRITE_SET_ROW(64      | 34 | t6,     REVERSE(64       | 34 | t6),     0, 102, 58); \
  UFO_SPRITE_SET_ROW(64      | 51,          REVERSE(64       | 51),          0, 126, 18); \
  UFO_SPRITE_SET_ROW(64  | 1 | 57,          REVERSE(64  | 1  | 57),          0, 126, 58); \
  UFO_SPRITE_SET_ROW(128 | 2 | 206 | f0,    REVERSE(128 | 2  | 206 | f0),    7, 0,   18); \
  UFO_SPRITE_SET_ROW(128 | 2 | 206 | f1,    REVERSE(128 | 2  | 206 | f1),    7, 0,   58); \
  UFO_SPRITE_SET_ROW(128 | 4 | f2,          REVERSE(128 | 4 | f2),          15, 0,   18); \
  UFO_SPRITE_SET_ROW(128 | 4 | f3,          REVERSE(128 | 4  | f3),         15, 0,   18); \
  UFO_SPRITE_SET_ROW(128 | 4 | f4,          REVERSE(128 | 4 | f4),          15, 0,   58); \
  UFO_SPRITE_SET_ROW(128 | 2 | 206 | f5,    REVERSE(128 | 2   | 206 | f5),   7, 0,   18); \
  UFO_SPRITE_SET_ROW(128 | 2 | 206 | f6,    REVERSE(128 | 2   | 206 | f6),   7, 0,   58); \
  UFO_SPRITE_SET_ROW(64  | 1 | 57,          REVERSE(64  | 1   | 57),         0, 126, 18); \
  UFO_SPRITE_SET_ROW(64      | 51 ,         REVERSE(64        | 51),         0, 126, 58); \
  UFO_SPRITE_SET_ROW(64      | 34 | t6,     REVERSE(64        | 34 | t6),    0, 102, 18); \
  UFO_SPRITE_SET_ROW(64      | 38 | t5,     REVERSE(64        | 38 | t5),    0, 102, 58); \
  UFO_SPRITE_SET_ROW(32      | 4 | t4,      REVERSE(32        | 4 | t4),     0, 102, 18); \
  UFO_SPRITE_SET_ROW(32      | 20 | t3,     REVERSE(32        | 20 | t3),    0, 38,  58); \
  UFO_SPRITE_SET_ROW(16      | 12 | t2,     REVERSE(16        | 12 | t2),    0, 0,   18); \
  UFO_SPRITE_SET_ROW(24      | 6 | t1,      REVERSE(24        | 6 | t1),     0, 0,   58); \
  UFO_SPRITE_SET_ROW(12      | 2 | t0,      REVERSE(12        | 2 | t0),     0, 0,   18); \
  UFO_SPRITE_SET_ROW(3,                     REVERSE(3),                      0, 0,   58); \
  UFO_SPRITE_SET_ROW_END(0); \
  ANTIC.wsync = 0;

#define SHADOW_SPRITE_SET_ROW(p0d, p1d) \
  __asm__("lda #%s", p0d); \
  __asm__("ldx #%s", p1d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00d"); \
  __asm__("stx $d00e");

#define SHADOW_SPRITE_SET_ROW_END(d) \
  __asm__("lda #%s", d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00d"); \
  __asm__("sta $d00e");

#define SHADOW_SPRITE_SET_ALL() \
  ANTIC.wsync = 0; \
  GTIA_WRITE.hposp0 = hposShadow; \
  GTIA_WRITE.hposp1 = 8 + hposShadow; \
  ANTIC.wsync = 0; \
  GTIA_WRITE.hposp2 = 2 + hposShadow; \
  GTIA_WRITE.hposp3 = hposShadow; \
  ANTIC.wsync = 0; \
  GTIA_WRITE.prior = PRIOR_PF03_P03; \
  SHADOW_SPRITE_SET_ROW(1,   REVERSE(1)); \
  SHADOW_SPRITE_SET_ROW(7,   REVERSE(7)); \
  SHADOW_SPRITE_SET_ROW(15,  REVERSE(15)); \
  SHADOW_SPRITE_SET_ROW(31,  REVERSE(31)); \
  SHADOW_SPRITE_SET_ROW(63,  REVERSE(63)); \
  SHADOW_SPRITE_SET_ROW(63,  REVERSE(63)); \
  SHADOW_SPRITE_SET_ROW(127, REVERSE(127)); \
  SHADOW_SPRITE_SET_ROW(127, REVERSE(127)); \
  SHADOW_SPRITE_SET_ROW(127, REVERSE(127)); \
  SHADOW_SPRITE_SET_ROW(63,  REVERSE(63)); \
  SHADOW_SPRITE_SET_ROW(63,  REVERSE(63)); \
  SHADOW_SPRITE_SET_ROW(31,  REVERSE(31)); \
  SHADOW_SPRITE_SET_ROW(15,  REVERSE(15)); \
  SHADOW_SPRITE_SET_ROW(7,   REVERSE(7)); \
  SHADOW_SPRITE_SET_ROW(1,   REVERSE(1)); \
  SHADOW_SPRITE_SET_ROW_END(0); \
  GTIA_WRITE.prior = PRIOR_P03_PF03; \
  ANTIC.wsync = 0;

#define UFO_SPRITE_SET_ALL2() \
 { UFO_SPRITE_SET_ALL_F(16,16,24,48,96,32,32, 0, 0, 0, 1, 3, 1, 1); }

#define UFO_SPRITE_SET_ALL1() \
 { UFO_SPRITE_SET_ALL_F(32,32,96,48,24,16,16, 1, 1, 3, 1, 0, 0, 0); }

#define COMBINE_STEERING_SET_ROW(d) \
  __asm__("lda #%s", d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d011");

#define COMBINE_STEERING_SET_ALL() \
    COMBINE_STEERING_SET_ROW(255); \
    COMBINE_STEERING_SET_ROW(255); \
    COMBINE_STEERING_SET_ROW(255); \
    COMBINE_STEERING_SET_ROW(0);

#define COMBINE_SPRITE_SET_ROW(d) \
  __asm__("lda #%s", d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00d"); \
  __asm__("sta $d00e"); \
  __asm__("sta $d00f"); \
  __asm__("sta $d010");

#define COMBINE_SPRITE_SET_ROW_END(d) \
  __asm__("lda #%s", d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00d"); \
  __asm__("sta $d00e"); \
  __asm__("sta $d00f"); \
  __asm__("sta $d010");

#define COMBINE_SPRITE_SET_PLAYER_1(d, e) \
  __asm__("lda #%s", d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00d"); \
  __asm__("lda #%s", e); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00d");

  #define COMBINE_SPRITE_SET_PLAYER_2(d, e) \
  __asm__("lda #%s", d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00e"); \
  __asm__("lda #%s", e); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00e");

  #define COMBINE_SPRITE_SET_PLAYER_3(d, e) \
  __asm__("lda #%s", d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00f"); \
  __asm__("lda #%s", e); \
  __asm__("sta $d40a"); \
  __asm__("sta $d00f");

  #define COMBINE_SPRITE_SET_PLAYER_4(d, e) \
  __asm__("lda #%s", d); \
  __asm__("sta $d40a"); \
  __asm__("sta $d010"); \
  __asm__("lda #%s", e); \
  __asm__("sta $d40a"); \
  __asm__("sta $d010");

#define COMBINE_SPRITES(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11) \
  ANTIC.wsync = 0; \
  COMBINE_SPRITE_SET_ROW(36 | t0); \
  COMBINE_SPRITE_SET_ROW(36 | t1); \
  COMBINE_SPRITE_SET_ROW(36 | t2); \
  COMBINE_SPRITE_SET_ROW(24); \
  COMBINE_SPRITE_SET_ROW(24); \
  COMBINE_SPRITE_SET_ROW(24); \
  COMBINE_SPRITE_SET_ROW(60 | 129 | t3); \
  COMBINE_SPRITE_SET_ROW(36 | 129 | t4); \
  COMBINE_SPRITE_SET_ROW(60 | 129 | t5); \
  COMBINE_SPRITE_SET_ROW(60 | 129 | t6); \
  COMBINE_SPRITE_SET_ROW(60 | 129 | t7); \
  COMBINE_SPRITE_SET_ROW(60 | 129 | t8); \
  COMBINE_SPRITE_SET_ROW(60 | 129 | t9); \
  COMBINE_SPRITE_SET_ROW(60 | 129 | t10); \
  COMBINE_SPRITE_SET_ROW(60 | 129 | t11); \
  COMBINE_SPRITE_SET_ROW(24); \
  COMBINE_SPRITE_SET_ROW(24); \
  COMBINE_SPRITE_SET_ROW_END(0); \
  COMBINE_SPRITE_SET_PLAYER_1(255,0); \
  COMBINE_SPRITE_SET_PLAYER_2(195,0); \
  COMBINE_SPRITE_SET_PLAYER_3(219,0); \
  COMBINE_SPRITE_SET_PLAYER_4(165,0); \
  COMBINE_SPRITE_SET_ROW_END(0); \
  ANTIC.wsync = 0;


#define COMBINE_SPRITES1() \
  COMBINE_SPRITES(94, 129, 94,   66, 0, 66, 0, 66, 0, 66, 0, 66)

#define COMBINE_SPRITES2() \
  COMBINE_SPRITES(129, 90, 129,   0, 66, 0, 66, 0, 66, 0, 66, 0)

void dli_routine7(void);

#define DLI_DELAY() \
    ANTIC.vscrol = fs; \
    GTIA_WRITE.sizep3 = 1; \
    ANTIC.wsync = 0; \
    GTIA_WRITE.hposp0 = hposDrone; \
    GTIA_WRITE.hposp1 = 8 + hposDrone; \
    GTIA_WRITE.hposp2 = 2 + hposDrone; \
    GTIA_WRITE.hposp3 = hposDrone; \
    ANTIC.wsync = 0; \
    GTIA_WRITE.colpm0 = 0x00; \
    GTIA_WRITE.colpm1 = 0x02; \
    GTIA_WRITE.colpm2 = 0x44; \
    GTIA_WRITE.colpm3 = 0x84;


#define DLI_PLAYERS() \
    ANTIC.wsync = 0; \
    GTIA_WRITE.hposp0 = hposp0; \
    GTIA_WRITE.hposm0 = hposm0; \
    GTIA_WRITE.colpm0 = colpm0; \
    ANTIC.wsync = 0; \
    GTIA_WRITE.hposp1 = hposp1; \
    GTIA_WRITE.hposm1 = hposm1; \
    GTIA_WRITE.colpm1 = colpm1; \
    ANTIC.wsync = 0; \
    GTIA_WRITE.hposp2 = hposp2; \
    GTIA_WRITE.hposm2 = hposm2; \
    GTIA_WRITE.colpm2 = colpm2; \
    ANTIC.wsync = 0; \
    GTIA_WRITE.hposp3 = hposp3; \
    GTIA_WRITE.hposm3 = hposm3; \
    GTIA_WRITE.colpm3 = colpm3; \
    ANTIC.wsync = 0; \
    GTIA_WRITE.sizep3 = 0; \
    GTIA_WRITE.sizem = 0x55; \
    ANTIC.wsync = 0; \
    COMBINE_STEERING_SET_ALL();

#define DLI_ENTER() \
    asm("pha"); \
    asm("txa"); \
    asm("pha"); \
    ANTIC.wsync = 0; \
    asm("tya"); \
    asm("pha");

#define DLI_EXIT() \
    asm("pla"); \
    asm("tay"); \
    asm("pla"); \
    asm("tax"); \
    asm("pla"); \
    asm("rti");


#define DLI_ROUTINE(vs, vsn, da, ca, wsyncs) \
void dli_routine##vs(void) { \
    DLI_ENTER(); \
    DLI_DELAY(); \
    wsyncCount = wsyncs; \
    while (wsyncCount != 0) { \
        wsyncCount--; \
        ANTIC.wsync = 0; \
    } \
    UFO_SPRITE_SET_ALL##da(); \
    SHADOW_SPRITE_SET_ALL(); \
    wsyncCount = 108; \
    while (wsyncCount != 0) { \
        wsyncCount--; \
        ANTIC.wsync = 0; \
    } \
    DLI_PLAYERS(); \
    COMBINE_SPRITES##ca(); \
    OS.vdslst = &dli_routine##vsn; \
    DLI_EXIT(); \
}

DLI_ROUTINE(0, 7, 2, 2, 17)
DLI_ROUTINE(1, 0, 2, 2, 18)
DLI_ROUTINE(2, 1, 1, 2, 19)
DLI_ROUTINE(3, 2, 1, 2, 20)
DLI_ROUTINE(4, 3, 2, 1, 21)
DLI_ROUTINE(5, 4, 2, 1, 22)
DLI_ROUTINE(6, 5, 1, 1, 23)
DLI_ROUTINE(7, 6, 1, 1, 23)

#define MOVE_PLAYER(pn, lb, ub) \
    /* Read paddle */ \
    if (hposp##pn != 0) { \
    trigger = PEEK(PTRIG##pn); \
    paddle = PEEK(PADDL##pn); \
    /* Determine new steering direction */ \
    if (paddle > PADDLE_STEER_LEFT) newDirp##pn = 0; \
    else if (paddle < PADDLE_STEER_RIGHT) newDirp##pn = 4; \
    else newDirp##pn = 2; \
    /* Trigger pressed or Player changed direction? */ \
    if (((trigger == 0) && (lastTrigp##pn != 0)) || (newDirp##pn != lastDirp##pn)) \
    { \
      /* Yes, force move now */ \
      lastDirp##pn = newDirp##pn; \
      delayp##pn = PADDLE_STEER_DELAY; \
    } \
    lastTrigp##pn = trigger; \
    delayp##pn++; \
    if (delayp##pn > PADDLE_STEER_DELAY) \
    { \
      /* Move player, update steering indicator */ \
      delayp##pn = 0; \
      if (paddle > PADDLE_STEER_LEFT) { if (hposp##pn > lb) hposp##pn += -4; hposm##pn = hposp##pn; } \
      else if (paddle < PADDLE_STEER_RIGHT) { if (hposp##pn < ub) hposp##pn += 4; hposm##pn = hposp##pn + 4; } \
      else hposm##pn = hposp##pn + 2; \
    } }

uint8_t *phrc;
uint8_t *phrct;
uint8_t *phrd; /* Drone Row Pointer */

#define HIT_PLAYER(pn) if (hposp##pn != 0) { \
    paddle = (hposp##pn - (PLAYER_MIN_HPOS - PLAYER_WIDTH)) >> 2; \
    phrct = phrc + paddle; \
    if (*phrct == CHAR_CROP) { *phrct = CHAR_BLANK; score++; } \
    else if (*phrct == CHAR_ROCK) { if (colpm##pn == ((HUE_MAGENTA << 4) | 0x06)) { idleInit = 1; } else { colpm##pn = (HUE_MAGENTA << 4) | 0x06; } } \
    else if (*phrct == (CHAR_EMP | 0x80)) { *phrct = CHAR_BLANK; colpm##pn = (HUE_BLUE2 << 4) | 0x06; } \
    else if (*phrct == CHAR_REPAIR) { *phrct = CHAR_BLANK; colpm##pn = 0; } \
    phrct++; \
    if (*phrct == CHAR_CROP) { *phrct = CHAR_BLANK; score++; } \
    else if (*phrct == CHAR_ROCK) { if (colpm##pn == ((HUE_MAGENTA << 4) | 0x06)) { idleInit = 1; } else { colpm##pn = (HUE_MAGENTA << 4) | 0x06; } } \
    else if (*phrct == (CHAR_EMP | 0x80)) { *phrct = CHAR_BLANK; colpm##pn = (HUE_BLUE2 << 4) | 0x06; } \
    else if (*phrct == CHAR_REPAIR) { *phrct = CHAR_BLANK; colpm##pn = 0; } }

#define BLOW_PLAYER(pn) { \
    blowUp##pn = (hposp##pn == 0) && (hposm##pn == 0); \
    if ((hposp##pn != 0) && (hposm##pn != 0) && (lastTrigp##pn == 0) && ((colpm##pn & 0xF0) == (HUE_BLUE2 << 4))) { blowUp##pn = 1; } }

#define TOGGLE_PLAYER(pn) \
    /* Read paddle */ \
    trigger = PEEK(PTRIG##pn); \
    if ((trigger == 1) && (lastTrigp##pn != trigger)) \
    { \
        if (trigCountp##pn < 15) { \
            if (hposp##pn == 0) \
            { \
                hposp##pn = PLAYER##pn##_DFL_HPOS; \
                hposm##pn = PLAYER##pn##_DFL_HPOS + 2; \
            } \
            else \
            { \
                hposm##pn = 0; \
                hposp##pn = 0; \
            } \
        } \
        trigCountp##pn = 0; \
    } \
    else if (trigger == 0) { \
        trigCountp##pn++; \
        if ((trigCountp##pn > 180) && (hposp##pn != 0)) { \
            trigCountp##pn = 0; \
            dayInit = 1; \
        } \
    } \
    lastTrigp##pn = trigger;

void dvbi_routine_GameIdleInit(void);
void dvbi_routine_Notice(void);

/* --------------------------------------------------------------------------------------------- */
/* Delayed VBI - MODE: Game Started                                                              */
/* --------------------------------------------------------------------------------------------- */
void dvbi_routine_GameRunning(void)
{
    GTIA_WRITE.prior = PRIOR_P03_PF03;

    if (blown == 0)
    {
        if (targeted == 0)
        {
        if ((droneTarget > hposDrone))
        {
            hposDrone++;
            hposShadow = hposShadowDelta + hposDrone - 16;
        }
        else if ((droneTarget < hposDrone))
        {
            hposDrone--;
            hposShadow = hposShadowDelta + hposDrone - 16;
        }
        else 
        {
            targeted = (rf() & 0x03) + 1;
        }
        }    
        else if (targeted == 1)
        {
            hposDrone++;
            hposShadow = hposShadowDelta + hposDrone - 16;
        }
        else if (targeted == 2)
        {
            hposDrone--;
            hposShadow = hposShadowDelta + hposDrone - 16;
        }
    }

    /* Coarse scroll? */
    if (fs == 0)
    {
      #if 1
        rp = &rows[line][3];
        memset(rp, 1, NUM_COLUMNS - 6);
       #endif     
    }
    else if (fs == 0x07)
    {
        /* Yes, wrap? */
        if (line == 0)
        {
            /* Yes, start back at beginning */
            line = NUM_ROWS - 1;
            dlp = &dlist;
            vblanks++;
        }
        else
        {
            line--;
            dlp += DL_SIZE;
        }

        /* Copy proper display list, advance to next coarse scroll */
        memcpy(dl, dlp, DL_SIZE - 3);
    }
    else if (fs == 1)
    {
        animation++;
        animation &= 0x03;
        memcpy(p + (CHAR_REPAIR * CHAR_SET_SIZE), &repairsCharAnimation[animation], CHAR_SET_SIZE);
        memcpy(p + (CHAR_EMP * CHAR_SET_SIZE), &empCharAnimation[animation], CHAR_SET_SIZE);
#if 1
        if (cline == 0)
        {
            /* Yes, start back at beginning */
            cline = NUM_ROWS - 1;
            phrc = &rows[NUM_ROWS - 1][0];
        }
        else
        {
            cline--;
            phrc -= NUM_COLUMNS;
        }

        if (dline == 0)
        {
            /* Yes, start back at beginning */
            dline = NUM_ROWS - 1;
            phrd = &rows[NUM_ROWS - 1][0];
        }
        else
        {
            dline--;
            phrd -= NUM_COLUMNS;
        }

        //memset(&rows[line], 1, NUM_COLUMNS);

#endif            
    }
    else if (fs == 2)
    {
        HIT_PLAYER(0);
        HIT_PLAYER(1);
        HIT_PLAYER(2);
        HIT_PLAYER(3);
    }
    else if (fs == 3)
    {
        /* Drone blowing up animation and sound */
        if (blowUp != 0)
        {
            if (blowUp == 1) {
                ufoVolume = 15;
                //hposDrone = 120; 
                blowUp++;
            }
            else if (blowUp == 2) {
                GTIA_WRITE.sizep0 = 1;
                GTIA_WRITE.sizep1 = 1;
                GTIA_WRITE.sizep2 = 1;
                GTIA_WRITE.sizep3 = 1;
                //hposDrone = 120 - 4;
                hposDrone -= 4;
                blowUp++;
            }
            else if (blowUp == 3) {
                GTIA_WRITE.sizep0 = 3;
                GTIA_WRITE.sizep1 = 3;
                GTIA_WRITE.sizep2 = 3;
                GTIA_WRITE.sizep3 = 3;
                //hposDrone = 120 - 12;
                hposDrone -= 12;
                blowUp++;
            }
            else {
                /* Hide the drone */
                GTIA_WRITE.sizep0 = 0;
                GTIA_WRITE.sizep1 = 0;
                GTIA_WRITE.sizep2 = 0;
                GTIA_WRITE.sizep3 = 0;
                hposDrone = 0;
                hposShadow = 0;
                blowUp = 0;
                blown = 1;
                dayInit = 1;
                //OS.vvblkd = &dvbi_routine_Notice;
            }
        }
        else
        {
#if 1            
            /* Move shadoow to show day going by */
            hposShadowCounts++;
            if (hposShadowCounts > 16)
            {
                hposShadowCounts = 0;;
                hposShadowDelta++;
                if (hposShadowDelta >= 48)
                {
                    /* Shadow reached end of day!!  */
                    idleInit = 1;
                }
                else if (hposShadowDelta > 40)
                {
                    mainbgcolor = MAIN_COLOR_START - (hposShadowDelta - 40);
                    OS.color4 = mainbgcolor;
                }
            }
#endif                
        }
    }
    else if (fs == 5)
    {
        /* Drone eats crops, randomly drops items */
        if ((hposDrone >= PLAYER_MIN_HPOS) && (hposDrone < PLAYER_MAX_HPOS))
        {
            rev = ((hposDrone - PLAYER_MIN_HPOS) >> 2) + 3;
            phrd += rev;
            rfv(ramp);
            if (ramp < 40)
            {
                *phrd = CHAR_ROCK;
                //rows[dline][rev] = CHAR_ROCK;
            }
            else if (ramp < 50)
            {
                *phrd = CHAR_REPAIR;
                //rows[dline][rev] = CHAR_REPAIR;
            }
            else if (ramp < 55)
            {
                *phrd = CHAR_EMP | 0x80;
                //rows[dline][rev] = CHAR_EMP | 0x80;
            }
            else
            {
                *phrd = CHAR_BLANK;
                //rows[dline][rev] = 0;
            }
            phrd++;
            rev++;
            rfv(ramp);
            if (ramp > 214)
            {
                *phrd = CHAR_ROCK;
                //rows[dline][rev] = CHAR_ROCK;
            }
            else if (ramp > 204)
            {
                *phrd = CHAR_REPAIR;
                //rows[dline][rev] = CHAR_REPAIR;
            }
            else if (ramp > 199)
            {
                *phrd = CHAR_EMP | 0x80;
                //rows[dline][rev] = CHAR_EMP | 0x80;
            }
            else
            {
                *phrd = CHAR_BLANK;
                //rows[dline][rev] = 0;
            }
            phrd -= rev;
        }
    }
    else if (fs == 6)
    {
        /* Check for blow up drone */
        if ((blowUp == 0) && (blown == 0))
        {
            BLOW_PLAYER(0);
            BLOW_PLAYER(1);
            BLOW_PLAYER(2);
            BLOW_PLAYER(3);
            if (blowUp0 && blowUp1 && blowUp2 && blowUp3)
            {
                blowUp = 1;
            }
        }
    }

    fs--;
    fs &= 0x07;

    if (rmtplayCount < 5)
    {
        RMTPlay;
        if (ufoVolume != 0)
        {
            /* Blow up sound effect */
            POKEY_WRITE.audf4 = 32;
            POKEY_WRITE.audc4 = ufoVolume;
            ufoVolume--;
        }
#if 0
        else if (rev)
        {
          POKEY_WRITE.audf4 = tone;
          POKEY_WRITE.audc4 = 0x80 | (volume >> 4);
          if (ramp)
          {
            if (volume < 127) volume += 2;
          }
          else if (volume > 16) volume-=3;
        }
        else
        {
          POKEY_WRITE.audf4 = tone;
          POKEY_WRITE.audc4 = 0x80 | (volume >> 4);
          if (ramp)
          {
            if (volume < 127) volume += 2;
          }
          else if (volume > 16) volume -= 3;
        }
#endif

    }
    rmtplayCount++;
    if (rmtplayCount > 5) rmtplayCount = 0;

#if 1
    if (hposp0 == 0) thposp0 = PLAYER_MIN_HPOS;
    else thposp0 = hposp0 + PLAYER_WIDTH;

    if (hposp1 == 0)
    {
        thposp0 = PLAYER_MIN_HPOS;
    }
    else
    {
        thposp0 = hposp0 + PLAYER_WIDTH;
    }

    if (hposp3 == 0) thposp3 = PLAYER_MAX_HPOS;
    else thposp3 = hposp3 - PLAYER_WIDTH;
    
    if (thposp1 == 0) thposp1 = PLAYER_MIN_HPOS;
    if (thposp2 == 0) thposp2 = PLAYER_MIN_HPOS;
    if (thposp3 == 0) thposp3 = PLAYER_MIN_HPOS;

    MOVE_PLAYER(0, PLAYER_MIN_HPOS, PLAYER_MAX_HPOS);
    MOVE_PLAYER(1, PLAYER_MIN_HPOS, PLAYER_MAX_HPOS);
    MOVE_PLAYER(2, PLAYER_MIN_HPOS, PLAYER_MAX_HPOS);
    MOVE_PLAYER(3, PLAYER_MIN_HPOS, PLAYER_MAX_HPOS);
#endif

    /* JMP to XITVBV */
    __asm__("jmp $E462");
}


/* --------------------------------------------------------------------------------------------- */
/* Delayed VBI - MODE: Game Started                                                              */
/* --------------------------------------------------------------------------------------------- */
void dvbi_routine_GameIdle(void)
{
    GTIA_WRITE.prior = PRIOR_P03_PF03;

    /* Coarse scroll? */
    if (fs == 0)
    {

    }
    else if (fs == 0x07)
    {
        /* Yes, wrap? */
        if (line == 0)
        {
            /* Yes, start back at beginning */
            line = NUM_ROWS - 1;
            dlp = &dlist;
            vblanks++;
            if (vblanks > 3) vblanks = 0;
        }
        else
        {
            line--;
            dlp += DL_SIZE;
        }

        /* Copy proper display list, advance to next coarse scroll */
        memcpy(dl, dlp, DL_SIZE - 3);
    }
    else if (fs == 2)
    {
        if (vblanks == 0)
        {
            WRITE_TEXT(8, ROW_ADDR_OFF, scoreStr, sizeof(scoreStr));
        }
        else if (vblanks == 1)
        {
            WRITE_TEXT(8, ROW_ADDR_OFF, paddleStr, sizeof(paddleStr));
        }
        else if (vblanks == 2)
        {
            WRITE_TEXT(8, ROW_ADDR_OFF, togglePlayerStr, sizeof(togglePlayerStr));
        }
        else
        {
            WRITE_TEXT(8, ROW_ADDR_OFF, fireStartStr, sizeof(fireStartStr));
        }
    }
    else if (fs == 3)
    {
        if (line == NUM_ROWS - 1)
        {
            hposShadow++;
            if (hposShadow >= (hposDrone + 16))
            {
                /* Reset the day */
                hposShadow = hposDrone - 16;
                mainbgcolor = MAIN_COLOR_START;
                OS.color4 = mainbgcolor;
            }
            else //if (hposShadow & 0x01)
            {
#if 0 
// Daytime background color transitions               
                if (hposShadow >= hposDrone)
                {
                    OS.color4 = mainbgcolor--;
                }
                else
                {
                    OS.color4 = mainbgcolor++;
                }
#endif
            }
        }
    }
    else if (fs == 4)
    {
        animation++;
        animation &= 0x03;
        memcpy(p + (CHAR_REPAIR * CHAR_SET_SIZE), &repairsCharAnimation[animation], CHAR_SET_SIZE);
        memcpy(p + (CHAR_EMP * CHAR_SET_SIZE), &empCharAnimation[animation], CHAR_SET_SIZE);

        hposShadow++;
        if (hposShadow >= (hposDrone + 16))
        {
            /* Reset */
            hposShadow = hposDrone - 16;
        }
    }
    fs--;
    fs &= 0x07;

    TOGGLE_PLAYER(0);
    TOGGLE_PLAYER(1);
    TOGGLE_PLAYER(2);
    TOGGLE_PLAYER(3);
    
    if (rmtplayCount < 5)
    {
#if RMT_RUN == 1  
    RMTPlay;
#endif      
    }
    rmtplayCount++;
    if (rmtplayCount > 5) rmtplayCount = 0;

    /* JMP to XITVBV */
    __asm__("jmp $E462");
}

/* --------------------------------------------------------------------------------------------- */
/* Delayed VBI - MODE: Game Idle Initialization                                                  */
/* --------------------------------------------------------------------------------------------- */
void dvbi_routine_GameIdleInit(void)
{
    /* JMP to XITVBV */
    __asm__("jmp $E462");
}

/* --------------------------------------------------------------------------------------------- */
/* Delayed VBI - MODE: Show Notice                                                               */
/* --------------------------------------------------------------------------------------------- */
void dvbi_routine_Notice(void)
{
    /* Switch to Idle State */
    if (rmtplayCount < 5)
    {
#if RMT_RUN == 1  
    RMTPlay;
#endif      
    }
    rmtplayCount++;
    if (rmtplayCount > 5) rmtplayCount = 0;

    vblanks++;
    if (vblanks == 0)
    {
        day++;
        runInit = 1; 
        //OS.vvblkd = &dvbi_routine_GameIdleInit; 

    }

    __asm__("jmp $E462");
}

#if DO_LYRICS == 1
/* --------------------------------------------------------------------------------------------- */
/* Delayed VBI - MODE: Game Idle Initialization                                                  */
/* --------------------------------------------------------------------------------------------- */
void dvbi_routine_ShowLyrics(void)
{
    /* Switch to Idle State */
    if (rmtplayCount < 5)
    {
#if RMT_RUN == 1  
    RMTPlay;
#endif      
    }
    rmtplayCount++;
    if (rmtplayCount > 5) rmtplayCount = 0;

    //OS.color4 = mainbgcolor;

    __asm__("jmp $E462");
}

uint8_t LyricWait(uint8_t vblanks)
{
    line = 0;
    while (line != vblanks)
    {
        line++;
        waitForVBLANK();
        if (PEEK(PTRIG0) == 0) return 1;
        if (PEEK(PTRIG1) == 0) return 1;
        if (PEEK(PTRIG2) == 0) return 1;
        if (PEEK(PTRIG3) == 0) return 1;
    }
    return 0;
}

#endif
/* --------------------------------------------------------------------------------------------- */
/* Main loop                                                                                     */
/* --------------------------------------------------------------------------------------------- */
void main(void)
{
#if DO_LYRICS == 1

#define LYRIC(lp, ls) \
        memcpy(&notice[0], lp, 40); \
        if (LyricWait(48)) goto lyricBreak;

#define LYRIC2(lp, ls) \
        memcpy(&notice[0], lp, 40); \
        if (LyricWait(96)) goto lyricBreak;

    /* Display Lyrics to Music */

    /* Copy the next display list to base DL List RAM */
    waitForVBLANK();
    OS.color0 = 0;
    //OS.color1 = 0;
    OS.color2 = 0;
    OS.color3 = 0;
    OS.color4 = 0;

    //OS.chbas = 0xE0;
    memcpy((void *)DL_ADDR, &dlistlyrics, DL_SIZE);
    ANTIC.nmien = 0x00;
    OS.vvblkd = &dvbi_routine_ShowLyrics;
    OS.sdlst = (void *)DL_ADDR;
    RMTInitIdle;
    ANTIC.nmien = 0xC0;

    //for (;;)
    {
        LyricWait(192);

        LYRIC(ly1, 26);
        LYRIC(ly2, 28);
        LYRIC(ly1, 26);
        LYRIC(ly3, 29);

        LYRIC2(ly4, 39);
        LYRIC2(ly5, 37);
        LYRIC2(ly6, 39);
        LYRIC2(ly7, 40);

        LYRIC2(ly8, 38);
        LYRIC2(ly9, 36);
        LYRIC2(ly10, 38);
        LYRIC2(ly11, 37);

        memset(&notice[0], 0, 40);

        LyricWait(192);

        LYRIC(ly1, 26);
        LYRIC(ly2, 28);
        LYRIC(ly1, 26);
        LYRIC(ly3, 29);

        LYRIC2(ly12, 38);
        LYRIC2(ly13, 35);
        LYRIC2(ly14, 36);
        LYRIC2(ly15, 38);

        LYRIC2(ly16, 36);
        LYRIC2(ly17, 40);
        LYRIC2(ly18, 35);
        LYRIC2(ly19, 37);

        memset(&notice[0], 0, 40);
    }
lyricBreak:    

    /* Initialize Terrain */
#if 1
    RMTStop;
    ANTIC.nmien = 0x00;
#endif

    /* Generate rock walls */
    memset(rows, 0, sizeof(rows));
    for (line = 0; line < NUM_ROWS; line++)
    {
        rows[line][0] = (rf() & 0x03) + 5;
        rows[line][1] = (rf() & 0x03) + 5;

        rows[line][NUM_COLUMNS - 1] = (rf() & 0x03) + 5;
        rows[line][NUM_COLUMNS - 2] = (rf() & 0x03) + 5;
    }

    /* Copy the next display list to base DL List RAM */
    memcpy((void *)DL_ADDR, &dlist, DL_SIZE);

    ANTIC.nmien = 0xC0;
    waitForVBLANK();

    /* Initialize and sitch to custom character set @ CS_ADDR */
    OS.chbas = CS_ADDR_HI;

    /* Initialize display list */
    waitForVBLANK();
    ANTIC.nmien = 0x00;
    dl = (char *)DL_ADDR;
    memcpy(dl, dlp, DL_SIZE);
    line = 0;
    fs = 7;
    ANTIC.vscrol = fs;
    dline = 5;
    cline = 25;
    phrc = &rows[cline][0];
    phrd = &rows[dline][0];

    /* Initialize position of players */
    lastTrigp0 = 1;
    lastTrigp1 = 1;
    lastTrigp2 = 1;
    lastTrigp3 = 1;
#if 0
    hposp0 = 0;
    hposm0 = 0;
    colpm0 = 0;
    hposp1 = 0;
    hposm1 = 0;
    colpm1 = 0;
    hposp2 = 0;
    hposm2 = 0;
    colpm2 = 0;
    hposp3 = 0;
    hposm3 = 0;
    colpm3 = 0;
#endif
    /* Initialize playfield colors */
    OS.color0 = PF0_COLOR;
    OS.color1 = PF1_COLOR;
    OS.color2 = PF2_COLOR;
    OS.color3 = PF3_COLOR;
    OS.color4 = mainbgcolor;

    /* Install Delayed VBI, DLI, and Display List */
    //OS.vvblkd = dvbi_routine_GameStarted;
    idleInit = 1;
    OS.vvblkd = &dvbi_routine_GameIdleInit;
    ANTIC.nmien = 0xC0;

    while (1)
    {

        if (idleInit == 1)
        {
#if 1
            ANTIC.nmien = 0x00;
            
#if RMT_RUN == 1      
            RMTInitIdle;
#endif
            //WRITE_TEXT(9, ROW_ADDR_OFF, togglePlayerStr, sizeof(togglePlayerStr));
            //WRITE_TEXT(10, ROW_ADDR_OFF, fireStartStr, sizeof(fireStartStr));
            /* Generate Farmsquad logo */
            //const char framsquad[FARM_SQUAD_LINES][NUM_PLAY_COLUMNS] =
            for (paddle = 0; paddle < NUM_ROWS; paddle++)
            {
                memset(&rows[paddle][ROW_ADDR_OFF], 0, NUM_PLAY_COLUMNS);
            }

            for (paddle = 0; paddle < FARM_SQUAD_LINES; paddle++)
            {
                memcpy(&rows[paddle + 9][ROW_ADDR_OFF], &framsquad[paddle], NUM_PLAY_COLUMNS);
            }
            WRITE_TEXT(16, ROW_ADDR_OFF, cropRockStr, sizeof(cropRockStr));
            WRITE_TEXT(17, ROW_ADDR_OFF, repairEmpStr, sizeof(repairEmpStr));

            if (score != 0)
            {
                WRITE_TEXT(6, ROW_ADDR_OFF, gameOverStr, sizeof(gameOverStr));
                
                if (score > hiscore) hiscore = score;

                scoreStr[0] = ((score / 10000) % 10 << 1) + CHAR_0;
                scoreStr[1] = scoreStr[0] + 1;

                scoreStr[2] = ((score / 1000) % 10 << 1) + CHAR_0;
                scoreStr[3] = scoreStr[2] + 1;

                scoreStr[4] = ((score / 100) % 10 << 1) + CHAR_0;
                scoreStr[5] = scoreStr[4] + 1;

                scoreStr[6] = ((score / 10) % 10 << 1) + CHAR_0;
                scoreStr[7] = scoreStr[6] + 1;

                scoreStr[8] = (score % 10 << 1) + CHAR_0;
                scoreStr[9] = scoreStr[8] + 1;

                scoreStr[24] = ((hiscore / 10000) % 10 << 1) + CHAR_0;
                scoreStr[25] = scoreStr[24] + 1;

                scoreStr[26] = ((hiscore / 1000) % 10 << 1) + CHAR_0;
                scoreStr[27] = scoreStr[26] + 1;

                scoreStr[28] = ((hiscore / 100) % 10 << 1) + CHAR_0;
                scoreStr[29] = scoreStr[28] + 1;

                scoreStr[30] = ((hiscore / 10) % 10 << 1) + CHAR_0;
                scoreStr[31] = scoreStr[30] + 1;

                scoreStr[32] = (hiscore % 10 << 1) + CHAR_0;
                scoreStr[33] = scoreStr[32] + 1;
            }
            score = 0;

            if (hposp0 != 0)
            {
                hposp0 = PLAYER0_DFL_HPOS; \
                hposm0 = PLAYER0_DFL_HPOS + 2; \
            }
            if (hposp1 != 0)
            {
                hposp1 = PLAYER1_DFL_HPOS; \
                hposm1 = PLAYER1_DFL_HPOS + 2; \
            }
            if (hposp2 != 0)
            {
                hposp2 = PLAYER2_DFL_HPOS; \
                hposm2 = PLAYER2_DFL_HPOS + 2; \
            }
            if (hposp3 != 0)
            {
                hposp3 = PLAYER3_DFL_HPOS; \
                hposm3 = PLAYER3_DFL_HPOS + 2; \
            }

            lastTrigp0 = 1;
            lastTrigp1 = 1;
            lastTrigp2 = 1;
            lastTrigp3 = 1;
            colpm0 = 0;
            colpm1 = 0;
            colpm2 = 0;
            colpm3 = 0;
            trigCountp0 = 0;
            trigCountp1 = 0;
            trigCountp2 = 0;
            trigCountp3 = 0;

            hposDrone = 120;
            hposShadow = hposDrone - 16;

            mainbgcolor = MAIN_COLOR_START;
            OS.color4 = mainbgcolor;

            OS.vvblkd = &dvbi_routine_GameIdle;
            OS.vdslst = &dli_routine7;
            OS.sdlst = (void *)DL_ADDR;
            dl = (char *)DL_ADDR;
            memcpy(dl, dlp, DL_SIZE);
            line = 0;
            fs = 7;
            ANTIC.vscrol = fs;
            dline = 5;
            cline = 24;
            phrc = &rows[cline][0];
            phrd = &rows[dline][0];

            ANTIC.nmien = 0xC0;
#endif                
            day = 1;
            hposShadowDelta = 0;
            hposShadowCounts = 0;
            idleInit = 0;
        }
        else if (runInit == 1)
        {
#if 1
            ANTIC.nmien = 0x00;
            
#if RMT_RUN == 1      
            RMTInitRun;
#endif
            hposDrone = 120;
            hposShadow = hposDrone - 16;
            hposShadowDelta = 0;
            hposShadowCounts = 0;
            blowUp = 0;
            blown = 0;

            lastTrigp0 = 1;
            lastTrigp1 = 1;
            lastTrigp2 = 1;
            lastTrigp3 = 1;
            colpm0 = 0;
            colpm1 = 0;
            colpm2 = 0;
            colpm3 = 0;
            trigCountp0 = 0;
            trigCountp1 = 0;
            trigCountp2 = 0;
            trigCountp3 = 0;
            for (paddle = 0; paddle < NUM_ROWS; paddle++)
            {
                memset(&rows[paddle][ROW_ADDR_OFF], 0, NUM_PLAY_COLUMNS);
            }
            OS.vvblkd = &dvbi_routine_GameRunning;
            OS.vdslst = &dli_routine7;
            OS.sdlst = (void *)DL_ADDR;
            dl = (char *)DL_ADDR;
            memcpy(dl, dlp, DL_SIZE);
            line = 0;
            fs = 7;
            ANTIC.vscrol = fs;
            dline = 5;
            cline = 25;
            phrc = &rows[cline][0];
            phrd = &rows[dline][0];
            vblanks = 0;

            /* Initialize playfield colors */
            OS.color0 = PF0_COLOR;
            OS.color1 = PF1_COLOR;
            OS.color2 = PF2_COLOR;
            OS.color3 = PF3_COLOR;
            mainbgcolor = MAIN_COLOR_START;
            OS.color4 = mainbgcolor;

            /* Initialize and sitch to custom character set @ CS_ADDR */
            OS.chbas = CS_ADDR_HI;

            ANTIC.nmien = 0xC0;
#endif                
            runInit = 0;
        }
        else if (dayInit == 1)
        {
            dayInit = 0;
            vblanks = 0;
            waitForVBLANK();
            ANTIC.nmien = 0x00;
            memcpy((void *)DL_ADDR, &dlistlyrics, DL_SIZE);
            OS.vvblkd = &dvbi_routine_Notice;

            memset(&notice[0], 0, 40);

            //notice[28] = ((day / 100) % 10 << 1) + CHAR_0;
            //notice[29] = scoreStr[28] + 1;

            //notice[30] = ((day / 10) % 10 << 1) + CHAR_0;
            //notice[31] = notice[30] + 1;

            notice[20] = ((day / 100) % 10) + 16;
            notice[21] = ((day / 10) % 10) + 16;
            notice[22] = (day % 10) + 16;

            memcpy(&notice[16], "\x24" "ay", 3);

            OS.color0 = 0;
            //OS.color1 = 0;
            OS.color2 = 0;
            OS.color3 = 0;
            OS.color4 = 0;
            OS.chbas = 0xE0;

            ANTIC.nmien = 0xC0;
        }

#if 1
        droneTargetCount++;
        if ((droneTargetCount == 320) && (hposp0 != 0)) { targeted = 0; droneTarget = hposp0 - 4; }
        else if ((droneTargetCount == 640) && (hposp1 != 0)) { targeted = 0; droneTarget = hposp1 - 4; }
        else if ((droneTargetCount == 960) && (hposp2 != 0)) { targeted = 0; droneTarget = hposp2 - 4; }
        else if ((droneTargetCount == 1280) && (hposp3 != 0)) { targeted = 0; droneTarget = hposp3 - 4; }

        if (droneTargetCount > 1280) droneTargetCount = 0;
#endif            
    }
}
